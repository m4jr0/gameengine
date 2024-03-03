// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifdef COMET_DEBUG
#ifndef COMET_COMET_CORE_MEMORY_STRING_ID_ALLOCATOR_H_
#define COMET_COMET_CORE_MEMORY_STRING_ID_ALLOCATOR_H_

#include "comet_precompile.h"

#include "comet/core/memory/allocator/stack_allocator.h"

namespace comet {
namespace memory {
using StringIdAllocator = StackAllocator;
}  // namespace memory
}  // namespace comet

#endif  // COMET_COMET_CORE_MEMORY_STRING_ID_ALLOCATOR_H_
#endif  // COMET_DEBUG
