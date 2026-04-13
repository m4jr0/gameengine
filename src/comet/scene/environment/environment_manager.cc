// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "environment_manager.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/math/geometry.h"
#include "comet/math/math_common.h"
#include "comet/math/math_interpolation.h"
#include "comet/physics/physics_manager.h"
#include "comet/profiler/profiler.h"
#include "comet/rendering/light/light_manager.h"
#include "comet/rendering/rendering_manager.h"

namespace comet {
namespace scene {
EnvironmentManager& EnvironmentManager::Get() {
  static EnvironmentManager singleton{};
  return singleton;
}
void EnvironmentManager::Initialize() {
  Manager::Initialize();

  const auto& shadow_settings{
      rendering::RenderingManager::Get().GetShadowSettings()};

  rendering::LightDescr sun{};
  sun.props.type = rendering::LightType::Directional;
  sun.props.direction = math::Vec3{.0f, -1.0f, .0f};
  sun.props.color = math::Vec3{1.0f, .95f, .85f};
  sun.props.intensity = 4.0f;

  sun.shadow.is_enabled = true;
  sun.shadow.max_distance = shadow_settings.max_distance;
  sun.shadow.bias_constant = shadow_settings.bias_constant;
  sun.shadow.bias_slope = shadow_settings.bias_slope;

  sun_light_id_ = rendering::LightManager::Get().Generate(sun);
}

void EnvironmentManager::Shutdown() {
  if (sun_light_id_ != rendering::kInvalidLightId) {
    rendering::LightManager::Get().Destroy(sun_light_id_);
    sun_light_id_ = rendering::kInvalidLightId;
  }

  Manager::Shutdown();
}

void EnvironmentManager::Update(frame::FramePacket* packet) {
  COMET_PROFILE("EnvironmentManager::Update");

  AdvanceTime(
      static_cast<f32>(physics::PhysicsManager::Get().GetFixedDeltaTime()));

  packet->ambient_color = ComputeAmbientColor();

  if (sun_light_id_ == rendering::kInvalidLightId) {
    return;
  }

  auto time_hours{GetEffectiveTimeOfDayHours()};
  auto day_alpha{time_hours / 24.0f};
  auto azimuth{azimuth_offset_ + day_alpha * static_cast<f32>(math::kTwoPi)};

  constexpr f32 kNoonElevationDeg{60.0f};
  auto solar_phase{day_alpha * math::kTwoPi - math::kHalfPi};
  auto max_elevation{math::ConvertToRadians(kNoonElevationDeg)};
  auto elevation{static_cast<f32>(max_elevation * math::Sin(solar_phase))};

  auto sun_direction{-GenerateSunDirection(azimuth, elevation)};
  auto sun_daylight{math::Max(.0f, sun_direction.y * -1.0f)};
  constexpr f32 kNightIntensity{.05f};
  constexpr f32 kDayIntensity{4.0f};
  auto sun_intensity{math::Lerp(kNightIntensity, kDayIntensity, sun_daylight)};

  auto& light_manager{rendering::LightManager::Get()};
  light_manager.SetDirection(sun_light_id_, sun_direction);
  light_manager.SetIntensity(sun_light_id_, sun_intensity);
}

// Time controls.
void EnvironmentManager::SetTimeFrozen(bool is_frozen) {
  is_time_frozen_ = is_frozen;
}

bool EnvironmentManager::IsTimeFrozen() const { return is_time_frozen_; }

void EnvironmentManager::SetTimeOfDayHours(f32 hours) {
  time_of_day_hours_ = math::Wrap(hours, 24.0f);
}

f32 EnvironmentManager::GetTimeOfDayHours() const { return time_of_day_hours_; }

void EnvironmentManager::SetDayDurationSeconds(f32 seconds) {
  day_duration_seconds_ = math::Max(seconds, .001f);
}

f32 EnvironmentManager::GetDayDurationSeconds() const {
  return day_duration_seconds_;
}

void EnvironmentManager::SetDayTimeScale(f32 scale) {
  day_time_scale_ = math::Max(scale, .0f);
}

f32 EnvironmentManager::GetDayTimeScale() const { return day_time_scale_; }

void EnvironmentManager::SetDayAccelerationFactor(f32 factor) {
  factor = math::Max(factor, .001f);
  SetDayDurationSeconds(kRealDayDurationSeconds / factor);
}

f32 EnvironmentManager::GetDayAccelerationFactor() const {
  if (day_duration_seconds_ <= .0f) {
    return .0f;
  }

  return kRealDayDurationSeconds / day_duration_seconds_;
}

f32 EnvironmentManager::GetEffectiveDayDurationSeconds() const {
  if (day_time_scale_ <= .0f) {
    return .0f;
  }

  return day_duration_seconds_ / day_time_scale_;
}

f32 EnvironmentManager::GetEffectiveDayAccelerationFactor() const {
  auto effective_duration{GetEffectiveDayDurationSeconds()};
  if (effective_duration <= .0f) {
    return .0f;
  }

  return kRealDayDurationSeconds / effective_duration;
}

void EnvironmentManager::SetEffectiveDayDurationSeconds(f32 seconds) {
  seconds = math::Max(seconds, .001f);
  auto base_duration{seconds * math::Max(day_time_scale_, .001f)};
  SetDayDurationSeconds(base_duration);
}

void EnvironmentManager::SetEffectiveDayAccelerationFactor(f32 factor) {
  factor = math::Max(factor, .001f);
  SetEffectiveDayDurationSeconds(kRealDayDurationSeconds / factor);
}

s32 EnvironmentManager::GetDayDurationHoursPart() const {
  auto total_seconds{static_cast<s32>(day_duration_seconds_)};
  return total_seconds / 3600;
}

s32 EnvironmentManager::GetDayDurationMinutesPart() const {
  auto total_seconds{static_cast<s32>(day_duration_seconds_)};
  return (total_seconds % 3600) / 60;
}

s32 EnvironmentManager::GetDayDurationSecondsPart() const {
  auto total_seconds{static_cast<s32>(day_duration_seconds_)};
  return total_seconds % 60;
}

void EnvironmentManager::SetDayDurationHms(s32 hours, s32 minutes,
                                           s32 seconds) {
  hours = math::Max(hours, 0);
  minutes = math::Clamp(minutes, 0, 59);
  seconds = math::Clamp(seconds, 0, 59);

  auto total_seconds{static_cast<f32>(hours * 3600 + minutes * 60 + seconds)};
  SetDayDurationSeconds(total_seconds);
}

s32 EnvironmentManager::GetEffectiveDayDurationHoursPart() const {
  auto total_seconds{static_cast<s32>(GetEffectiveDayDurationSeconds())};
  return total_seconds / 3600;
}

s32 EnvironmentManager::GetEffectiveDayDurationMinutesPart() const {
  auto total_seconds{static_cast<s32>(GetEffectiveDayDurationSeconds())};
  return (total_seconds % 3600) / 60;
}

s32 EnvironmentManager::GetEffectiveDayDurationSecondsPart() const {
  auto total_seconds{static_cast<s32>(GetEffectiveDayDurationSeconds())};
  return total_seconds % 60;
}

void EnvironmentManager::SetEffectiveDayDurationHms(s32 hours, s32 minutes,
                                                    s32 seconds) {
  hours = math::Max(hours, 0);
  minutes = math::Clamp(minutes, 0, 59);
  seconds = math::Clamp(seconds, 0, 59);

  auto total_seconds{static_cast<f32>(hours * 3600 + minutes * 60 + seconds)};
  SetEffectiveDayDurationSeconds(total_seconds);
}

// Playback window.
void EnvironmentManager::SetUseDayWindow(bool use_day_window) {
  use_day_window_ = use_day_window;
}

bool EnvironmentManager::IsUsingDayWindow() const { return use_day_window_; }

void EnvironmentManager::SetDayWindowStartHours(f32 hours) {
  day_window_start_hours_ = math::Clamp(hours, .0f, 24.0f);
}

f32 EnvironmentManager::GetDayWindowStartHours() const {
  return day_window_start_hours_;
}

void EnvironmentManager::SetDayWindowEndHours(f32 hours) {
  day_window_end_hours_ = math::Clamp(hours, .0f, 24.0f);
}

f32 EnvironmentManager::GetDayWindowEndHours() const {
  return day_window_end_hours_;
}

// Sun.
void EnvironmentManager::SetAzimuthOffsetRadians(f32 radians) {
  azimuth_offset_ = radians;
}

f32 EnvironmentManager::GetAzimuthOffsetRadians() const {
  return azimuth_offset_;
}

f32 EnvironmentManager::GetEffectiveTimeOfDayHours() const {
  if (!use_day_window_) {
    return time_of_day_hours_;
  }

  auto start{day_window_start_hours_};
  auto end{day_window_end_hours_};

  // Degenerate case.
  if (end <= start) {
    return start;
  }

  auto normalized_day{time_of_day_hours_ / 24.0f};
  return math::Lerp(start, end, normalized_day);
}

math::Vec3 EnvironmentManager::ComputeAmbientColor() const {
  auto time_hours{GetEffectiveTimeOfDayHours()};
  auto day_alpha{time_hours / 24.0f};
  auto solar_phase{day_alpha * math::kTwoPi - math::kHalfPi};

  constexpr f32 kNoonElevationDeg{60.0f};
  auto max_elevation{math::ConvertToRadians(kNoonElevationDeg)};
  auto elevation{static_cast<f32>(max_elevation * math::Sin(solar_phase))};
  auto sun_height{
      math::Clamp((math::Sin(elevation) + .15f) / 1.15f, .0f, 1.0f)};

  math::Vec3 night{.01f, .015f, .03f};
  math::Vec3 dawn{.20f, .10f, .03f};
  math::Vec3 day{.18f, .20f, .24f};

  night *= .6f;

  if (sun_height < .35f) {
    auto t{sun_height / .35f};
    return math::Lerp(night, dawn, t);
  }

  auto t{(sun_height - .35f) / .65f};
  return math::Lerp(dawn, day, t);
}

void EnvironmentManager::AdvanceTime(f32 delta_seconds) {
  if (is_time_frozen_) {
    return;
  }

  if (day_duration_seconds_ <= .0f) {
    return;
  }

  time_of_day_hours_ +=
      delta_seconds * 24.0f / day_duration_seconds_ * day_time_scale_;
  time_of_day_hours_ = math::Wrap(time_of_day_hours_, 24.0f);
}

math::Vec3 EnvironmentManager::GenerateSunDirection(f32 azimuth,
                                                    f32 elevation) const {
  auto cos_elev{math::Cos(elevation)};

  // Map angles onto a unit sphere.
  math::Vec3 dir{};
  dir.x = cos_elev * math::Cos(azimuth);
  dir.y = math::Sin(elevation);
  dir.z = cos_elev * math::Sin(azimuth);
  math::Normalize(dir);
  return dir;
}
}  // namespace scene
}  // namespace comet
