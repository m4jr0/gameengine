// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_ALGORITHM_ALGORITHM_COMMON_H_
#define COMET_COMET_CORE_ALGORITHM_ALGORITHM_COMMON_H_

#include "comet/core/essentials.h"

namespace comet {
struct Less {
  template <typename T, typename U>
  constexpr bool operator()(const T& lhs, const U& rhs) const {
    return lhs < rhs;
  }
};

template <typename Iterator>
usize Distance(Iterator first, Iterator last) {
  usize count{0};

  while (first != last) {
    ++first;
    ++count;
  }

  return count;
}

template <typename Iterator, typename Distance>
void Advance(Iterator& it, Distance distance) {
  if (distance > 0) {
    while (distance-- != 0) {
      ++it;
    }
  } else {
    while (distance++ != 0) {
      --it;
    }
  }
}
}  // namespace comet

#endif  // COMET_COMET_CORE_ALGORITHM_ALGORITHM_COMMON_H_
