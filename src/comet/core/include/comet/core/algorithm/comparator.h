// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_ALGORITHM_COMPARATOR_H_
#define COMET_COMET_CORE_ALGORITHM_COMPARATOR_H_

#include "comet/core/essentials.h"

namespace comet {
struct Less {
  template <typename T, typename U>
  constexpr bool operator()(const T& lhs, const U& rhs) const {
    return lhs < rhs;
  }
};
}  // namespace comet

#endif  // COMET_COMET_CORE_ALGORITHM_COMPARATOR_H_
