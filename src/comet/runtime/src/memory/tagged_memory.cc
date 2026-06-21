// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_runtime_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/runtime/memory/tagged_memory.h"
////////////////////////////////////////////////////////////////////////////////

// External. ///////////////////////////////////////////////////////////////////
#include <immintrin.h>

#include <cstdint>
#include <cstdio>
#include <cstring>

#ifndef COMET_MSVC
#include <sys/sysinfo.h>
#include <unistd.h>
#endif  // !COMET_MSVC

#ifdef COMET_INVESTIGATE_MEMORY_CORRUPTION
#include <atomic>
#endif  // COMET_INVESTIGATE_MEMORY_CORRUPTION
////////////////////////////////////////////////////////////////////////////////

#include "comet/runtime/memory/allocation_tracking.h"
#include "comet/core/container/array.h"

#ifdef COMET_MSVC
#include "comet/core/windows.h"
#endif  // COMET_MSVC

#include "comet/core/string/c_string.h"
#include "comet/runtime/memory/memory_label.h"
#include "comet/core/processor/processor.h"

namespace comet {
namespace memory {
void* Allocate(usize size, MemoryTag tag) {
  return AllocateAligned(size, 1, tag);
}

void* AllocateAligned(usize size, Alignment align,
                      [[maybe_unused]] MemoryTag tag) {
  usize allocation_size{size + align};
  auto* ptr{new u8[allocation_size]};

  if (ptr == nullptr) {
    COMET_ASSERT(false, "memory_utils::AllocateAligned", "allocation failed",
                 "size", size, "alignment", align, "tag",
                 GetMemoryTagLabel(tag));
    throw std::bad_alloc();
  }

  COMET_REGISTER_PLATFORM_ALLOCATION(ptr, allocation_size, tag);
  COMET_POISON(ptr, allocation_size);
  return StoreShiftAndReturnAligned(ptr, size, allocation_size, align);
}

void Deallocate(void* ptr) {
  COMET_ASSERT(ptr != nullptr, "memory_utils::Deallocate", "pointer is null");
  auto* raw_ptr{ResolveNonAligned(ptr)};
  COMET_REGISTER_PLATFORM_DEALLOCATION(raw_ptr);
  delete[] static_cast<u8*>(raw_ptr);
}
}  // namespace memory
}  // namespace comet
