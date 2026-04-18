// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_SCENE_ENVIRONMENT_MANAGER_H_
#define COMET_COMET_SCENE_ENVIRONMENT_MANAGER_H_

#include "comet/core/essentials.h"
#include "comet/core/frame/frame_packet.h"
#include "comet/core/manager.h"
#include "comet/math/geometry.h"

namespace comet {
namespace scene {
class EnvironmentManager : public Manager {
 public:
  static EnvironmentManager& Get();

  void Update(frame::FramePacket* packet);

  // Time controls.
  void SetTimeFrozen(bool is_frozen);
  bool IsTimeFrozen() const;

  void SetTimeOfDayHours(f32 hours);
  f32 GetTimeOfDayHours() const;

  void SetDayDurationSeconds(f32 seconds);
  f32 GetDayDurationSeconds() const;

  void SetDayTimeScale(f32 scale);
  f32 GetDayTimeScale() const;

  void SetDayAccelerationFactor(f32 factor);
  f32 GetDayAccelerationFactor() const;

  f32 GetEffectiveDayDurationSeconds() const;
  f32 GetEffectiveDayAccelerationFactor() const;
  void SetEffectiveDayDurationSeconds(f32 seconds);
  void SetEffectiveDayAccelerationFactor(f32 factor);

  s32 GetDayDurationHoursPart() const;
  s32 GetDayDurationMinutesPart() const;
  s32 GetDayDurationSecondsPart() const;
  void SetDayDurationHms(s32 hours, s32 minutes, s32 seconds);

  s32 GetEffectiveDayDurationHoursPart() const;
  s32 GetEffectiveDayDurationMinutesPart() const;
  s32 GetEffectiveDayDurationSecondsPart() const;
  void SetEffectiveDayDurationHms(s32 hours, s32 minutes, s32 seconds);

  // Playback window.
  void SetUseDayWindow(bool use_day_window);
  bool IsUsingDayWindow() const;

  void SetDayWindowStartHours(f32 hours);
  f32 GetDayWindowStartHours() const;

  void SetDayWindowEndHours(f32 hours);
  f32 GetDayWindowEndHours() const;

  // Sun.
  void SetAzimuthOffsetRadians(f32 radians);
  f32 GetAzimuthOffsetRadians() const;

  f32 GetEffectiveTimeOfDayHours() const;
  math::Vec3 ComputeAmbientColor() const;

 protected:
  void OnInitialize() override;
  void OnShutdown() override;

 private:
  static inline constexpr f32 kRealDayDurationSeconds{86400.0f};

  void AdvanceTime(f32 delta_seconds);

  bool is_time_frozen_{false};

  f32 time_of_day_hours_{12.0f};
  f32 day_duration_seconds_{25.0f * 60.0f};  // 25 minutes by default.
  f32 day_time_scale_{1.0f};

  // Playback window.
  f32 day_window_start_hours_{.0f};
  f32 day_window_end_hours_{24.0f};
  bool use_day_window_{false};

  // Sun properties.
  f32 azimuth_offset_{static_cast<f32>(-math::kHalfPi)};

 private:
  rendering::LightHandle sun_light_{};
  math::Vec3 GenerateSunDirection(f32 azimuth, f32 elevation) const;
};
}  // namespace scene
}  // namespace comet

#endif  // COMET_COMET_SCENE_ENVIRONMENT_MANAGER_H_
