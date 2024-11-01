// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "aligned_allocator.h"

namespace comet {
namespace memory {
AlignedAllocator::~AlignedAllocator() {
  COMET_ASSERT(
      !is_initialized_,
      "Destructor called for aligned allocator, but it is still initialized!");
}

void AlignedAllocator::Initialize() {
  COMET_ASSERT(
      !is_initialized_,
      "Tried to initialize aligned allocator, but it is already done!");
  is_initialized_ = true;
}

void AlignedAllocator::Destroy() {
  COMET_ASSERT(
      is_initialized_,
      "Tried to shutdown aligned allocator, but it is not initialized!");
  is_initialized_ = false;
}

void* AlignedAllocator::Allocate(usize size) {
  return AllocateAligned(size, 1);
}

bool AlignedAllocator::IsInitialized() const noexcept {
  return is_initialized_;
}
}  // namespace memory
}  // namespace comet
