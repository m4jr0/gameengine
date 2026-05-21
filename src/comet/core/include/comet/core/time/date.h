// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_CORE_TIME_DATE_H_
#define COMET_CORE_TIME_DATE_H_

// External. ///////////////////////////////////////////////////////////////////
#include <chrono>
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/essentials.h"

namespace comet {  // >:3 Move to time if time is in core now?
namespace internal {
std::chrono::time_point<std::chrono::system_clock> GetChronoTimePoint(
    const std::time_t&);
f64 GetSeconds(const std::chrono::time_point<std::chrono::system_clock>& time);
}  // namespace internal

f64 GetNow();
u64 GetTimestampSeconds();
u64 GetTimestampMilliSeconds();
u64 GetTimestampNanoSeconds();
}  // namespace comet

#endif  // COMET_CORE_TIME_DATE_H_
