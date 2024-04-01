// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_ALLOCATION_H_
#define COMET_COMET_CORE_ALLOCATION_H_

#include <atomic>

#include "comet/core/essentials.h"

namespace comet {
namespace memory {
namespace internal {
struct MemoryUse {
  static_assert(std::atomic<usize>::is_always_lock_free,
                "std::atomic<usize> needs to be always lock-free. Unsupported "
                "architecture");

  std::atomic<usize> total_allocated{0};
  std::atomic<usize> total_freed{0};
};
}  // namespace internal

usize GetTotalAllocatedMemory();
usize GetTotalFreedMemory();
usize GetMemoryUse();
}  // namespace memory
}  // namespace comet

#endif  // COMET_COMET_CORE_ALLOCATION_H_
