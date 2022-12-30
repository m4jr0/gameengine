// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_TIME_TIME_MANAGER_H_
#define COMET_COMET_TIME_TIME_MANAGER_H_

#include "comet_precompile.h"

namespace comet {
namespace time {
using Interpolation = f64;

class TimeManager {
 public:
  TimeManager() = default;
  TimeManager(const TimeManager&) = delete;
  TimeManager(TimeManager&&) = delete;
  TimeManager& operator=(const TimeManager&) = delete;
  TimeManager& operator=(TimeManager&&) = delete;
  ~TimeManager() = default;

  static f64 GetRealNow();
  f64 GetNow();
  void Initialize();
  void Update();
  void Destroy();
  void Stop() noexcept;
  void Normalize() noexcept;

  const f64 GetFixedDeltaTime() const noexcept;
  const f64 GetDeltaTime() const noexcept;
  const f64 GetCurrentTime() const noexcept;
  const f32 GetTimeScale() const noexcept;
  void SetFixedDeltaTime(f64 fixed_delta_time) noexcept;
  void SetTimeScale(f32 time_scale) noexcept;

 private:
  f64 fixed_delta_time_{16.66};  // 60 Hz refresh by default.
  f64 current_time_{0.0};
  f64 previous_time_{0.0};
  f64 delta_time_{0.0};
  f32 time_scale_{1.0f};
};
}  // namespace time
}  // namespace comet

#endif  // COMET_COMET_TIME_TIME_MANAGER_H_
