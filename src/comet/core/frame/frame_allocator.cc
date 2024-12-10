// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "frame_allocator.h"

#include "comet/core/concurrency/fiber/fiber_context.h"

namespace comet {
namespace frame {
thread_local memory::AllocatorHandle tls_frame_allocator_handle{nullptr};
thread_local memory::AllocatorHandle tls_double_frame_allocator_handle{nullptr};

memory::Allocator& GetFrameAllocator() {
  COMET_ASSERT(tls_frame_allocator_handle != nullptr,
               "Current frame allocator handle is null! No allocator handle "
               "has been attached for this thread!");
  COMET_ASSERT(tls_frame_allocator_handle->allocator != nullptr,
               "Current frame allocator is null for this thread!");
  return *tls_frame_allocator_handle->allocator;
}

memory::Allocator& GetDoubleFrameAllocator() {
  COMET_ASSERT(tls_double_frame_allocator_handle != nullptr,
               "Current double frame allocator handle is null! No allocator "
               "handle has been attached for this thread!");
  COMET_ASSERT(tls_double_frame_allocator_handle->allocator != nullptr,
               "Current frame allocator is null for this thread!");
  return *tls_double_frame_allocator_handle->allocator;
}

void AttachFrameAllocator(memory::AllocatorHandle handle) {
  COMET_ASSERT(handle != nullptr, "Frame allocator handle provided is null!");
  tls_frame_allocator_handle = handle;
}

void AttachDoubleFrameAllocator(memory::AllocatorHandle handle) {
  COMET_ASSERT(handle != nullptr,
               "Double frame allocator handle provided is null!");
  tls_double_frame_allocator_handle = handle;
}

void DetachFrameAllocator() {
  COMET_ASSERT(tls_frame_allocator_handle != nullptr,
               "No frame allocator handle has been provided!");
  tls_frame_allocator_handle = nullptr;
}

void DetachDoubleFrameAllocator() {
  COMET_ASSERT(tls_double_frame_allocator_handle != nullptr,
               "No double frame allocator handle has been provided!");
  tls_double_frame_allocator_handle = nullptr;
}
}  // namespace frame
}  // namespace comet
