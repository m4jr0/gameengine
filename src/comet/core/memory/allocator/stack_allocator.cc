// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "comet_pch.h"

#include "stack_allocator.h"

#include <utility>

#include "comet/core/memory/memory_utils.h"
#include "comet/core/memory/tagged_heap.h"

namespace comet {
namespace memory {
StackAllocator::StackAllocator(usize capacity, MemoryTag memory_tag)
    : memory_tag_{memory_tag},
      capacity_{capacity},
      root_{nullptr},
      marker_{root_} {}

StackAllocator::StackAllocator(StackAllocator&& other) noexcept
    : StatefulAllocator{std::move(other)},
      memory_tag_{other.memory_tag_},
      capacity_{other.capacity_},
      root_{other.root_},
      marker_{other.marker_} {
  other.memory_tag_ = kEngineMemoryTagUntagged;
  other.capacity_ = 0;
  other.root_ = nullptr;
  other.marker_ = nullptr;
}

StackAllocator& StackAllocator::operator=(StackAllocator&& other) noexcept {
  if (this == &other) {
    return *this;
  }

  if (root_ != nullptr) {
    TaggedHeap::Get().DeallocateAll(memory_tag_);
  }

  StatefulAllocator::operator=(other);
  memory_tag_ = other.memory_tag_;
  capacity_ = other.capacity_;
  root_ = other.root_;
  marker_ = other.marker_;

  other.memory_tag_ = kEngineMemoryTagUntagged;
  other.capacity_ = 0;
  other.root_ = nullptr;
  other.marker_ = nullptr;
  return *this;
}

void StackAllocator::Initialize() {
  StatefulAllocator::Initialize();
  COMET_ASSERT(capacity_ > 0, "Capacity is ", capacity_, "!");
  root_ = static_cast<u8*>(TaggedHeap::Get().AllocateAligned(
      capacity_, alignof(u8), memory_tag_, &capacity_));
  marker_ = root_;
}

void StackAllocator::Destroy() {
  StatefulAllocator::Destroy();
  TaggedHeap::Get().DeallocateAll(memory_tag_);
  root_ = nullptr;
  capacity_ = 0;
  marker_ = nullptr;
}

void* StackAllocator::AllocateAligned(usize size, Alignment align) {
  COMET_ASSERT(size > 0, "Allocation size provided is 0!");
  auto* p{AlignPointer(marker_, align)};
  COMET_ASSERT(p + size <= root_ + capacity_,
               "Could not allocate enough memory (", size, ")!");
  marker_ = p + size;
  return p;
}

void StackAllocator::Deallocate(void*) {
  // A stack allocator does not support individual deallocations, as it is
  // intended for temporary data only. Memory is only released when Clear() is
  // called, which resets the entire stack.
}

void StackAllocator::Clear() { marker_ = root_; }

FiberStackAllocator::FiberStackAllocator(usize base_capacity,
                                         MemoryTag memory_tag,
                                         MemoryTag extended_memory_tag)
    : memory_tag_{memory_tag},
      extended_memory_tag_{extended_memory_tag},
      base_capacity_{base_capacity},
      extended_capacity_{0},
      root_{nullptr},
      marker_{root_} {}

void FiberStackAllocator::Initialize() {
  StatefulAllocator::Initialize();
  thread_capacity_ = TaggedHeap::Get().GetBlockSize() - 1;
  thread_contexts_.Initialize();
  COMET_ASSERT(base_capacity_ > 0, "Capacity is ", base_capacity_, "!");

  auto size{thread_contexts_.GetSize()};
  auto& tagged_heap{TaggedHeap::Get()};

  for (usize i{0}; i < size; ++i) {
    auto& context{thread_contexts_.GetFromIndex(i)};
    context.root = context.marker =
        static_cast<u8*>(tagged_heap.AllocateAligned(thread_capacity_,
                                                     alignof(u8), memory_tag_));
  }
}

void FiberStackAllocator::Destroy() {
  StatefulAllocator::Destroy();
  thread_contexts_.Destroy();
  auto& tagged_heap{TaggedHeap::Get()};
  tagged_heap.DeallocateAll(memory_tag_);
  tagged_heap.DeallocateAll(extended_memory_tag_);
  root_ = nullptr;
  base_capacity_ = 0;
  marker_ = nullptr;
}

void* FiberStackAllocator::AllocateAligned(usize size, Alignment align) {
  COMET_ASSERT(size > 0, "Allocation size provided is 0!");
  auto& context{thread_contexts_.Get()};
  auto* p{AlignPointer(context.marker, align)};

  if (p + size <= context.root + thread_capacity_) {
    context.marker = p + size;
    return p;
  }

  fiber::FiberLockGuard lock{mutex_};

  if (root_ == nullptr) {
    AllocateCommonMemory();
  }

  if (extended_marker_ == nullptr) {
    p = AlignPointer(marker_, align);

    if (p + size <= root_ + base_capacity_) {
      marker_ = p + size;
      return p;
    }

    COMET_ASSERT(extended_memory_tag_ != kEngineMemoryTagInvalid,
                 "Could not allocate enough memory (", size, ")!");
    ExtendCommonMemory(size * 2);
  }

  p = AlignPointer(extended_marker_, align);

  if (p + size > extended_root_ + extended_capacity_) {
    ExtendCommonMemory(size * 2);
    p = AlignPointer(extended_marker_, align);
  }

  extended_marker_ = p + size;
  return p;
}

void FiberStackAllocator::Deallocate(void*) {
  // A stack allocator does not support individual deallocations, as it is
  // intended for temporary data only. Memory is only released when Clear() is
  // called, which resets the entire stack.
}

void FiberStackAllocator::Clear() {
  for (auto& context : thread_contexts_) {
    context.marker = context.root;
  }

  if (extended_root_ != nullptr) {
    TaggedHeap::Get().DeallocateAll(extended_memory_tag_);
    extended_root_ = extended_marker_ = nullptr;
  }

  marker_ = root_;
}

void FiberStackAllocator::AllocateCommonMemory() {
  root_ = static_cast<u8*>(
      TaggedHeap::Get().Allocate(base_capacity_, memory_tag_, &base_capacity_));
  marker_ = root_;
}

void FiberStackAllocator::ExtendCommonMemory(usize capacity) {
  COMET_ASSERT(extended_memory_tag_ != kEngineMemoryTagInvalid,
               "Extended memory is not enabled on this stack allocator!");
  extended_root_ = static_cast<u8*>(TaggedHeap::Get().Allocate(
      capacity, extended_memory_tag_, &extended_capacity_));
  extended_marker_ = extended_root_;
}

IOStackAllocator::IOStackAllocator(usize thread_capacity, MemoryTag memory_tag)
    : memory_tag_{memory_tag}, thread_capacity_{thread_capacity} {}

void IOStackAllocator::Initialize() {
  StatefulAllocator::Initialize();
  thread_contexts_.Initialize();
  auto size{thread_contexts_.GetSize()};
  auto& tagged_heap{TaggedHeap::Get()};

  for (usize i{0}; i < size; ++i) {
    auto& context{thread_contexts_.GetFromIndex(i)};
    context.root = context.marker = static_cast<u8*>(
        tagged_heap.AllocateBlockAligned(alignof(u8), memory_tag_));
  }
}

void IOStackAllocator::Destroy() {
  StatefulAllocator::Destroy();
  thread_contexts_.Destroy();
  TaggedHeap::Get().DeallocateAll(memory_tag_);
}

void* IOStackAllocator::AllocateAligned(usize size, Alignment align) {
  COMET_ASSERT(size > 0, "Allocation size provided is 0!");
  auto& context{thread_contexts_.Get()};
  auto* p{AlignPointer(context.marker, align)};
  COMET_ASSERT(p + size <= context.root + thread_capacity_,
               "Could not allocate enough memory (", size, ")!");
  context.marker = p + size;
  return p;
}

void IOStackAllocator::Deallocate(void*) {
  // A stack allocator does not support individual deallocations, as it is
  // intended for temporary data only. Memory is only released when Clear() is
  // called, which resets the entire stack.
}

void IOStackAllocator::Clear() {
  for (auto& context : thread_contexts_) {
    context.marker = context.root;
  }
}

LockFreeStackAllocator::LockFreeStackAllocator(usize capacity,
                                               MemoryTag memory_tag)
    : memory_tag_{memory_tag},
      capacity_{capacity},
      offset_{kInvalidOffset_},
      root_{nullptr} {}

void LockFreeStackAllocator::Initialize() {
  StatefulAllocator::Initialize();
  offset_ = 0;
  root_ = static_cast<u8*>(Allocate(capacity_));
}

void LockFreeStackAllocator::Destroy() {
  StatefulAllocator::Destroy();
  Deallocate(root_);
  offset_ = kInvalidOffset_;
  root_ = nullptr;
}

void* LockFreeStackAllocator::AllocateAligned(usize size, Alignment align) {
  COMET_ASSERT(size > 0, "Allocation size provided is 0!");
  auto current_offset{offset_.load(std::memory_order_relaxed)};
  COMET_ASSERT(current_offset >= 0, "Invalid offset: ", current_offset, "!");
  auto aligned_offset{
      memory::AlignAddress(reinterpret_cast<uptr>(root_ + current_offset),
                           align) -
      reinterpret_cast<uptr>(root_)};
  auto new_offset{aligned_offset + size};

  COMET_ASSERT(new_offset < capacity_, "Could not allocate enough memory (",
               size, ")!");

  while (!offset_.compare_exchange_weak(current_offset, new_offset,
                                        std::memory_order_acquire)) {
    COMET_ASSERT(current_offset >= 0, "Invalid offset: ", current_offset, "!");
    aligned_offset = memory::AlignAddress(
        reinterpret_cast<uptr>(root_ + current_offset), align);
    new_offset = aligned_offset + size;
    COMET_ASSERT(new_offset > capacity_, "Could not allocate enough memory (",
                 size, ")!");
  }

  return root_ + aligned_offset;
}

void LockFreeStackAllocator::Deallocate(void*) {
  // A stack allocator does not support individual deallocations, as it is
  // intended for temporary data only. Memory is only released when Clear() is
  // called, which resets the entire stack.
}

void LockFreeStackAllocator::Clear() {
  offset_.store(0, std::memory_order_release);
}
}  // namespace memory
}  // namespace comet
