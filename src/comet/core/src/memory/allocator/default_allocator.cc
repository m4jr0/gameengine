// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/core/memory/allocator/default_allocator.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/memory/allocator/system_allocator.h"

namespace comet {
namespace memory {
namespace {
SystemAllocator fallback_allocator{};
Allocator* default_allocator{
    &fallback_allocator};  // >:3 Don't forget setting the default allocator!
bool is_custom_allocator_attached{false};
}  // namespace

void AttachDefaultAllocator(Allocator* allocator) {
  COMET_ASSERT(allocator != nullptr, "memory::AttachDefaultAllocator",
               "allocator is null");
  COMET_ASSERT(!is_custom_allocator_attached, "memory::AttachDefaultAllocator",
               "default allocator is already attached");

  default_allocator = allocator;
  is_custom_allocator_attached = true;
}

void DetachDefaultAllocator() {
  COMET_ASSERT(is_custom_allocator_attached, "memory::DetachDefaultAllocator",
               "no custom allocator is attached");

  default_allocator = &fallback_allocator;
  is_custom_allocator_attached = false;
}

Allocator& GetDefaultAllocator() { return *default_allocator; }

bool IsDefaultAllocatorAttached() { return is_custom_allocator_attached; }
}  // namespace memory
}  // namespace comet