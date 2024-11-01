// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_FRAME_FRAME_ALLOCATOR_H_
#define COMET_COMET_CORE_FRAME_FRAME_ALLOCATOR_H_

#include "comet/core/essentials.h"
#include "comet/core/memory/allocator/aligned_allocator.h"
#include "comet/core/memory/allocator/stack_allocator.h"
#include "comet/core/memory/memory.h"

namespace comet {
namespace frame {
using FiberFrameAllocator = memory::FiberStackAllocator;
using FiberDoubleFrameAllocator =
    memory::DoubleStackAllocator<FiberFrameAllocator>;

using IOFrameAllocator = memory::IOStackAllocator;
using IODoubleFrameAllocator = memory::DoubleStackAllocator<IOFrameAllocator>;

memory::AlignedAllocator& GetFrameAllocator();
memory::AlignedAllocator& GetDoubleFrameAllocator();

void AttachFrameAllocator(memory::AlignedAllocatorHandle handle);
void AttachDoubleFrameAllocator(memory::AlignedAllocatorHandle handle);
void DetachFrameAllocator();
void DetachDoubleFrameAllocator();
}  // namespace frame
}  // namespace comet

#define COMET_FRAME_ALLOC(size) comet::frame::GetFrameAllocator().Allocate(size)
#define COMET_FRAME_ALLOC_ALIGNED(size, align) \
  comet::frame::GetFrameAllocator().AllocateAligned(size, align)
#define COMET_FRAME_ALLOC_MANY(T, count) \
  comet::frame::GetFrameAllocator().AllocateMany<T>(count)
#define COMET_FRAME_ALLOC_ONE(T) \
  comet::frame::GetFrameAllocator().AllocateOne<T>()
#define COMET_FRAME_ALLOC_ONE_AND_POPULATE(T, ...) \
  comet::frame::GetFrameAllocator().AllocateOneAndPopulate<T>(__VA_ARGS__)

#define COMET_DOUBLE_FRAME_ALLOC(size) \
  comet::frame::GetDoubleFrameAllocator().Allocate(size)
#define COMET_DOUBLE_FRAME_ALLOC_ALIGNED(size, align) \
  comet::frame::GetDoubleFrameAllocator().AllocateAligned(size, align)
#define COMET_DOUBLE_FRAME_ALLOC_MANY(T, count) \
  comet::frame::GetDoubleFrameAllocator().AllocateMany<T>(count)
#define COMET_DOUBLE_FRAME_ALLOC_ONE(T) \
  comet::frame::GetDoubleFrameAllocator().AllocateOne<T>()
#define COMET_DOUBLE_FRAME_ALLOC_ONE_AND_POPULATE(T, ...) \
  comet::frame::GetDoubleFrameAllocator().AllocateOneAndPopulate<T>(__VA_ARGS__)

#endif  // COMET_COMET_CORE_FRAME_FRAME_ALLOCATOR_H_
