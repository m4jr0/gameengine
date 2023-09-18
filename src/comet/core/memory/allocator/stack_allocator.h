// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_MEMORY_ALLOCATOR_STACK_ALLOCATOR_H_
#define COMET_COMET_CORE_MEMORY_ALLOCATOR_STACK_ALLOCATOR_H_

#include "comet_precompile.h"

namespace comet {
using StackAllocatorMarker = u8*;

class StackAllocator {
 public:
  StackAllocator() = delete;
  explicit StackAllocator(uindex capacity);
  StackAllocator(const StackAllocator&) = delete;
  StackAllocator(StackAllocator&&) = delete;
  StackAllocator& operator=(const StackAllocator&) = delete;
  StackAllocator& operator=(StackAllocator&&) = delete;
  ~StackAllocator() = default;

  void Initialize();
  void Destroy();
  void* Allocate(uindex size, u8 align = 0);
  void Clear();
  uindex GetSize() const noexcept;

 private:
  u8* root_{nullptr};
  uindex size_{0};
  uindex capacity_{0};
  StackAllocatorMarker marker_{0};
};
}  // namespace comet

#endif  // COMET_COMET_CORE_MEMORY_ALLOCATOR_STACK_ALLOCATOR_H_
