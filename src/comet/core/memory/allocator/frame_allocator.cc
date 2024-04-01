// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "frame_allocator.h"

namespace comet {
namespace memory {
TwoFrameAllocator::TwoFrameAllocator(usize total_capacity)
    : stacks_{StackAllocator{total_capacity / 2},
              StackAllocator{total_capacity / 2}} {}

void TwoFrameAllocator::Initialize() {
  stacks_[0].Initialize();
  stacks_[1].Initialize();
}

void TwoFrameAllocator::Destroy() {
  stacks_[0].Destroy();
  stacks_[1].Destroy();
}

void* TwoFrameAllocator::Allocate(usize size, u8 align) {
  return stacks_[current_stack_].Allocate(size, align);
}

void TwoFrameAllocator::SwapFrames() {
  current_stack_ = static_cast<u8>(!current_stack_);
}

void TwoFrameAllocator::ClearCurrent() { stacks_[current_stack_].Clear(); }

usize TwoFrameAllocator::GetCurrentSize() const noexcept {
  return stacks_[current_stack_].GetSize();
}

usize TwoFrameAllocator::GetTotalSize() const noexcept {
  return stacks_[0].GetSize() + stacks_[1].GetSize();
}
}  // namespace memory
}  // namespace comet
