// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "stateful_allocator.h"
////////////////////////////////////////////////////////////////////////////////

namespace comet {
namespace memory {
StatefulAllocator::~StatefulAllocator() {
  COMET_ASSERT(!is_initialized_,
               "Destructor called for allocator, but it is still initialized!");
}

void StatefulAllocator::Initialize() {
  COMET_ASSERT(!is_initialized_,
               "Tried to initialize allocator, but it is already done!");
  is_initialized_ = true;
}

void StatefulAllocator::Destroy() {
  COMET_ASSERT(is_initialized_,
               "Tried to shutdown allocator, but it is not initialized!");
  is_initialized_ = false;
}

bool StatefulAllocator::IsInitialized() const noexcept {
  return is_initialized_;
}
}  // namespace memory
}  // namespace comet
