// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_ALLOCATION_H_
#define COMET_COMET_CORE_ALLOCATION_H_

#include "comet/core/type/primitive.h"

namespace comet {
namespace internal {
struct MemoryUse {
  uindex total_allocated{0};
  uindex total_freed{0};
};
}  // namespace internal

uindex GetTotalAllocatedMemory();
uindex GetTotalFreedMemory();
uindex GetMemoryUse();
}  // namespace comet

#endif  // COMET_COMET_CORE_ALLOCATION_H_
