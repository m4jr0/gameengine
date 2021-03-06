// Copyright 2021 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef KOMA_UTILS_DATE_HPP_
#define KOMA_UTILS_DATE_HPP_

constexpr auto kLoggerKomaUtilsDate = "koma_utils";

#include <chrono>
#include <ctime>

namespace koma {
namespace date {
double GetNow();

std::chrono::time_point<std::chrono::system_clock> GetChronoTimePoint(
    const std::time_t &);

double GetDouble(const std::chrono::time_point<std::chrono::system_clock> &);
double GetDouble(const std::time_t &);
}  // namespace date
}  // namespace koma

#endif  // KOMA_UTILS_DATE_HPP_
