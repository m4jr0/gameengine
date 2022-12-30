// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "time_manager.h"

#include "comet/utils/date.h"

namespace comet {
namespace time {
f64 TimeManager::GetRealNow() { return utils::date::GetNow(); }

f64 TimeManager::GetNow() {
  const auto delta_time{GetRealNow() - previous_time_};
  const auto time_to_add{delta_time * time_scale_};

  return previous_time_ + time_to_add;
}

void TimeManager::Initialize() { previous_time_ = GetRealNow(); }

void TimeManager::Update() {
  current_time_ = GetNow();
  delta_time_ = current_time_ - previous_time_;
  previous_time_ = current_time_;
}

void TimeManager::Destroy() {}

void TimeManager::Stop() noexcept { SetTimeScale(0.0f); }

void TimeManager::Normalize() noexcept { SetTimeScale(1.0f); }

const f64 TimeManager::GetFixedDeltaTime() const noexcept {
  return fixed_delta_time_;
}

const f64 TimeManager::GetDeltaTime() const noexcept { return delta_time_; }

const f64 TimeManager::GetCurrentTime() const noexcept { return current_time_; }

const f32 TimeManager::GetTimeScale() const noexcept { return time_scale_; }

void TimeManager::SetFixedDeltaTime(f64 fixed_delta_time) noexcept {
  fixed_delta_time_ = fixed_delta_time;
}

void TimeManager::SetTimeScale(f32 time_scale) noexcept {
  time_scale_ = time_scale;
}
}  // namespace time
}  // namespace comet
