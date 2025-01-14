// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_MEMORY_VIRTUAL_MEMORY_H_
#define COMET_COMET_CORE_MEMORY_VIRTUAL_MEMORY_H_

#include "comet/core/essentials.h"

namespace comet {
namespace memory {
void* ReserveVirtualMemory(usize size);
void* CommitVirtualMemory(void* memory, usize size);
void FreeVirtualMemory(void* memory, usize size);
}  // namespace memory
}  // namespace comet

#endif  // COMET_COMET_CORE_MEMORY_VIRTUAL_MEMORY_H_
