// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_core_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/core/memory/allocator/system_allocator.h"
////////////////////////////////////////////////////////////////////////////////

// External. ///////////////////////////////////////////////////////////////////
#include <limits>
#include <new>
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/memory/memory_utils.h"

namespace comet {
namespace memory {
void* SystemAllocator::AllocateAligned(usize size, Alignment align) {
  if (align == 0 || align > kMaxAlignment ||
      (align & (align - 1)) != 0) {
    throw std::bad_alloc{};
  }

  if (size > std::numeric_limits<usize>::max() - align) {
    throw std::bad_array_new_length{};
  }

  const auto allocation_size{size + align};
  auto* raw_ptr{::operator new(allocation_size)};
  return StoreShiftAndReturnAligned(raw_ptr, size, allocation_size, align);
}

void SystemAllocator::Deallocate(void* ptr) {
  if (ptr == nullptr) {
    return;
  }

  ::operator delete(ResolveNonAligned(ptr));
}
}  // namespace memory
}  // namespace comet
