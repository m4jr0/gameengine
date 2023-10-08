// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_MEMORY_ALLOCATOR_FRAME_ALLOCATOR_H_
#define COMET_COMET_CORE_MEMORY_ALLOCATOR_FRAME_ALLOCATOR_H_

#include "comet_precompile.h"

#include "comet/core/conf/configuration_value.h"
#include "comet/core/memory/allocator/stack_allocator.h"

namespace comet {
namespace memory {
using OneFrameAllocator = StackAllocator;

class TwoFrameAllocator {
 public:
  TwoFrameAllocator() = delete;
  explicit TwoFrameAllocator(uindex total_capacity);
  TwoFrameAllocator(const TwoFrameAllocator&) = delete;
  TwoFrameAllocator(TwoFrameAllocator&&) = delete;
  TwoFrameAllocator& operator=(const TwoFrameAllocator&) = delete;
  TwoFrameAllocator& operator=(TwoFrameAllocator&&) = delete;
  ~TwoFrameAllocator() = default;

  void Initialize();
  void Destroy();
  void* Allocate(uindex size, u8 align = 0);
  void SwapFrames();
  void ClearCurrent();
  uindex GetCurrentSize() const noexcept;
  uindex GetTotalSize() const noexcept;

 private:
  u8 current_stack_{0};
  StackAllocator stacks_[2]{
      StackAllocator{conf::GetDefaultValue(conf::kCoreOneFrameAllocatorCapacity)
                             .u32_value /
                         2,
                     MemoryTag::TwoFrames},
      StackAllocator{conf::GetDefaultValue(conf::kCoreOneFrameAllocatorCapacity)
                             .u32_value /
                         2,
                     MemoryTag::TwoFrames}};
};
}  // namespace memory
}  // namespace comet

#endif  // COMET_COMET_CORE_MEMORY_ALLOCATOR_FRAME_ALLOCATOR_H_
