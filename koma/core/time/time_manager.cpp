// Copyright 2018 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Allows debugging memory leaks.
// TODO(m4jr0): Find a better solution.
#include "../../debug.hpp"

#include <chrono>

#include "time_manager.hpp"

namespace koma {
double TimeManager::GetRealNow() {
  auto current_time_ = std::chrono::system_clock::now();

  auto duration_in_seconds =
      std::chrono::duration<double>(current_time_.time_since_epoch());

  return duration_in_seconds.count() * 1000;
}

double TimeManager::GetNow() {
  double time_delta = this->GetRealNow() - this->previous_time_;
  double time_to_add = time_delta * this->time_scale_;

  return this->previous_time_ + time_to_add;
}

void TimeManager::Initialize() { this->previous_time_ = this->GetRealNow(); }

void TimeManager::Update() {
  this->current_time_ = this->GetNow();
  this->time_delta_ = this->current_time_ - this->previous_time_;
  this->previous_time_ = this->current_time_;
}

void TimeManager::Stop() noexcept { this->time_scale(0.0f); }

void TimeManager::Normalize() noexcept { this->time_scale(1.0f); }

const double TimeManager::time_delta() const noexcept {
  return this->time_delta_;
}

const double TimeManager::current_time() const noexcept {
  return this->current_time_;
}

const float TimeManager::time_scale() const noexcept {
  return this->time_scale_;
}

void TimeManager::time_scale(float time_scale) noexcept {
  this->time_scale_ = time_scale;
}
};  // namespace koma
