// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_TYPE_STL_TYPES_H_
#define COMET_COMET_CORE_TYPE_STL_TYPES_H_

#include <vector>

#include "comet/core/essentials.h"
#include "comet/core/frame/stl/one_frame_allocator.h"

namespace comet {
template <typename T>
using one_frame_vector = std::vector<T, frame::one_cycle_frame_allocator<T>>;
}  // namespace comet

#endif  // COMET_COMET_CORE_TYPE_STL_TYPES_H_
