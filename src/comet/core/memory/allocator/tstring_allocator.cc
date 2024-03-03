// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "tstring_allocator.h"

#include "comet/core/memory/memory.h"

namespace comet {
TStringAllocator::TStringAllocator(uindex capacity) {
  // TODO(m4jr0): Implement specific TString allocator.
}

TStringAllocator::~TStringAllocator() {
  COMET_ASSERT(
      !is_initialized_,
      "Destructor called for TString allocator, but it is still initialized!");
}

void TStringAllocator::Initialize() {
  COMET_ASSERT(
      !is_initialized_,
      "Tried to initialize TString allocator, but it is already done!");
  // TODO(m4jr0): Implement specific TString allocator.
  is_initialized_ = true;
}

void TStringAllocator::Destroy() {
  COMET_ASSERT(
      is_initialized_,
      "Tried to destroy TString allocator, but it is not initialized!");
  // TODO(m4jr0): Implement specific TString allocator.
  is_initialized_ = false;
}

void* TStringAllocator::Allocate(uindex size) {
  // TODO(m4jr0): Implement specific TString allocator.
  return AllocateAligned<tchar>(size, MemoryTag::TString);
}

void TStringAllocator::Deallocate(void* p) {
  // TODO(m4jr0): Implement specific TString allocator.
  comet::Deallocate(p);
}

void TStringAllocator::Clear() {
  // TODO(m4jr0): Implement specific TString allocator.
}

uindex TStringAllocator::GetSize() const noexcept {
  // TODO(m4jr0): Implement specific TString allocator.
  return 0;
}
}  // namespace comet
