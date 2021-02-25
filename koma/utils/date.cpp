// Copyright 2021 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "date.hpp"

#ifdef _WIN32
// Allow debugging memory leaks on Windows.
#include <debug_windows.hpp>
#endif  // _WIN32

namespace koma {
namespace date {
double GetNow() {
  auto current_time = std::chrono::system_clock::now();

  return GetDouble(current_time);
}

std::chrono::time_point<std::chrono::system_clock> GetChronoTimePoint(
  const std::time_t &time) {
  return std::chrono::system_clock::from_time_t(time);
}

double GetDouble(
  const std::chrono::time_point<std::chrono::system_clock> &time) {
  auto duration_in_seconds =
    std::chrono::duration<double>(time.time_since_epoch());

  return duration_in_seconds.count() * 1000;
}

double GetDouble(const std::time_t &time) {
  return GetDouble(GetChronoTimePoint(time));
}
}  // namespace date
}  // namespace koma
