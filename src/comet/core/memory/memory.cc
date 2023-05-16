// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "memory.h"

#include "comet/core/string.h"

namespace comet {
std::string GetMemorySizeString(uindex size) {
  constexpr std::array kUnits{"bytes", "KiB", "MiB", "GiB", "TiB",
                              "PiB",   "EiB", "ZiB", "YiB"};
  uindex l{0};
  auto n{static_cast<f64>(size)};

  while (n >= 1024 && ++l) {
    n = n / 1024;
  }

  COMET_ASSERT(l < kUnits.size(), "Size is too big: ", size, "!");
  auto str{ConvertToStr(n, n < 10 && l > 0 ? 1 : 0)};
  str.reserve(str.size() + 6);
  str += ' ';
  str += kUnits[l];
  return str;
}
}  // namespace comet
