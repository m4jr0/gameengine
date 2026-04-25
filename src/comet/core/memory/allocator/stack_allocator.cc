// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "stack_allocator.h"
////////////////////////////////////////////////////////////////////////////////

// External. ///////////////////////////////////////////////////////////////////
#include <utility>
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/memory/memory_utils.h"
#include "comet/core/memory/tagged_heap.h"

#if defined(COMET_DEBUG_STACK_ALLOCATOR) && \
    defined(COMET_VERBOSE_ALLOCATOR_LOGS)
#include "comet/core/logger/logging.h"
#include "comet/core/memory/memory_label.h"
#endif  // defined(COMET_DEBUG_STACK_ALLOCATOR) &&
        //  defined(COMET_VERBOSE_ALLOCATOR_LOGS)

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

  StatefulAllocator::operator=(std::move(other));
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

void* StackAllocator::AllocateAligned(usize size, Alignment align) {
  COMET_ASSERT(IsInitialized(), "StackAllocator::AllocateAligned",
               "allocator is not initialized");
  COMET_ASSERT(root_ != nullptr, "StackAllocator::AllocateAligned",
               "allocator is not initialized");
  COMET_ASSERT(capacity_ > 0, "StackAllocator::AllocateAligned",
               "capacity is invalid", "capacity", capacity_);
  COMET_ASSERT(align > 0, "StackAllocator::AllocateAligned",
               "alignment is invalid", "align", align);
  COMET_ASSERT(size > 0, "StackAllocator::AllocateAligned",
               "allocation size is zero");

  auto* p{AlignPointer(marker_, align)};

  COMET_ASSERT(p + size <= root_ + capacity_, "StackAllocator::AllocateAligned",
               "allocation exceeds capacity", "size", size, "capacity",
               capacity_);

  marker_ = p + size;

#ifdef COMET_DEBUG_STACK_ALLOCATOR
  ++allocation_count_;
  const auto used{static_cast<usize>(marker_ - root_)};
  peak_used_size_ = used > peak_used_size_ ? used : peak_used_size_;
#endif  // COMET_DEBUG_STACK_ALLOCATOR

  COMET_ASSERT(marker_ >= root_ && marker_ <= root_ + capacity_,
               "StackAllocator::AllocateAligned", "marker out of bounds",
               "marker", marker_, "root", root_, "capacity", capacity_);

  return p;
}

void StackAllocator::Deallocate(void*) {
  COMET_ASSERT(IsInitialized(), "StackAllocator::Deallocate",
               "allocator is not initialized");
  // A stack allocator does not support individual deallocations, as it is
  // intended for temporary data only. Memory is only released when Clear() is
  // called, which resets the entire stack.
}

void StackAllocator::Clear() {
  COMET_ASSERT(IsInitialized(), "StackAllocator::Clear",
               "allocator is not initialized");
  marker_ = root_;

#ifdef COMET_DEBUG_STACK_ALLOCATOR
  ++clear_count_;
#endif  // COMET_DEBUG_STACK_ALLOCATOR
}

void StackAllocator::OnInitialize() {
  COMET_ASSERT(capacity_ > 0, "StackAllocator::OnInitialize",
               "capacity is invalid", "capacity", capacity_);

  root_ = static_cast<u8*>(TaggedHeap::Get().AllocateAligned(
      capacity_, alignof(u8), memory_tag_, &capacity_));
  marker_ = root_;

#ifdef COMET_DEBUG_STACK_ALLOCATOR
  allocation_count_ = 0;
  clear_count_ = 0;
  peak_used_size_ = 0;
#endif  // COMET_DEBUG_STACK_ALLOCATOR
}

void StackAllocator::OnDestroy() {
#if defined(COMET_DEBUG_STACK_ALLOCATOR) && \
    defined(COMET_VERBOSE_ALLOCATOR_LOGS)
  COMET_LOG_DEBUG(LoggerType::Core, "StackAllocator::OnDestroy",
                  "destroy stack allocator", "tag",
                  GetMemoryTagLabel(memory_tag_), "capacity", capacity_,
                  "allocation_count", allocation_count_, "clear_count",
                  clear_count_, "peak_used_size", peak_used_size_);
#endif  // defined(COMET_DEBUG_STACK_ALLOCATOR) &&
        //  defined(COMET_VERBOSE_ALLOCATOR_LOGS)

  TaggedHeap::Get().DeallocateAll(memory_tag_);
  root_ = nullptr;
  capacity_ = 0;
  marker_ = nullptr;

#ifdef COMET_DEBUG_STACK_ALLOCATOR
  allocation_count_ = 0;
  clear_count_ = 0;
  peak_used_size_ = 0;
#endif  // COMET_DEBUG_STACK_ALLOCATOR
}

FiberStackAllocator::FiberStackAllocator(usize base_capacity,
                                         MemoryTag memory_tag,
                                         MemoryTag extended_memory_tag)
    : memory_tag_{memory_tag},
      extended_memory_tag_{extended_memory_tag},
      base_capacity_{base_capacity},
      extended_capacity_{0},
      root_{nullptr},
      marker_{root_} {}

void* FiberStackAllocator::AllocateAligned(usize size, Alignment align) {
  COMET_ASSERT(IsInitialized(), "FiberStackAllocator::AllocateAligned",
               "allocator is not initialized");
  COMET_ASSERT(align > 0, "FiberStackAllocator::AllocateAligned",
               "alignment is invalid", "align", align);
  COMET_ASSERT(size > 0, "FiberStackAllocator::AllocateAligned",
               "allocation size is zero");

  auto& context{thread_contexts_.Get()};
  COMET_ASSERT(context.root != nullptr, "FiberStackAllocator::AllocateAligned",
               "thread context root is null");
  COMET_ASSERT(context.marker != nullptr,
               "FiberStackAllocator::AllocateAligned",
               "thread context marker is null");

  auto* p{AlignPointer(context.marker, align)};

  if (p + size <= context.root + thread_capacity_) {
    context.marker = p + size;

#ifdef COMET_DEBUG_STACK_ALLOCATOR
    ++thread_allocation_count_;
#endif  // COMET_DEBUG_STACK_ALLOCATOR

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

#ifdef COMET_DEBUG_STACK_ALLOCATOR
      ++common_allocation_count_;
      const auto used{static_cast<usize>(marker_ - root_)};
      peak_common_used_size_ =
          used > peak_common_used_size_ ? used : peak_common_used_size_;
#endif  // COMET_DEBUG_STACK_ALLOCATOR

      return p;
    }

    COMET_ASSERT(extended_memory_tag_ != kEngineMemoryTagInvalid,
                 "FiberStackAllocator::AllocateAligned",
                 "extended memory is disabled", "size", size);

    ExtendCommonMemory(size * 2);
  }

  p = AlignPointer(extended_marker_, align);

  if (p + size > extended_root_ + extended_capacity_) {
    ExtendCommonMemory(size * 2);
    p = AlignPointer(extended_marker_, align);
  }

  extended_marker_ = p + size;

#ifdef COMET_DEBUG_STACK_ALLOCATOR
  ++extended_allocation_count_;
  const auto used{static_cast<usize>(extended_marker_ - extended_root_)};
  peak_extended_used_size_ =
      used > peak_extended_used_size_ ? used : peak_extended_used_size_;
#endif  // COMET_DEBUG_STACK_ALLOCATOR
  return p;
}

void FiberStackAllocator::Deallocate(void*) {
  COMET_ASSERT(IsInitialized(), "FiberStackAllocator::Deallocate",
               "allocator is not initialized");
  // A stack allocator does not support individual deallocations, as it is
  // intended for temporary data only. Memory is only released when Clear() is
  // called, which resets the entire stack.
}

void FiberStackAllocator::Clear() {
  COMET_ASSERT(IsInitialized(), "FiberStackAllocator::Clear",
               "allocator is not initialized");

  for (auto& context : thread_contexts_) {
    context.marker = context.root;
  }

#ifdef COMET_DEBUG_STACK_ALLOCATOR
  ++clear_count_;
#endif  // COMET_DEBUG_STACK_ALLOCATOR

#if defined(COMET_DEBUG_STACK_ALLOCATOR) && \
    defined(COMET_VERBOSE_ALLOCATOR_LOGS)
  COMET_LOG_DEBUG(LoggerType::Core, "FiberStackAllocator::Clear",
                  "clear fiber stack allocator", "tag",
                  GetMemoryTagLabel(memory_tag_), "extended_tag",
                  GetMemoryTagLabel(extended_memory_tag_), "thread_allocations",
                  thread_allocation_count_, "common_allocations",
                  common_allocation_count_, "extended_allocations",
                  extended_allocation_count_, "extended_grow_count",
                  extended_grow_count_, "clear_count", clear_count_,
                  "peak_common_used_size", peak_common_used_size_,
                  "peak_extended_used_size", peak_extended_used_size_,
                  "total_extended_capacity", total_extended_capacity_,
                  "peak_extended_capacity", peak_extended_capacity_);
#endif  // defined(COMET_DEBUG_STACK_ALLOCATOR) &&
        //  defined(COMET_VERBOSE_ALLOCATOR_LOGS)

  if (extended_root_ != nullptr) {
    TaggedHeap::Get().DeallocateAll(extended_memory_tag_);
    extended_root_ = extended_marker_ = nullptr;
    extended_capacity_ = 0;
  }

  marker_ = root_;

#ifdef COMET_DEBUG_STACK_ALLOCATOR
  total_extended_capacity_ = 0;
  peak_extended_used_size_ = 0;
#endif  // COMET_DEBUG_STACK_ALLOCATOR
}

void FiberStackAllocator::OnInitialize() {
  thread_capacity_ = TaggedHeap::Get().GetBlockSize() - 1;
  thread_contexts_.Initialize();

  COMET_ASSERT(base_capacity_ > 0, "FiberStackAllocator::OnInitialize",
               "base capacity is invalid", "base_capacity", base_capacity_);

  const auto size{thread_contexts_.GetSize()};
  auto& tagged_heap{TaggedHeap::Get()};

  for (usize i{0}; i < size; ++i) {
    auto& context{thread_contexts_.GetFromIndex(i)};
    context.root = context.marker =
        static_cast<u8*>(tagged_heap.AllocateAligned(thread_capacity_,
                                                     alignof(u8), memory_tag_));
  }

#ifdef COMET_DEBUG_STACK_ALLOCATOR
  thread_allocation_count_ = 0;
  common_allocation_count_ = 0;
  extended_allocation_count_ = 0;
  extended_grow_count_ = 0;
  clear_count_ = 0;
  peak_common_used_size_ = 0;
  peak_extended_used_size_ = 0;
  total_extended_capacity_ = 0;
  peak_extended_capacity_ = 0;
#endif  // COMET_DEBUG_STACK_ALLOCATOR
}

void FiberStackAllocator::OnDestroy() {
  thread_contexts_.Destroy();
  auto& tagged_heap{TaggedHeap::Get()};
  tagged_heap.DeallocateAll(memory_tag_);
  tagged_heap.DeallocateAll(extended_memory_tag_);
  root_ = nullptr;
  base_capacity_ = 0;
  marker_ = nullptr;

  extended_root_ = nullptr;
  extended_marker_ = nullptr;
  extended_capacity_ = 0;

  thread_capacity_ = 0;

#ifdef COMET_DEBUG_STACK_ALLOCATOR
  thread_allocation_count_ = 0;
  common_allocation_count_ = 0;
  extended_allocation_count_ = 0;
  extended_grow_count_ = 0;
  clear_count_ = 0;
  peak_common_used_size_ = 0;
  peak_extended_used_size_ = 0;
  total_extended_capacity_ = 0;
  peak_extended_capacity_ = 0;
#endif  // COMET_DEBUG_STACK_ALLOCATOR
}

void FiberStackAllocator::AllocateCommonMemory() {
  COMET_ASSERT(IsInitialized(), "FiberStackAllocator::AllocateCommonMemory",
               "allocator is not initialized");
  root_ = static_cast<u8*>(
      TaggedHeap::Get().Allocate(base_capacity_, memory_tag_, &base_capacity_));
  marker_ = root_;

  COMET_ASSERT(root_ != nullptr, "FiberStackAllocator::AllocateCommonMemory",
               "failed to allocate common stack memory", "capacity",
               base_capacity_);
}

void FiberStackAllocator::ExtendCommonMemory(usize capacity) {
  COMET_ASSERT(IsInitialized(), "FiberStackAllocator::ExtendCommonMemory",
               "allocator is not initialized");
  COMET_ASSERT(capacity > 0, "FiberStackAllocator::ExtendCommonMemory",
               "capacity is invalid", "capacity", capacity);
  COMET_ASSERT(extended_memory_tag_ != kEngineMemoryTagInvalid,
               "FiberStackAllocator::ExtendCommonMemory",
               "extended memory is disabled");

  extended_root_ = static_cast<u8*>(TaggedHeap::Get().Allocate(
      capacity, extended_memory_tag_, &extended_capacity_));

  COMET_ASSERT(extended_root_ != nullptr,
               "FiberStackAllocator::ExtendCommonMemory",
               "failed to allocate extended stack memory", "requested_capacity",
               capacity, "actual_capacity", extended_capacity_);
  COMET_ASSERT(
      extended_capacity_ >= capacity, "FiberStackAllocator::ExtendCommonMemory",
      "allocated capacity smaller than requested", "requested_capacity",
      capacity, "actual_capacity", extended_capacity_);

#ifdef COMET_DEBUG_STACK_ALLOCATOR
  ++extended_grow_count_;
  total_extended_capacity_ += extended_capacity_;

  peak_extended_capacity_ = extended_capacity_ > peak_extended_capacity_
                                ? extended_capacity_
                                : peak_extended_capacity_;
#endif  // COMET_DEBUG_STACK_ALLOCATOR

  extended_marker_ = extended_root_;

#if defined(COMET_DEBUG_STACK_ALLOCATOR) && \
    defined(COMET_VERBOSE_ALLOCATOR_LOGS)
  COMET_LOG_DEBUG(LoggerType::Core, "FiberStackAllocator::ExtendCommonMemory",
                  "extended stack memory allocated", "tag",
                  GetMemoryTagLabel(extended_memory_tag_), "requested_capacity",
                  capacity, "actual_capacity", extended_capacity_,
                  "extended_grow_count", extended_grow_count_,
                  "total_extended_capacity", total_extended_capacity_,
                  "peak_extended_capacity", peak_extended_capacity_, "root",
                  extended_root_);
#endif  // defined(COMET_DEBUG_STACK_ALLOCATOR) &&
        //  defined(COMET_VERBOSE_ALLOCATOR_LOGS)
}

IOStackAllocator::IOStackAllocator(usize thread_capacity, MemoryTag memory_tag)
    : memory_tag_{memory_tag}, thread_capacity_{thread_capacity} {}

void* IOStackAllocator::AllocateAligned(usize size, Alignment align) {
  COMET_ASSERT(IsInitialized(), "IOStackAllocator::AllocateAligned",
               "allocator is not initialized");
  COMET_ASSERT(align > 0, "IOStackAllocator::AllocateAligned",
               "alignment is invalid", "align", align);
  COMET_ASSERT(thread_capacity_ > 0, "IOStackAllocator::AllocateAligned",
               "capacity is invalid", "capacity", thread_capacity_);
  COMET_ASSERT(size > 0, "IOStackAllocator::AllocateAligned",
               "allocation size is zero");

  auto& context{thread_contexts_.Get()};
  COMET_ASSERT(context.root != nullptr, "IOStackAllocator::AllocateAligned",
               "thread context root is null");
  COMET_ASSERT(context.marker != nullptr, "IOStackAllocator::AllocateAligned",
               "thread context marker is null");

  auto* p{AlignPointer(context.marker, align)};

  COMET_ASSERT(p + size <= context.root + thread_capacity_,
               "IOStackAllocator::AllocateAligned",
               "allocation exceeds thread capacity", "size", size,
               "thread_capacity", thread_capacity_);

  context.marker = p + size;

#ifdef COMET_DEBUG_STACK_ALLOCATOR
  ++allocation_count_;
#endif  // COMET_DEBUG_STACK_ALLOCATOR

  return p;
}

void IOStackAllocator::Deallocate(void*) {
  COMET_ASSERT(IsInitialized(), "IOStackAllocator::Deallocate",
               "allocator is not initialized");
  // A stack allocator does not support individual deallocations, as it is
  // intended for temporary data only. Memory is only released when Clear() is
  // called, which resets the entire stack.
}

void IOStackAllocator::Clear() {
  COMET_ASSERT(IsInitialized(), "IOStackAllocator::Clear",
               "allocator is not initialized");

  for (auto& context : thread_contexts_) {
    context.marker = context.root;
  }

#ifdef COMET_DEBUG_STACK_ALLOCATOR
  ++clear_count_;
#endif  // COMET_DEBUG_STACK_ALLOCATOR
}

void IOStackAllocator::OnInitialize() {
  COMET_ASSERT(thread_capacity_ > 0, "IOStackAllocator::OnInitialize",
               "thread capacity is invalid", "thread_capacity",
               thread_capacity_);

  thread_contexts_.Initialize();

  const auto requested_thread_capacity{thread_capacity_};
  auto actual_thread_capacity{requested_thread_capacity};

  auto& tagged_heap{TaggedHeap::Get()};
  const auto size{thread_contexts_.GetSize()};

  for (usize i{0}; i < size; ++i) {
    auto& context{thread_contexts_.GetFromIndex(i)};
    auto capacity{requested_thread_capacity};

    context.root = context.marker =
        static_cast<u8*>(tagged_heap.AllocateAligned(capacity, alignof(u8),
                                                     memory_tag_, &capacity));

    COMET_ASSERT(context.root != nullptr, "IOStackAllocator::OnInitialize",
                 "failed to allocate thread stack", "thread_index", i,
                 "requested_capacity", requested_thread_capacity,
                 "actual_capacity", capacity);

    COMET_ASSERT(capacity >= requested_thread_capacity,
                 "IOStackAllocator::OnInitialize",
                 "allocated capacity smaller than requested", "thread_index", i,
                 "requested_capacity", requested_thread_capacity,
                 "actual_capacity", capacity);

    actual_thread_capacity =
        capacity < actual_thread_capacity ? capacity : actual_thread_capacity;
  }

  thread_capacity_ = actual_thread_capacity;

#ifdef COMET_DEBUG_STACK_ALLOCATOR
  allocation_count_ = 0;
  clear_count_ = 0;
#endif  // COMET_DEBUG_STACK_ALLOCATOR
}

void IOStackAllocator::OnDestroy() {
  thread_contexts_.Destroy();

#if defined(COMET_DEBUG_STACK_ALLOCATOR) && \
    defined(COMET_VERBOSE_ALLOCATOR_LOGS)
  COMET_LOG_DEBUG(LoggerType::Core, "IOStackAllocator::OnDestroy",
                  "destroy io stack allocator", "tag",
                  GetMemoryTagLabel(memory_tag_), "thread_capacity",
                  thread_capacity_, "allocation_count", allocation_count_,
                  "clear_count", clear_count_);
#endif  // defined(COMET_DEBUG_STACK_ALLOCATOR) &&
        //  defined(COMET_VERBOSE_ALLOCATOR_LOGS)

  TaggedHeap::Get().DeallocateAll(memory_tag_);
  thread_capacity_ = 0;

#ifdef COMET_DEBUG_STACK_ALLOCATOR
  allocation_count_ = 0;
  clear_count_ = 0;
#endif  // COMET_DEBUG_STACK_ALLOCATOR
}

LockFreeStackAllocator::LockFreeStackAllocator(usize capacity,
                                               MemoryTag memory_tag)
    : memory_tag_{memory_tag},
      capacity_{capacity},
      offset_{kInvalidOffset_},
      root_{nullptr} {}

void* LockFreeStackAllocator::AllocateAligned(usize size, Alignment align) {
  COMET_ASSERT(IsInitialized(), "LockFreeStackAllocator::AllocateAligned",
               "allocator is not initialized");
  COMET_ASSERT(root_ != nullptr, "LockFreeStackAllocator::AllocateAligned",
               "allocator root is null");
  COMET_ASSERT(capacity_ > 0, "LockFreeStackAllocator::AllocateAligned",
               "capacity is invalid", "capacity", capacity_);
  COMET_ASSERT(align > 0, "LockFreeStackAllocator::AllocateAligned",
               "alignment is invalid", "align", align);
  COMET_ASSERT(size > 0, "LockFreeStackAllocator::AllocateAligned",
               "allocation size is zero");

  auto current_offset{offset_.load(std::memory_order_relaxed)};

  COMET_ASSERT(current_offset >= 0, "LockFreeStackAllocator::AllocateAligned",
               "offset is invalid", "offset", current_offset);

  auto aligned_offset{
      AlignAddress(reinterpret_cast<uptr>(root_ + current_offset), align) -
      reinterpret_cast<uptr>(root_)};
  auto new_offset{aligned_offset + size};

  COMET_ASSERT(new_offset <= capacity_,
               "LockFreeStackAllocator::AllocateAligned",
               "allocation exceeds capacity", "size", size, "capacity",
               capacity_, "new_offset", new_offset);

  while (!offset_.compare_exchange_weak(current_offset, new_offset,
                                        std::memory_order_acq_rel,
                                        std::memory_order_relaxed)) {
    COMET_ASSERT(current_offset >= 0, "LockFreeStackAllocator::AllocateAligned",
                 "offset is invalid", "offset", current_offset);

    aligned_offset =
        AlignAddress(reinterpret_cast<uptr>(root_ + current_offset), align) -
        reinterpret_cast<uptr>(root_);
    new_offset = aligned_offset + size;

    COMET_ASSERT(new_offset <= capacity_,
                 "LockFreeStackAllocator::AllocateAligned",
                 "allocation exceeds capacity", "size", size, "capacity",
                 capacity_, "new_offset", new_offset);
  }

#ifdef COMET_DEBUG_STACK_ALLOCATOR
  allocation_count_.fetch_add(1, std::memory_order_relaxed);
  auto peak{peak_used_size_.load(std::memory_order_relaxed)};

  while (new_offset > peak &&
         !peak_used_size_.compare_exchange_weak(peak, new_offset,
                                                std::memory_order_relaxed)) {
  }
#endif  // COMET_DEBUG_STACK_ALLOCATOR

  return root_ + aligned_offset;
}

void LockFreeStackAllocator::Deallocate(void*) {
  COMET_ASSERT(IsInitialized(), "LockFreeStackAllocator::Deallocate",
               "allocator is not initialized");
  // A stack allocator does not support individual deallocations, as it is
  // intended for temporary data only. Memory is only released when Clear() is
  // called, which resets the entire stack.
}

void LockFreeStackAllocator::Clear() {
  COMET_ASSERT(IsInitialized(), "LockFreeStackAllocator::Clear",
               "allocator is not initialized");
  offset_.store(0, std::memory_order_release);
#ifdef COMET_DEBUG_STACK_ALLOCATOR
  clear_count_.fetch_add(1, std::memory_order_relaxed);
#endif  // COMET_DEBUG_STACK_ALLOCATOR
}

void LockFreeStackAllocator::OnInitialize() {
  COMET_ASSERT(capacity_ > 0, "LockFreeStackAllocator::OnInitialize",
               "capacity is invalid", "capacity", capacity_);
  offset_ = 0;
  root_ = static_cast<u8*>(TaggedHeap::Get().AllocateAligned(
      capacity_, alignof(u8), memory_tag_, &capacity_));

#ifdef COMET_DEBUG_STACK_ALLOCATOR
  allocation_count_.store(0, std::memory_order_relaxed);
  peak_used_size_.store(0, std::memory_order_relaxed);
  clear_count_.store(0, std::memory_order_relaxed);
#endif  // COMET_DEBUG_STACK_ALLOCATOR
}

void LockFreeStackAllocator::OnDestroy() {
#if defined(COMET_DEBUG_STACK_ALLOCATOR) && \
    defined(COMET_VERBOSE_ALLOCATOR_LOGS)
  COMET_LOG_DEBUG(
      LoggerType::Core, "LockFreeStackAllocator::OnDestroy",
      "destroy lock-free stack allocator", "tag",
      GetMemoryTagLabel(memory_tag_), "capacity", capacity_, "allocation_count",
      allocation_count_.load(std::memory_order_relaxed), "clear_count",
      clear_count_.load(std::memory_order_relaxed), "peak_used_size",
      peak_used_size_.load(std::memory_order_relaxed));
#endif  // defined(COMET_DEBUG_STACK_ALLOCATOR) &&
        //  defined(COMET_VERBOSE_ALLOCATOR_LOGS)

  TaggedHeap::Get().DeallocateAll(memory_tag_);
  offset_ = kInvalidOffset_;
  root_ = nullptr;
  capacity_ = 0;

#ifdef COMET_DEBUG_STACK_ALLOCATOR
  allocation_count_.store(0, std::memory_order_relaxed);
  peak_used_size_.store(0, std::memory_order_relaxed);
  clear_count_.store(0, std::memory_order_relaxed);
#endif  // COMET_DEBUG_STACK_ALLOCATOR
}
}  // namespace memory
}  // namespace comet
