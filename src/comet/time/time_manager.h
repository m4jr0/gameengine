// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_TIME_TIME_MANAGER_H_
#define COMET_COMET_TIME_TIME_MANAGER_H_

#include "comet_precompile.h"

#include "comet/core/manager.h"

#ifdef COMET_MSVC
#undef GetCurrentTime
#endif  // COMET_MSVC

namespace comet {
namespace time {
using Interpolation = f64;

class TimeManager : public Manager {
 public:
  static TimeManager& Get();

  TimeManager() = default;
  TimeManager(const TimeManager&) = delete;
  TimeManager(TimeManager&&) = delete;
  TimeManager& operator=(const TimeManager&) = delete;
  TimeManager& operator=(TimeManager&&) = delete;
  virtual ~TimeManager() = default;

  f64 GetRealNow();
  f64 GetNow();
  void Initialize() override;
  void Shutdown() override;
  void Update();
  void Stop() noexcept;
  void Normalize() noexcept;

  f64 GetFixedDeltaTime() const noexcept;
  f64 GetDeltaTime() const noexcept;
  f64 GetCurrentTime() const noexcept;
  f32 GetTimeScale() const noexcept;
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
