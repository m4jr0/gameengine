// Copyright 2018 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include <chrono>
#include <iostream>

#include "time_manager.hpp"

namespace koma {
double TimeManager::GetNow() {
  auto current_time_ = std::chrono::system_clock::now();

  auto duration_in_seconds = std::chrono::duration<double>(
    current_time_.time_since_epoch()
  );

  return duration_in_seconds.count() * 1000;
}

void TimeManager::Initialize() {
  this->previous_time_ = TimeManager::GetNow();
}

void TimeManager::Update() {
  this->current_time_ = TimeManager::GetNow();
  this->time_delta_ = this->current_time_ - this->previous_time_;
  this->previous_time_ = this->current_time_;

  this->time_counter_ += this->time_delta_;

  if (this->time_counter_ > 1000) {
    this->NotifyObservers("second");
    this->time_counter_ = 0.0;
  }
}

void TimeManager::Stop() {
  this->NotifyObservers("stop");
}

void TimeManager::Resume() {
  this->NotifyObservers("run");
}

const double TimeManager::time_delta() const {
  return this->time_delta_;
}

const double TimeManager::current_time() const {
  return this->current_time_;
}
};  // namespace koma