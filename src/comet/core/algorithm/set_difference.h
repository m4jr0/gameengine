// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_ALGORITHM_SET_DIFFERENCE_H_
#define COMET_COMET_CORE_ALGORITHM_SET_DIFFERENCE_H_

#include "comet/core/essentials.h"

namespace comet {
template <typename InIt1, typename InIt2, typename OutIt,
          typename Comparer = Less>
OutIt SetDifference(InIt1 first1, InIt1 last1, InIt2 first2, InIt2 last2,
                    OutIt dst, Comparer comparer = Comparer{}) {
  while (first1 != last1) {
    if (first2 == last2) {
      while (first1 != last1) {
        *dst = *first1;
        ++dst;
        ++first1;
      }

      break;
    }

    if (comparer(*first1, *first2)) {
      *dst = *first1;
      ++dst;
      ++first1;
    } else if (comparer(*first2, *first1)) {
      ++first2;
    } else {
      ++first1;
      ++first2;
    }
  }

  return dst;
}
}  // namespace comet

#endif  // COMET_COMET_CORE_ALGORITHM_SET_DIFFERENCE_H_
