// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "time_manager.h"

#include "comet/utils/date.h"

namespace comet {
namespace time {
f64 TimeManager::GetRealNow() { return utils::date::GetNow(); }

f64 TimeManager::GetNow() {
  const auto time_delta{GetRealNow() - previous_time_};
  const auto time_to_add{time_delta * time_scale_};

  return previous_time_ + time_to_add;
}

void TimeManager::Initialize() { previous_time_ = GetRealNow(); }

void TimeManager::Update() {
  current_time_ = GetNow();
  time_delta_ = current_time_ - previous_time_;
  previous_time_ = current_time_;
}

void TimeManager::Destroy() {}

void TimeManager::Stop() noexcept { SetTimeScale(0.0f); }

void TimeManager::Normalize() noexcept { SetTimeScale(1.0f); }

const f64 TimeManager::GetTimeDelta() const noexcept { return time_delta_; }

const f64 TimeManager::GetCurrentTime() const noexcept { return current_time_; }

const f32 TimeManager::GetTimeScale() const noexcept { return time_scale_; }

void TimeManager::SetTimeScale(f32 time_scale) noexcept {
  time_scale_ = time_scale;
}
}  // namespace time
}  // namespace comet
