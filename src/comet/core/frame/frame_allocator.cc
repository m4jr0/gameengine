// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "frame_allocator.h"
////////////////////////////////////////////////////////////////////////////////

namespace comet {
namespace frame {
static thread_local memory::Allocator* tls_frame_allocator{nullptr};
static thread_local memory::Allocator* tls_double_frame_allocator{nullptr};

memory::Allocator& GetFrameAllocator() {
  COMET_ASSERT(tls_frame_allocator != nullptr,
               "Current frame allocator is null for this thread!");
  return *tls_frame_allocator;
}

memory::Allocator& GetDoubleFrameAllocator() {
  COMET_ASSERT(tls_double_frame_allocator != nullptr,
               "Current frame allocator is null for this thread!");
  return *tls_double_frame_allocator;
}

void AttachFrameAllocator(memory::Allocator* allocator) {
  COMET_ASSERT(allocator != nullptr, "Frame allocator provided is null!");
  tls_frame_allocator = allocator;
}

void AttachDoubleFrameAllocator(memory::Allocator* allocator) {
  COMET_ASSERT(allocator != nullptr,
               "Double frame allocator provided is null!");
  tls_double_frame_allocator = allocator;
}

void DetachFrameAllocator() {
  COMET_ASSERT(tls_frame_allocator != nullptr,
               "No frame allocator has been provided!");
  tls_frame_allocator = nullptr;
}

void DetachDoubleFrameAllocator() {
  COMET_ASSERT(tls_double_frame_allocator != nullptr,
               "No double frame allocator has been provided!");
  tls_double_frame_allocator = nullptr;
}
}  // namespace frame
}  // namespace comet
