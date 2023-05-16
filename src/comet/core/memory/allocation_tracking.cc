// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "allocation_tracking.h"

namespace comet {
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
void* operator new(std::size_t size) throw(std::bad_alloc) {
  comet::internal::memory_use.total_allocated += size;
  return std::malloc(size);
}

void operator delete(void* p, std::size_t size) throw() {
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
#endif  // COMET_DEBUG
