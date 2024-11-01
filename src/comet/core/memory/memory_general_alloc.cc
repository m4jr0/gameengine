// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "memory_general_alloc.h"

#include <new>

#include "comet/core/memory/allocation_tracking.h"

namespace comet {
namespace memory {
void* Allocate(usize size, MemoryTag tag) {
  return AllocateAligned(size, 1, tag);
}

void* AllocateAligned(usize size, Alignment align,
                      [[maybe_unused]] MemoryTag tag) {
  // TODO(m4jr0): Handle memory tag.
  usize allocation_size{size + align};
  auto* ptr{new u8[allocation_size]};

  if (ptr == nullptr) {
    COMET_ASSERT(false, "Unable to allocate with size ", size, ", align ",
                 align, " and tag ", GetMemoryTagLabel(tag), "!");
    throw std::bad_alloc();
  }

  COMET_REGISTER_PLATFORM_ALLOCATION(ptr, allocation_size, tag);
  COMET_POISON(ptr, allocation_size);
  return StoreShiftAndReturnAligned(ptr, size, allocation_size, align);
}

void Deallocate(void* ptr) {
  COMET_REGISTER_PLATFORM_DEALLOCATION(ptr);
  delete[] static_cast<u8*>(ResolveNonAligned(ptr));
}
}  // namespace memory
}  // namespace comet
