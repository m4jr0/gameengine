// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "free_list_allocator.h"
////////////////////////////////////////////////////////////////////////////////

// External. ///////////////////////////////////////////////////////////////////
#include <utility>
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/memory/memory_utils.h"
#include "comet/core/memory/tagged_heap.h"

#if defined(COMET_DEBUG_FREE_LIST_ALLOCATOR) && \
    defined(COMET_VERBOSE_ALLOCATOR_LOGS)
#include "comet/core/logger/logging.h"
#include "comet/core/memory/memory_label.h"
#endif  // defined(COMET_DEBUG_FREE_LIST_ALLOCATOR) &&
        // defined(COMET_VERBOSE_ALLOCATOR_LOGS)

namespace comet {
namespace memory {
FiberFreeListAllocator::FiberFreeListAllocator(usize allocation_unit_size,
                                               usize block_count,
                                               MemoryTag memory_tag)
    : block_size_{memory::AlignSize(allocation_unit_size + sizeof(Block),
                                    alignof(Block))},
      initial_block_count_{block_count},
      free_block_count_{0},
      memory_tag_{memory_tag} {
  COMET_ASSERT(block_size_ >= allocation_unit_size + sizeof(Block),
               "FiberFreeListAllocator::FiberFreeListAllocator",
               "block size too small for allocation unit", "block_size",
               block_size_, "required_min",
               allocation_unit_size + sizeof(Block));
  COMET_ASSERT(block_size_ > 0,
               "FiberFreeListAllocator::FiberFreeListAllocator",
               "invalid block size", "block_size", block_size_);
  COMET_ASSERT(initial_block_count_ > 0,
               "FiberFreeListAllocator::FiberFreeListAllocator",
               "invalid initial block count", "initial_block_count",
               initial_block_count_);
}

FiberFreeListAllocator::FiberFreeListAllocator(
    FiberFreeListAllocator&& other) noexcept
    : StatefulAllocator{std::move(other)},
      block_size_{other.block_size_},
      initial_block_count_{other.initial_block_count_},
      free_block_count_{other.free_block_count_},
      memory_tag_{other.memory_tag_},
      head_{other.head_},
      tail_{other.tail_} {
  other.block_size_ = 0;
  other.initial_block_count_ = 0;
  other.free_block_count_ = 0;
  other.memory_tag_ = kEngineMemoryTagUntagged;
  other.head_ = nullptr;
  other.tail_ = nullptr;
}

FiberFreeListAllocator& FiberFreeListAllocator::operator=(
    FiberFreeListAllocator&& other) noexcept {
  if (this == &other) {
    return *this;
  }

  StatefulAllocator::operator=(std::move(other));
  block_size_ = other.block_size_;
  initial_block_count_ = other.initial_block_count_;
  free_block_count_ = other.free_block_count_;
  memory_tag_ = other.memory_tag_;
  head_ = other.head_;
  tail_ = other.tail_;

  other.block_size_ = 0;
  other.initial_block_count_ = 0;
  other.free_block_count_ = 0;
  other.memory_tag_ = kEngineMemoryTagUntagged;
  other.head_ = nullptr;
  other.tail_ = nullptr;
  return *this;
}

void* FiberFreeListAllocator::AllocateAligned(usize size, Alignment align) {
  COMET_ASSERT(IsInitialized(), "FiberFreeListAllocator::AllocateAligned",
               "allocator is not initialized");
  COMET_ASSERT(size > 0, "FiberFreeListAllocator::AllocateAligned",
               "allocation size is zero");

  // Add alignment storage + header of first block.
  const auto allocation_size{size + align + sizeof(Block)};

  fiber::FiberLockGuard lock{mutex_};

  auto* head_block{ReserveBlocks(allocation_size)};

  if (head_block == nullptr) {
    head_block = Grow(allocation_size);
  }

  COMET_ASSERT(head_block != nullptr, "FiberFreeListAllocator::AllocateAligned",
               "allocation failed", "requested_size", size, "allocation_size",
               allocation_size);

  auto* ptr{reinterpret_cast<u8*>(head_block) + sizeof(Block)};
  auto* cursor{head_block->next};

  head_block->is_free = false;
  --free_block_count_;

  // Remove header of first block.
  auto new_block_size{block_size_};

  while (new_block_size < allocation_size) {
    COMET_ASSERT(cursor != nullptr, "FiberFreeListAllocator::AllocateAligned",
                 "ran out of blocks while reserving allocation");
    COMET_ASSERT(cursor->is_free, "FiberFreeListAllocator::AllocateAligned",
                 "encountered non-free block during allocation");

    new_block_size += block_size_;
    --free_block_count_;
    cursor = cursor->next;
  }

  head_block->next = cursor;
  head_block->size = new_block_size;

  if (cursor == nullptr) {
    tail_ = head_block;
  }

  COMET_ASSERT(head_block->size > 0, "FiberFreeListAllocator::AllocateAligned",
               "invalid block size after allocation", "block_size",
               head_block->size);
  COMET_ASSERT(head_block->size % block_size_ == 0,
               "FiberFreeListAllocator::AllocateAligned",
               "block size is not aligned to allocator block size",
               "block_size", head_block->size, "allocator_block_size",
               block_size_);

#ifdef COMET_DEBUG_FREE_LIST_ALLOCATOR
  head_block->magic = kMagicAllocated_;
  head_block->requested_size = size;
  ++live_allocation_count_;
  live_requested_size_ += size;
  live_reserved_size_ += new_block_size;

  DebugValidate("FiberFreeListAllocator::AllocateAligned");
#endif  // COMET_DEBUG_FREE_LIST_ALLOCATOR

  return StoreShiftAndReturnAligned(ptr, size, new_block_size, align);
}

void FiberFreeListAllocator::Deallocate(void* ptr) {
  COMET_ASSERT(IsInitialized(), "FiberFreeListAllocator::Deallocate",
               "allocator is not initialized");
  COMET_ASSERT(ptr != nullptr, "FiberFreeListAllocator::Deallocate",
               "pointer is null");

  auto* head_block{reinterpret_cast<Block*>(
      static_cast<u8*>(ResolveNonAligned(ptr)) - sizeof(Block))};

  auto saved_block_size{head_block->size};

  COMET_ASSERT(saved_block_size > 0, "FiberFreeListAllocator::Deallocate",
               "invalid block size during deallocation", "block_size",
               saved_block_size);

  fiber::FiberLockGuard lock{mutex_};

  COMET_ASSERT(head_block->size >= block_size_,
               "FiberFreeListAllocator::Deallocate",
               "block size smaller than allocator block size", "block_size",
               head_block->size, "allocator_block_size", block_size_);

#ifdef COMET_DEBUG_FREE_LIST_ALLOCATOR
  COMET_ASSERT(head_block->magic == kMagicAllocated_,
               "FiberFreeListAllocator::Deallocate",
               "invalid or double-freed block", "block", head_block, "magic",
               head_block->magic);
  COMET_ASSERT(live_allocation_count_ > 0, "FiberFreeListAllocator::Deallocate",
               "live allocation count underflow", "live_allocation_count",
               live_allocation_count_);
  COMET_ASSERT(live_reserved_size_ >= head_block->size,
               "FiberFreeListAllocator::Deallocate",
               "live reserved size underflow", "live_reserved_size",
               live_reserved_size_, "block_size", head_block->size);
  COMET_ASSERT(live_requested_size_ >= head_block->requested_size,
               "FiberFreeListAllocator::Deallocate",
               "live requested size underflow", "live_requested_size",
               live_requested_size_, "requested_size",
               head_block->requested_size);

  --live_allocation_count_;
  live_reserved_size_ -= head_block->size;
  live_requested_size_ -= head_block->requested_size;

  const auto region_id{head_block->region_id};
  const auto block_index{head_block->block_index};

  head_block->magic = kMagicDeallocated_;
  head_block->requested_size = 0;
  head_block->region_id = region_id;
  head_block->block_index = block_index;
#endif  // COMET_DEBUG_FREE_LIST_ALLOCATOR

  auto* block{head_block};
  auto* saved_next{head_block->next};

  head_block->is_free = true;
  head_block->size = block_size_;
  ++free_block_count_;

  saved_block_size -= block_size_;

  while (saved_block_size > 0) {
    auto* next_block{
        reinterpret_cast<Block*>(reinterpret_cast<u8*>(block) + block_size_)};

    block->next = next_block;
    block = next_block;

    block->is_free = true;
    block->size = block_size_;

#ifdef COMET_DEBUG_FREE_LIST_ALLOCATOR
    block->magic = kMagicDeallocated_;
    block->requested_size = 0;
    block->region_id = head_block->region_id;
    block->block_index =
        head_block->block_index +
        ((reinterpret_cast<u8*>(block) - reinterpret_cast<u8*>(head_block)) /
         block_size_);
#endif  // COMET_DEBUG_FREE_LIST_ALLOCATOR

    ++free_block_count_;
    saved_block_size -= block_size_;
  }

  block->next = saved_next;

  if (saved_next == nullptr) {
    tail_ = block;
  }

#ifdef COMET_DEBUG_FREE_LIST_ALLOCATOR
  DebugValidate("FiberFreeListAllocator::Deallocate");
#endif  // COMET_DEBUG_FREE_LIST_ALLOCATOR
}

void FiberFreeListAllocator::DeallocateAll() {
  fiber::FiberLockGuard lock{mutex_};

#ifdef COMET_DEBUG_FREE_LIST_ALLOCATOR
#ifdef COMET_VERBOSE_ALLOCATOR_LOGS
  COMET_LOG_DEBUG(LoggerType::Core, "FiberFreeListAllocator::DeallocateAll",
                  "deallocate all", "tag", GetMemoryTagLabel(memory_tag_),
                  "tag_value", memory_tag_);
#endif  // COMET_VERBOSE_ALLOCATOR_LOGS
  live_allocation_count_ = 0;
  live_requested_size_ = 0;
  live_reserved_size_ = 0;
#endif  // COMET_DEBUG_FREE_LIST_ALLOCATOR

  TaggedHeap::Get().DeallocateAll(memory_tag_);
  head_ = nullptr;
  tail_ = nullptr;
  free_block_count_ = 0;
}

void FiberFreeListAllocator::OnInitialize() {
  fiber::FiberLockGuard lock{mutex_};
  COMET_ASSERT(head_ == nullptr, "FiberFreeListAllocator::OnInitialize",
               "allocator already has a list");
  COMET_ASSERT(tail_ == nullptr, "FiberFreeListAllocator::OnInitialize",
               "allocator already has a tail");
  COMET_ASSERT(free_block_count_ == 0, "FiberFreeListAllocator::OnInitialize",
               "allocator already tracks free blocks");

  Grow(block_size_ * initial_block_count_);
}

void FiberFreeListAllocator::OnDestroy() {
  DeallocateAll();
  memory_tag_ = kEngineMemoryTagUntagged;
  block_size_ = 0;
  initial_block_count_ = 0;
  free_block_count_ = 0;
}

FiberFreeListAllocator::Block* FiberFreeListAllocator::Grow(usize size) {
#ifdef COMET_DEBUG_FREE_LIST_ALLOCATOR
  const auto region_id{next_region_id_++};
#endif  // COMET_DEBUG_FREE_LIST_ALLOCATOR

  const auto requested_size{memory::RoundUpToMultiple(size, block_size_)};

  auto actual_size{requested_size};
  auto* head_block{static_cast<Block*>(TaggedHeap::Get().AllocateAligned(
      actual_size, alignof(Block), memory_tag_, &actual_size))};

  COMET_ASSERT(head_block != nullptr, "FiberFreeListAllocator::Grow",
               "allocation failed", "requested_size", requested_size,
               "actual_size", actual_size, "block_size", block_size_);

  const auto usable_size{memory::RoundDownToMultiple(actual_size, block_size_)};

  COMET_ASSERT(usable_size >= requested_size, "FiberFreeListAllocator::Grow",
               "usable grown size is smaller than requested", "requested_size",
               requested_size, "actual_size", actual_size, "usable_size",
               usable_size, "block_size", block_size_);

  const auto block_count{usable_size / block_size_};

  COMET_ASSERT(block_count > 0, "FiberFreeListAllocator::Grow",
               "grown block count is zero", "usable_size", usable_size,
               "block_size", block_size_);

  auto* cursor{head_block};
  Block* new_tail{nullptr};

  for (usize i{0}; i < block_count; ++i) {
    cursor->is_free = true;
    cursor->size = block_size_;

#ifdef COMET_DEBUG_FREE_LIST_ALLOCATOR
    cursor->magic = kMagicDeallocated_;
    cursor->requested_size = 0;
    cursor->region_id = region_id;
    cursor->block_index = i;
#endif  // COMET_DEBUG_FREE_LIST_ALLOCATOR

    new_tail = cursor;

    if (i + 1 < block_count) {
      auto* next{reinterpret_cast<Block*>(reinterpret_cast<u8*>(cursor) +
                                          block_size_)};
      cursor->next = next;
      cursor = next;
    } else {
      cursor->next = nullptr;
    }
  }

  if (head_ == nullptr) {
    COMET_ASSERT(tail_ == nullptr, "FiberFreeListAllocator::Grow",
                 "tail is not null while head is null", "tail", tail_);
    head_ = head_block;
  } else {
    COMET_ASSERT(tail_ != nullptr, "FiberFreeListAllocator::Grow",
                 "tail is null while head is not null");
    COMET_ASSERT(tail_->next == nullptr, "FiberFreeListAllocator::Grow",
                 "tail is not the last block", "tail", tail_, "tail_next",
                 tail_->next);

    tail_->next = head_block;
  }

  tail_ = new_tail;
  free_block_count_ += block_count;

#ifdef COMET_DEBUG_FREE_LIST_ALLOCATOR
  ++grow_count_;
  total_grown_size_ += usable_size;
  DebugValidate("FiberFreeListAllocator::Grow");
#endif  // COMET_DEBUG_FREE_LIST_ALLOCATOR

  return head_block;
}

FiberFreeListAllocator::Block* FiberFreeListAllocator::ReserveBlocks(
    usize size) {
  if (head_ == nullptr) {
#if defined(COMET_DEBUG_FREE_LIST_ALLOCATOR) && \
    defined(COMET_VERBOSE_ALLOCATOR_LOGS)
    const auto stats{GetDebugStatsUnlocked()};

    COMET_LOG_DEBUG(LoggerType::Core, "FiberFreeListAllocator::ReserveBlocks",
                    "failed to reserve blocks, head is null", "tag",
                    GetMemoryTagLabel(memory_tag_), "tag_value", memory_tag_,
                    "requested_size", size, "largest_free_run",
                    stats.largest_free_run, "scanned_free_blocks",
                    stats.free_blocks, "scanned_allocated_blocks",
                    stats.allocated_blocks, "tracked_free_blocks",
                    free_block_count_, "head", head_, "tail", tail_);
#endif  // defined(COMET_DEBUG_FREE_LIST_ALLOCATOR) &&
        // defined(COMET_VERBOSE_ALLOCATOR_LOGS)
    return nullptr;
  }

  auto* head_block{head_};
  auto* cursor{head_block};
  usize contiguous_size{0};

  do {
    if (!cursor->is_free) {
      head_block = cursor = cursor->next;
      contiguous_size = 0;
      continue;
    }

    bool is_not_contiguous{reinterpret_cast<u8*>(cursor) !=
                           reinterpret_cast<u8*>(head_block) + contiguous_size};

    if (cursor != head_block && is_not_contiguous) {
      head_block = cursor;
      contiguous_size = 0;
    }

    contiguous_size += cursor->size;
    cursor = cursor->next;
  } while (cursor != nullptr && contiguous_size < size);

  if (contiguous_size < size || head_block == nullptr) {
#if defined(COMET_DEBUG_FREE_LIST_ALLOCATOR) && \
    defined(COMET_VERBOSE_ALLOCATOR_LOGS)
    const auto stats{GetDebugStatsUnlocked()};

    COMET_LOG_DEBUG(
        LoggerType::Core, "FiberFreeListAllocator::ReserveBlocks",
        "failed to reserve blocks", "tag", GetMemoryTagLabel(memory_tag_),
        "tag_value", memory_tag_, "requested_size", size, "largest_free_run",
        stats.largest_free_run, "scanned_free_blocks", stats.free_blocks,
        "scanned_allocated_blocks", stats.allocated_blocks,
        "tracked_free_blocks", free_block_count_, "head", head_, "tail", tail_);
#endif  // defined(COMET_DEBUG_FREE_LIST_ALLOCATOR) &&
        // defined(COMET_VERBOSE_ALLOCATOR_LOGS)
    return nullptr;
  }

  return head_block;
}

#ifdef COMET_DEBUG_FREE_LIST_ALLOCATOR
#ifdef COMET_VERBOSE_ALLOCATOR_LOGS
void FiberFreeListAllocator::DebugLogStats(const schar* context) const {
  fiber::FiberLockGuard lock{mutex_};
  const auto stats{GetDebugStatsUnlocked()};

  COMET_LOG_DEBUG(
      LoggerType::Core, "FiberFreeListAllocator::DebugLogStats", context, "tag",
      GetMemoryTagLabel(memory_tag_), "tag_value", memory_tag_, "block_size",
      block_size_, "tracked_free_blocks", free_block_count_,
      "scanned_free_blocks", stats.free_blocks, "scanned_allocated_blocks",
      stats.allocated_blocks, "largest_free_run", stats.largest_free_run,
      "live_allocation_count", live_allocation_count_, "live_requested_size",
      live_requested_size_, "live_reserved_size", live_reserved_size_,
      "grow_count", grow_count_, "total_grown_size", total_grown_size_);
}
#endif  // COMET_VERBOSE_ALLOCATOR_LOGS

void FiberFreeListAllocator::DebugValidate(const schar* context) const {
  usize scanned_blocks{0};
  usize scanned_free_blocks{0};

  const Block* last_block{nullptr};

  for (auto* cursor{head_}; cursor != nullptr; cursor = cursor->next) {
    last_block = cursor;
    COMET_ASSERT(cursor->size > 0, context, "block has zero size", "block",
                 cursor);
    COMET_ASSERT(cursor->size % block_size_ == 0, context,
                 "block size is not block aligned", "block", cursor, "size",
                 cursor->size, "block_size", block_size_);

    const auto block_count{cursor->size / block_size_};
    scanned_blocks += block_count;

    if (cursor->is_free) {
      scanned_free_blocks += block_count;
    }

    COMET_ASSERT(cursor->next != cursor, context, "block points to itself",
                 "block", cursor);

    const auto expected_next_address =
        reinterpret_cast<const u8*>(cursor) + cursor->size;

    if (cursor->next != nullptr &&
        cursor->region_id == cursor->next->region_id &&
        reinterpret_cast<const u8*>(cursor->next) == expected_next_address) {
      COMET_ASSERT(cursor->next->block_index ==
                       cursor->block_index + cursor->size / block_size_,
                   context, "contiguous next block has wrong index",
                   "block_index", cursor->block_index, "size_blocks",
                   cursor->size / block_size_, "next_block_index",
                   cursor->next->block_index);
    }
  }

  COMET_ASSERT(last_block == tail_, context,
               "tail is not the last reachable block", "reachable_tail",
               last_block, "tracked_tail", tail_);

  if (tail_ != nullptr) {
    COMET_ASSERT(tail_->next == nullptr, context, "tail next is not null",
                 "tail", tail_, "tail_next", tail_->next);
  }

  COMET_ASSERT(scanned_free_blocks == free_block_count_, context,
               "free block count mismatch", "scanned_free_blocks",
               scanned_free_blocks, "tracked_free_blocks", free_block_count_);

#ifdef COMET_VERBOSE_ALLOCATOR_LOGS
  COMET_LOG_DEBUG(LoggerType::Core, "FreeList validate", context, "tag",
                  GetMemoryTagLabel(memory_tag_), "tag_value", memory_tag_,
                  "scanned_blocks", scanned_blocks, "free_block_count",
                  free_block_count_);
#endif  // COMET_VERBOSE_ALLOCATOR_LOGS
}

FiberFreeListAllocator::DebugStats
FiberFreeListAllocator::GetDebugStatsUnlocked() const {
  DebugStats stats{};
  usize current_free_run{0};

  for (auto* cursor{head_}; cursor != nullptr; cursor = cursor->next) {
    if (cursor->is_free) {
      const auto blocks{cursor->size / block_size_};
      stats.free_blocks += blocks;
      current_free_run += cursor->size;

      if (current_free_run > stats.largest_free_run) {
        stats.largest_free_run = current_free_run;
      }
    } else {
      stats.allocated_blocks += cursor->size / block_size_;
      current_free_run = 0;
    }
  }

  return stats;
}
#endif  // COMET_DEBUG_FREE_LIST_ALLOCATOR
}  // namespace memory
}  // namespace comet
