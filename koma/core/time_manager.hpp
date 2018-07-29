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
  void Stop();
  void Normalize();

  const double time_delta() const;
  const double current_time() const;
  const float time_scale() const;
  void time_scale(float);

 private:
  double current_time_ = 0.0;
  double previous_time_ = 0.0;
  double time_delta_ = 0.0;
  float time_scale_ = 1.0f;
};
};  // namespace koma

#endif  // KOMA_CORE_TIME_MANAGER_HPP_
