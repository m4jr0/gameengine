// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_MEMORY_ALLOCATOR_CONTAINER_ALLOCATOR_H_
#define COMET_COMET_CORE_MEMORY_ALLOCATOR_CONTAINER_ALLOCATOR_H_

#include "comet/core/essentials.h"
#include "comet/core/memory/memory.h"

namespace comet {
class ContainerAllocator {
 public:
  ContainerAllocator() = default;
  virtual ~ContainerAllocator() = default;

  virtual void* Allocate(usize size, memory::Alignment align = 1) = 0;
  virtual void Deallocate(void* ptr) = 0;
};
}  // namespace comet

#endif  // COMET_COMET_CORE_MEMORY_ALLOCATOR_CONTAINER_ALLOCATOR_H_
