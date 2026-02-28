// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_ALGORITHM_INPLACE_MERGE_H_
#define COMET_COMET_CORE_ALGORITHM_INPLACE_MERGE_H_

// External. ///////////////////////////////////////////////////////////////////
#include <iterator>
#include <utility>
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/algorithm/algorithm_common.h"
#include "comet/core/essentials.h"
#include "comet/core/frame/frame_utils.h"
#include "comet/core/type/array.h"

namespace comet {
template <typename Iterator, typename Comparer = Less>
void InplaceMerge(Iterator begin, Iterator middle, Iterator end,
                  Comparer comparer = Comparer{}) {
  frame::FrameArray<typename std::iterator_traits<Iterator>::value_type> tmp{};
  tmp.Reserve(static_cast<usize>(Distance(begin, end)));

  Iterator left{begin};
  Iterator right{middle};

  while (left != middle && right != end) {
    if (comparer(*left, *right)) {
      tmp.EmplaceBack(*left);
      ++left;
    } else {
      tmp.EmplaceBack(*right);
      ++right;
    }
  }

  while (left != middle) {
    tmp.EmplaceBack(*left);
    ++left;
  }

  while (right != end) {
    tmp.EmplaceBack(*right);
    ++right;
  }

  std::move(tmp.begin(), tmp.end(), begin);
}
}  // namespace comet

#endif  // COMET_COMET_CORE_ALGORITHM_INPLACE_MERGE_H_
