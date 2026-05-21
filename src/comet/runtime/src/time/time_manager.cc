// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "runtime/time/time_manager.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/conf/configuration_manager.h"
#include "comet/core/date.h"
#include "comet/math/math_scalar.h"

namespace comet {
namespace time {
TimeManager& TimeManager::Get() {
  static TimeManager singleton{};
  return singleton;
}

f64 TimeManager::GetRealNow() const { return comet::GetNow(); }

f64 TimeManager::GetNow() const { return current_time_; }

void TimeManager::Update() {
  real_current_time_ = GetRealNow();
  unscaled_delta_time_ = real_current_time_ - real_previous_time_;

  if (unscaled_delta_time_ < .0) {
    unscaled_delta_time_ = .0;
  }

  delta_time_ = unscaled_delta_time_ * static_cast<f64>(time_scale_);
  current_time_ += delta_time_;

  real_previous_time_ = real_current_time_;
}

void TimeManager::Stop() noexcept { SetTimeScale(.0f); }

void TimeManager::Normalize() noexcept { SetTimeScale(1.0f); }

f64 TimeManager::GetFixedDeltaTime() const noexcept {
  return fixed_delta_time_;
}

f64 TimeManager::GetDeltaTime() const noexcept { return delta_time_; }

f64 TimeManager::GetUnscaledDeltaTime() const noexcept {
  return unscaled_delta_time_;
}

f64 TimeManager::GetCurrentTime() const noexcept { return current_time_; }

f64 TimeManager::GetRealTime() const noexcept { return real_current_time_; }

f64 TimeManager::GetUptime() const noexcept {
  return real_current_time_ - real_start_time_;
}

f32 TimeManager::GetTimeScale() const noexcept { return time_scale_; }

void TimeManager::SetTimeScale(f32 time_scale) noexcept {
  time_scale_ = time_scale < .0f ? .0f : time_scale;
}

void TimeManager::OnInitialize() {
  real_current_time_ = GetRealNow();
  real_previous_time_ = real_current_time_;
  real_start_time_ = real_current_time_;

  unscaled_delta_time_ = .0;
  current_time_ = .0;
  delta_time_ = .0;
  time_scale_ = 1.0f;

  // Round up to two decimal places.
  fixed_delta_time_ =
      (math::Ceil(COMET_CONF_F64(conf::kCoreMsPerUpdate) * 100.0) / 100.0) /
      1000.0;
}

void TimeManager::OnShutdown() {
  fixed_delta_time_ = .01666;
  real_current_time_ = .0;
  real_previous_time_ = .0;
  unscaled_delta_time_ = .0;
  current_time_ = .0;
  delta_time_ = .0;
  time_scale_ = 1.0f;
}
}  // namespace time
}  // namespace comet