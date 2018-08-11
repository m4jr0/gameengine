// Copyright 2018 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef KOMA_CORE_TIME_MANAGER_HPP_
#define KOMA_CORE_TIME_MANAGER_HPP_

#include <chrono>

namespace koma {
class TimeManager {
 public:
  static double GetRealNow();

  virtual ~TimeManager() = default;

  double GetNow();
  void Initialize();
  void Update();
  void Stop() noexcept;
  void Normalize() noexcept;

  const double time_delta() const noexcept { return this->time_delta_; }
  const double current_time() const noexcept { return this->current_time_; }
  const float time_scale() const noexcept { return this->time_scale_; }
  void time_scale(float time_scale) noexcept 
    { this->time_scale_ = time_scale; }

 private:
  double current_time_ = 0.0;
  double previous_time_ = 0.0;
  double time_delta_ = 0.0;
  float time_scale_ = 1.0f;
};
};  // namespace koma

#endif  // KOMA_CORE_TIME_MANAGER_HPP_
