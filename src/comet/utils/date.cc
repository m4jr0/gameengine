// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "date.h"

namespace comet {
namespace utils {
namespace date {
f64 GetNow() {
  const auto current_time{std::chrono::system_clock::now()};
  return GetMilliseconds(current_time);
}

std::chrono::time_point<std::chrono::system_clock> GetChronoTimePoint(
    const std::time_t& time) {
  return std::chrono::system_clock::from_time_t(time);
}

f64 GetMilliseconds(
    const std::chrono::time_point<std::chrono::system_clock>& time) {
  const auto duration_in_seconds{
      std::chrono::duration<f64>(time.time_since_epoch())};

  return duration_in_seconds.count() * 1000;
}

f64 GetMilliseconds(const std::time_t& time) {
  return GetMilliseconds(GetChronoTimePoint(time));
}
}  // namespace date
}  // namespace utils
}  // namespace comet
