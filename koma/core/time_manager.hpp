// Copyright 2018 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef KOMA_CORE_TIME_MANAGER_HPP_
#define KOMA_CORE_TIME_MANAGER_HPP_

#include <chrono>

#include "../utils/subject.hpp"

namespace koma {
class TimeManager : public Subject {
 public:
  virtual ~TimeManager() = default;
  static double GetNow();
  void Initialize();
  void Update();
  const double time_delta() const;
  const double current_time() const;

 private:
  double current_time_ = 0.0;
  double previous_time_ = 0.0;
  double time_delta_ = 0.0;
  double time_counter_ = 0.0;
};
};  // namespace koma

#endif  // KOMA_CORE_TIME_MANAGER_HPP_
