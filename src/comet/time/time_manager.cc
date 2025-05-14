// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "comet_pch.h"

#include "time_manager.h"

#include "comet/core/conf/configuration_manager.h"
#include "comet/core/date.h"
#include "comet/math/math_common.h"

namespace comet {
namespace time {
TimeManager& TimeManager::Get() {
  static TimeManager singleton{};
  return singleton;
}

f64 TimeManager::GetRealNow() { return comet::GetNow(); }

f64 TimeManager::GetNow() {
  const auto delta_time{GetRealNow() - previous_time_};
  const auto time_to_add{delta_time * time_scale_};

  return previous_time_ + time_to_add;
}

void TimeManager::Initialize() {
  Manager::Initialize();
  previous_time_ = GetRealNow();

  // Round up to two decimal places.
  fixed_delta_time_ =
      (math::Ceil(COMET_CONF_F64(conf::kCoreMsPerUpdate) * 100.0) / 100.0) /
      1000.0f;
}

void TimeManager::Shutdown() {
  fixed_delta_time_ = .01666f;
  current_time_ = 0.0;
  previous_time_ = 0.0;
  delta_time_ = 0.0;
  time_scale_ = 1.0f;
  Manager::Shutdown();
}

void TimeManager::Update() {
  current_time_ = GetNow();
  delta_time_ = current_time_ - previous_time_;
  previous_time_ = current_time_;
}

void TimeManager::Stop() noexcept { SetTimeScale(0.0f); }

void TimeManager::Normalize() noexcept { SetTimeScale(1.0f); }

f64 TimeManager::GetFixedDeltaTime() const noexcept {
  return fixed_delta_time_;
}

f64 TimeManager::GetDeltaTime() const noexcept { return delta_time_; }

f64 TimeManager::GetCurrentTime() const noexcept { return current_time_; }

f32 TimeManager::GetTimeScale() const noexcept { return time_scale_; }

void TimeManager::SetTimeScale(f32 time_scale) noexcept {
  time_scale_ = time_scale;
}
}  // namespace time
}  // namespace comet
