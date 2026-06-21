// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_CORE_MEMORY_ALLOCATOR_SYSTEM_ALLOCATOR_H_
#define COMET_CORE_MEMORY_ALLOCATOR_SYSTEM_ALLOCATOR_H_

#include <new>

#include "comet/core/essentials.h"
#include "comet/core/memory/allocator/allocator.h"
#include "comet/runtime/memory/tagged_memory.h"

namespace comet {
namespace memory {
class SystemAllocator final : public Allocator {
 public:
  void* AllocateAligned(usize size, Alignment align) override;
  void Deallocate(void* ptr) override;
};
}  // namespace memory
}  // namespace comet

#endif  // COMET_CORE_MEMORY_ALLOCATOR_SYSTEM_ALLOCATOR_H_