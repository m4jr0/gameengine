// Copyright 2021 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_CORE_TIME_MANAGER_HPP_
#define COMET_CORE_TIME_MANAGER_HPP_

#include "comet_precompile.hpp"
#include "core/manager.hpp"

namespace comet {
class TimeManager : public Manager {
 public:
  TimeManager() = default;
  TimeManager(const TimeManager&) = delete;
  TimeManager(TimeManager&&) = delete;
  TimeManager& operator=(const TimeManager&) = delete;
  TimeManager& operator=(TimeManager&&) = delete;
  virtual ~TimeManager() = default;

  static double GetRealNow();
  double GetNow();
  void Initialize() override;
  void Update() override;
  void Stop() noexcept;
  void Normalize() noexcept;

  const double time_delta() const noexcept;
  const double current_time() const noexcept;
  const float time_scale() const noexcept;
  void time_scale(float time_scale) noexcept;

 private:
  double current_time_ = 0.0;
  double previous_time_ = 0.0;
  double time_delta_ = 0.0;
  float time_scale_ = 1.0f;
};
}  // namespace comet

#endif  // COMET_CORE_TIME_MANAGER_HPP_
