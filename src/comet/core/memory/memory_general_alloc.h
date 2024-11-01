// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_MEMORY_MEMORY_GENERAL_ALLOC_H_
#define COMET_COMET_CORE_MEMORY_MEMORY_GENERAL_ALLOC_H_

#include "comet/core/essentials.h"
#include "comet/core/memory/memory.h"

namespace comet {
namespace memory {
void* Allocate(usize size, MemoryTag tag = kEngineMemoryTagUntagged);
void* AllocateAligned(usize size, Alignment align, MemoryTag tag);

template <typename T>
T* AllocateMany(usize count, MemoryTag tag) {
  return static_cast<T*>(AllocateAligned(sizeof(T) * count, alignof(T), tag));
}

template <typename T>
T* AllocateOne(MemoryTag tag) {
  return AllocateMany<T>(1, tag);
}

template <typename T, typename... Targs>
T* AllocateOneAndPopulate(MemoryTag tag, Targs... args) {
  auto* ptr{AllocateOne<T>(tag)};
  return Populate<T>(ptr, std::forward<Targs>(args)...);
}

void Deallocate(void* ptr);
}  // namespace memory
}  // namespace comet

#endif  // COMET_COMET_CORE_MEMORY_MEMORY_GENERAL_ALLOC_H_
