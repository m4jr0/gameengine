// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_MEMORY_ALLOCATOR_STACK_ALLOCATOR_H_
#define COMET_COMET_CORE_MEMORY_ALLOCATOR_STACK_ALLOCATOR_H_

#include "comet/core/memory/memory.h"
#include "comet/core/type/primitive.h"

namespace comet {
using StackAllocatorMarker = u8*;

class StackAllocator {
 public:
  StackAllocator() = delete;
  StackAllocator(uindex capacity, MemoryTag memory_tag = MemoryTag::Untagged);
  StackAllocator(const StackAllocator&) = delete;
  StackAllocator(StackAllocator&&) = delete;
  StackAllocator& operator=(const StackAllocator&) = delete;
  StackAllocator& operator=(StackAllocator&&) = delete;
  ~StackAllocator();

  void Initialize();
  void Destroy();
  void* Allocate(uindex size, u8 align = 0);
  void Clear();
  bool IsInitialized() const noexcept;
  uindex GetSize() const noexcept;

 private:
  bool is_initialized_{false};
  MemoryTag memory_tag_{MemoryTag::Untagged};
  u8* root_{nullptr};
  uindex size_{0};
  uindex capacity_{0};
  StackAllocatorMarker marker_{0};
};
}  // namespace comet

#endif  // COMET_COMET_CORE_MEMORY_ALLOCATOR_STACK_ALLOCATOR_H_
