// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "date.h"
////////////////////////////////////////////////////////////////////////////////

namespace comet {
namespace internal {
std::chrono::time_point<std::chrono::system_clock> GetChronoTimePoint(
    const std::time_t& time) {
  return std::chrono::system_clock::from_time_t(time);
}

f64 GetSeconds(const std::chrono::time_point<std::chrono::system_clock>& time) {
  const auto duration_in_seconds{
      std::chrono::duration<f64>(time.time_since_epoch())};

  return duration_in_seconds.count();
}
}  // namespace internal

f64 GetNow() {
  const auto current_time{std::chrono::system_clock::now()};
  return internal::GetSeconds(current_time);
}

u64 GetTimestampSeconds() {
  auto now{std::chrono::system_clock::now()};
  auto now_s{
      std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch())
          .count()};
  return static_cast<u64>(now_s);
}

u64 GetTimestampMilliSeconds() {
  auto now{std::chrono::system_clock::now()};
  auto now_ms{std::chrono::duration_cast<std::chrono::milliseconds>(
                  now.time_since_epoch())
                  .count()};
  return static_cast<u64>(now_ms);
}

u64 GetTimestampNanoSeconds() {
  auto now{std::chrono::system_clock::now()};
  auto now_ns{std::chrono::duration_cast<std::chrono::nanoseconds>(
                  now.time_since_epoch())
                  .count()};
  return static_cast<u64>(now_ns);
}
}  // namespace comet
