// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_MEMORY_ALLOCATOR_STL_ONE_FRAME_ALLOCATOR_H_
#define COMET_COMET_CORE_MEMORY_ALLOCATOR_STL_ONE_FRAME_ALLOCATOR_H_

#include "comet_precompile.h"

#include "comet/core/memory/memory.h"
#include "comet/core/memory/memory_manager.h"

namespace comet {
template <class T>
class one_frame_allocator {
 public:
  using value_type = T;

  one_frame_allocator() = default;

  template <class U>
  constexpr one_frame_allocator(const one_frame_allocator<U>&) noexcept {}

  T* allocate(std::size_t size) {
    return static_cast<T*>(
        memory::MemoryManager::Get().GetOneFrameAllocator().Allocate(
            size * sizeof(T), std::alignment_of_v<T>));
  }

  T* allocate_one() { return allocate(1); }

  void deallocate(T* const p, std::size_t size) noexcept {
    // Do nothing. Data will be purged at the end of the frame.
  }

  void deallocate_one(T* const p) noexcept {
    // Do nothing. Data will be purged at the end of the frame.
  }
};
}  // namespace comet
#endif  // COMET_COMET_CORE_MEMORY_ALLOCATOR_STL_ONE_FRAME_ALLOCATOR_H_
