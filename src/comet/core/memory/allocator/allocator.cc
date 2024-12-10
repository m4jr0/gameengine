// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "allocator.h"

namespace comet {
namespace memory {
Allocator::~Allocator() {
  COMET_ASSERT(!is_initialized_,
               "Destructor called for allocator, but it is still initialized!");
}

void Allocator::Initialize() {
  COMET_ASSERT(!is_initialized_,
               "Tried to initialize allocator, but it is already done!");
  is_initialized_ = true;
}

void Allocator::Destroy() {
  COMET_ASSERT(is_initialized_,
               "Tried to shutdown allocator, but it is not initialized!");
  is_initialized_ = false;
}

void* Allocator::Allocate(usize size) { return AllocateAligned(size, 1); }

bool Allocator::IsInitialized() const noexcept { return is_initialized_; }
}  // namespace memory
}  // namespace comet
