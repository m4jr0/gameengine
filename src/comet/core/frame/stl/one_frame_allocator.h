// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_FRAME_STL_ONE_FRAME_ALLOCATOR_H_
#define COMET_COMET_CORE_FRAME_STL_ONE_FRAME_ALLOCATOR_H_

#include "comet/core/essentials.h"
#include "comet/core/frame/frame_allocator.h"

namespace comet {
namespace frame {
template <class T>
class one_cycle_frame_allocator {
 public:
  using value_type = T;

  one_cycle_frame_allocator() = default;

  template <class U>
  constexpr one_cycle_frame_allocator(
      const one_cycle_frame_allocator<U>&) noexcept {}

  T* allocate(std::size_t size) {
    return static_cast<T*>(
        COMET_FRAME_ALLOC_ALIGNED(size * sizeof(T), std::alignment_of_v<T>));
  }

  T* allocate_one() { return allocate(1); }

  void deallocate(T* const, std::size_t) noexcept {
    // Do nothing. Data will be purged at the end of the frame cycle.
  }

  void deallocate_one(T* const) noexcept {
    // Do nothing. Data will be purged at the end of the frame cycle.
  }
};
}  // namespace frame
}  // namespace comet
#endif  // COMET_COMET_CORE_FRAME_STL_ONE_FRAME_ALLOCATOR_H_
