// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_CORE_MEMORY_ALLOCATOR_DEFAULT_ALLOCATOR_H_
#define COMET_CORE_MEMORY_ALLOCATOR_DEFAULT_ALLOCATOR_H_

#include "comet/core/essentials.h"
#include "comet/core/memory/allocator/allocator.h"

namespace comet {
namespace memory {
void AttachDefaultAllocator(Allocator* allocator);
void DetachDefaultAllocator();
Allocator& GetDefaultAllocator();
bool IsDefaultAllocatorAttached();
}  // namespace memory
}  // namespace comet

#endif  // COMET_CORE_MEMORY_ALLOCATOR_DEFAULT_ALLOCATOR_H_