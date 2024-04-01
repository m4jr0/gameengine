// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_MEMORY_ALLOCATOR_STL_TWO_FRAME_ALLOCATOR_H_
#define COMET_COMET_CORE_MEMORY_ALLOCATOR_STL_TWO_FRAME_ALLOCATOR_H_

#include "comet/core/essentials.h"
#include "comet/core/memory/memory_manager.h"

namespace comet {
namespace memory {
template <class T>
class two_frame_allocator {
 public:
  using value_type = T;

  two_frame_allocator() = default;

  template <class U>
  constexpr two_frame_allocator(const two_frame_allocator<U>&) noexcept {}

  T* allocate(std::size_t size) {
    return static_cast<T*>(
        memory::MemoryManager::Get().GetTwoFrameAllocator().Allocate(
            size * sizeof(T), std::alignment_of_v<T>));
  }

  T* allocate_one() { return allocate(1); }

  void deallocate(T* const, std::size_t) noexcept {
    // Do nothing. Data will be purged at the end of the frame after the next
    // one.
  }

  void deallocate_one(T* const) noexcept {
    // Do nothing. Data will be purged at the end of the frame.
  }
};
}  // namespace memory
}  // namespace comet
#endif  // COMET_COMET_CORE_MEMORY_ALLOCATOR_STL_TWO_FRAME_ALLOCATOR_H_
