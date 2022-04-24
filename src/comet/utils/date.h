// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_UTILS_DATE_H_
#define COMET_COMET_UTILS_DATE_H_

#include "comet_precompile.h"

namespace comet {
namespace utils {
namespace date {
f64 GetNow();

std::chrono::time_point<std::chrono::system_clock> GetChronoTimePoint(
    const std::time_t&);

f64 GetDouble(const std::chrono::time_point<std::chrono::system_clock>& time);
f64 GetDouble(const std::time_t& time);
}  // namespace date
}  // namespace utils
}  // namespace comet

#endif  // COMET_COMET_UTILS_DATE_H_
