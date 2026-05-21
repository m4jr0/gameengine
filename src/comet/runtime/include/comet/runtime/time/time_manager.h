// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RUNTIME_TIME_TIME_MANAGER_H_
#define COMET_RUNTIME_TIME_TIME_MANAGER_H_

#include "comet/core/essentials.h"
#include "comet/core/manager.h"

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
  ~TimeManager() override = default;

  void Update();

  f64 GetRealNow() const;
  f64 GetNow() const;

  void Stop() noexcept;
  void Normalize() noexcept;

  f64 GetFixedDeltaTime() const noexcept;
  f64 GetDeltaTime() const noexcept;
  f64 GetUnscaledDeltaTime() const noexcept;
  f64 GetCurrentTime() const noexcept;
  f64 GetRealTime() const noexcept;
  f64 GetUptime() const noexcept;
  f32 GetTimeScale() const noexcept;

  void SetTimeScale(f32 time_scale) noexcept;

 protected:
  void OnInitialize() override;
  void OnShutdown() override;

 private:
  f64 fixed_delta_time_{.01666};  // 60 Hz refresh by default.

  f64 real_current_time_{.0};
  f64 real_previous_time_{.0};
  f64 unscaled_delta_time_{.0};
  f64 real_start_time_{.0};

  f64 current_time_{.0};
  f64 delta_time_{.0};

  f32 time_scale_{1.0f};
};
}  // namespace time
}  // namespace comet

#endif  // COMET_RUNTIME_TIME_TIME_MANAGER_H_