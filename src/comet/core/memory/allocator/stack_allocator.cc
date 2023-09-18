// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "stack_allocator.h"

#include "comet/core/memory/memory.h"

namespace comet {
StackAllocator::StackAllocator(uindex capacity)
    : size_{0}, capacity_{capacity}, root_{nullptr}, marker_{root_} {}

void StackAllocator::Initialize() {
  COMET_ASSERT(capacity_ > 0, "Capacity is ", capacity_, "!");
  root_ = reinterpret_cast<u8*>(
      AllocateAligned(capacity_, alignof(u8), MemoryTag::Untagged));
  marker_ = root_;
}

void StackAllocator::Destroy() {
  Deallocate(root_);
  root_ = nullptr;
  size_ = 0;
  capacity_ = 0;
  marker_ = nullptr;
}

void* StackAllocator::Allocate(uindex size, u8 alignment) {
  auto* p{AlignPointer(marker_, alignment)};
  COMET_ASSERT(p + size <= root_ + capacity_, "Max capacity reached!");
  marker_ = p + size;
  return p;
}

void StackAllocator::Clear() { marker_ = root_; }

uindex StackAllocator::GetSize() const noexcept {
  COMET_ASSERT(marker_ > root_, "Stack allocator is malformed!");
  return marker_ - root_;
}
}  // namespace comet
