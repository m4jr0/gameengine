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
  COMET_ASSERT(!is_initialized_, "StatefulAllocator::~StatefulAllocator",
               "allocator is still initialized", "is_initialized",
               is_initialized_);
}

void StatefulAllocator::Initialize() {
  COMET_ASSERT(!is_initialized_, "StatefulAllocator::Initialize",
               "allocator is already initialized", "is_initialized",
               is_initialized_);

  OnInitialize();
  is_initialized_ = true;
}

void StatefulAllocator::Destroy() {
  COMET_ASSERT(is_initialized_, "StatefulAllocator::Destroy",
               "allocator is not initialized", "is_initialized",
               is_initialized_);

  OnDestroy();
  is_initialized_ = false;
}

bool StatefulAllocator::IsInitialized() const noexcept {
  return is_initialized_;
}

void StatefulAllocator::OnInitialize() {}

void StatefulAllocator::OnDestroy() {}
}  // namespace memory
}  // namespace comet
