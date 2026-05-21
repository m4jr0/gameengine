// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_CORE_ALGORITHM_INPLACE_MERGE_H_
#define COMET_CORE_ALGORITHM_INPLACE_MERGE_H_

// External. ///////////////////////////////////////////////////////////////////
#include <iterator>
#include <utility>
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/algorithm/comparator.h"
#include "comet/core/algorithm/iterator_utils.h"
#include "comet/core/essentials.h"
#include "comet/core/memory/allocator/allocator.h"
#include "comet/core/type/array.h"

namespace comet {
template <typename Iterator, typename Comparer = Less>
void InplaceMerge(Iterator begin, Iterator middle, Iterator end,
                  memory::Allocator* allocator,
                  Comparer comparer = Comparer{}) {
  Array<typename std::iterator_traits<Iterator>::value_type> tmp{allocator};
  tmp.Reserve(static_cast<usize>(Distance(begin, end)));

  Iterator left{begin};
  Iterator right{middle};

  while (left != middle && right != end) {
    if (comparer(*left, *right)) {
      tmp.EmplaceLast(*left);
      ++left;
    } else {
      tmp.EmplaceLast(*right);
      ++right;
    }
  }

  while (left != middle) {
    tmp.EmplaceLast(*left);
    ++left;
  }

  while (right != end) {
    tmp.EmplaceLast(*right);
    ++right;
  }

  std::move(tmp.begin(), tmp.end(), begin);
}
}  // namespace comet

#endif  // COMET_CORE_ALGORITHM_INPLACE_MERGE_H_
