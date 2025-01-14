// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_ALGORITHM_SORT_H_
#define COMET_COMET_CORE_ALGORITHM_SORT_H_

#include "comet/core/algorithm/algorithm_common.h"
#include "comet/core/algorithm/inplace_merge.h"
#include "comet/core/essentials.h"

namespace comet {
template <typename Iterator, typename Comparer = Less>
void Sort(Iterator begin, Iterator end, Comparer comparer = Comparer{}) {
  if (Distance(begin, end) <= 1) {
    return;
  }

  Iterator middle{begin};
  Advance(middle, Distance(begin, end) / 2);

  Sort(begin, middle, comparer);
  Sort(middle, end, comparer);

  InplaceMerge(begin, middle, end, comparer);
}
}  // namespace comet

#endif  // COMET_COMET_CORE_ALGORITHM_SORT_H_
