// Copyright 2021 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "time_manager.h"

#include "comet/utils/date.h"

#ifdef _WIN32
#include "debug_windows.h"
#endif  // _WIN32

namespace comet {
namespace time {
double TimeManager::GetRealNow() { return utils::date::GetNow(); }

double TimeManager::GetNow() {
  const auto time_delta = GetRealNow() - previous_time_;
  const auto time_to_add = time_delta * time_scale_;

  return previous_time_ + time_to_add;
}

void TimeManager::Initialize() { previous_time_ = GetRealNow(); }

void TimeManager::Update() {
  current_time_ = GetNow();
  time_delta_ = current_time_ - previous_time_;
  previous_time_ = current_time_;
}

void TimeManager::Stop() noexcept { time_scale(0.0f); }

void TimeManager::Normalize() noexcept { time_scale(1.0f); }

const double TimeManager::time_delta() const noexcept { return time_delta_; }

const double TimeManager::current_time() const noexcept {
  return current_time_;
}

const float TimeManager::time_scale() const noexcept { return time_scale_; }

void TimeManager::time_scale(float time_scale) noexcept {
  time_scale_ = time_scale;
}
}  // namespace time
}  // namespace comet
