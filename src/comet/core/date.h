// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_DATE_H_
#define COMET_COMET_CORE_DATE_H_

#include <chrono>

#include "comet/core/essentials.h"

namespace comet {
namespace internal {
std::chrono::time_point<std::chrono::system_clock> GetChronoTimePoint(
    const std::time_t&);
f64 GetMilliseconds(
    const std::chrono::time_point<std::chrono::system_clock>& time);
f64 GetMilliseconds(const std::time_t& time);
f64 GetMicroseconds(
    const std::chrono::time_point<std::chrono::system_clock>& time);
f64 GetMicroseconds(const std::time_t& time);
}  // namespace internal

f64 GetNow();
f64 GetPreciseNow();
u64 GetTimestampSeconds();
u64 GetTimestampMilliSeconds();
u64 GetTimestampNanoSeconds();
}  // namespace comet

#endif  // COMET_COMET_CORE_DATE_H_
