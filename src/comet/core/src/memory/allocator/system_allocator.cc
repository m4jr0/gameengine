// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_core_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/core/memory/allocator/system_allocator.h"
////////////////////////////////////////////////////////////////////////////////

namespace comet {
namespace memory {
void* SystemAllocator::AllocateAligned(usize size, Alignment align) {
  return ::operator new(size, std::align_val_t{align});
}

void SystemAllocator::Deallocate(void* ptr) { ::operator delete(ptr); }
}  // namespace memory
}  // namespace comet
