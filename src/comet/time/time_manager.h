// Copyright 2021 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_TIME_TIME_MANAGER_H_
#define COMET_COMET_TIME_TIME_MANAGER_H_

#include "comet/core/manager.h"
#include "comet_precompile.h"

namespace comet {
namespace time {
class TimeManager : public core::Manager {
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

  const double GetTimeDelta() const noexcept;
  const double GetCurrentTime() const noexcept;
  const float GetTimeScale() const noexcept;
  void SetTimeScale(float time_scale) noexcept;

 private:
  double current_time_ = 0.0;
  double previous_time_ = 0.0;
  double time_delta_ = 0.0;
  float time_scale_ = 1.0f;
};
}  // namespace time
}  // namespace comet

#endif  // COMET_COMET_TIME_TIME_MANAGER_H_
