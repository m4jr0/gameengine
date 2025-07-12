// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "comet_pch.h"

#include "allocator.h"

namespace comet {
namespace memory {
void* Allocator::Allocate(usize size) { return AllocateAligned(size, 1); }
}  // namespace memory
}  // namespace comet
