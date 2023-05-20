// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_TYPE_STRUCTURE_STL_TYPES_H_
#define COMET_COMET_CORE_TYPE_STRUCTURE_STL_TYPES_H_

#include "comet_precompile.h"

#include "comet/core/memory/allocator/stl/one_frame_allocator.h"
#include "comet/core/memory/allocator/stl/two_frame_allocator.h"

namespace comet {
// One-frame types.
template <class T>
using one_frame_vector = std::vector<T, one_frame_allocator<T>>;

// Two-frame types.
template <class T>
using two_frame_vector = std::vector<T, two_frame_allocator<T>>;
}  // namespace comet

#endif  // COMET_COMET_CORE_TYPE_STRUCTURE_STL_TYPES_H_
