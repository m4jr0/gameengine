// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "allocation_tracking.h"

#include "comet/core/compiler.h"
#include "comet/core/debug.h"
#include "comet/core/logger.h"

namespace comet {
namespace internal {
static MemoryUse memory_use{};
}  // namespace internal

uindex GetTotalAllocatedMemory() {
  return internal::memory_use.total_allocated;
}

uindex GetTotalFreedMemory() { return internal::memory_use.total_freed; }

uindex GetMemoryUse() {
  COMET_ASSERT(
      internal::memory_use.total_allocated >= internal::memory_use.total_freed,
      "Something wrong happened while allocating memory!");
  return internal::memory_use.total_allocated -
         internal::memory_use.total_freed;
}
}  // namespace comet

#ifdef COMET_DEBUG
void* operator new(std::size_t size) {
  comet::internal::memory_use.total_allocated += size;
  return std::malloc(size);
}

void operator delete(void* p, std::size_t size) {
  comet::internal::memory_use.total_freed += size;
  std::free(p);
}

void* operator new[](std::size_t size) {
  comet::internal::memory_use.total_allocated += size;
  return std::malloc(size);
}

void operator delete[](void* p, std::size_t size) {
  comet::internal::memory_use.total_freed += size;
  std::free(p);
}

#ifdef COMET_GCC
void operator delete(void* p) { std::free(p); }

void operator delete[](void* p) { std::free(p); }
#endif  // COMET_GCC
#endif  // COMET_DEBUG
