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

namespace comet {
namespace memory {
FiberFreeListAllocator::FiberFreeListAllocator(usize allocation_unit_size,
                                               usize block_count,
                                               MemoryTag memory_tag)
    : block_size_{memory::AlignSize(allocation_unit_size + sizeof(Block),
                                    alignof(Block))},
      block_count_{block_count},
      memory_tag_{memory_tag} {
  COMET_ASSERT(block_size_ >= allocation_unit_size + sizeof(Block),
               "FiberFreeListAllocator::FiberFreeListAllocator",
               "block size too small for allocation unit", "block_size",
               block_size_, "required_min",
               allocation_unit_size + sizeof(Block));
  COMET_ASSERT(block_size_ > 0,
               "FiberFreeListAllocator::FiberFreeListAllocator",
               "invalid block size", "block_size", block_size_);
  COMET_ASSERT(block_count_ > 0,
               "FiberFreeListAllocator::FiberFreeListAllocator",
               "invalid block count", "block_count", block_count_);
}

FiberFreeListAllocator::FiberFreeListAllocator(
    FiberFreeListAllocator&& other) noexcept
    : StatefulAllocator{std::move(other)},
      block_size_{other.block_size_},
      block_count_{other.block_count_},
      memory_tag_{other.memory_tag_},
      head_{other.head_},
      tail_{other.tail_} {
  other.block_size_ = 0;
  other.block_count_ = 0;
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
  block_count_ = other.block_count_;
  memory_tag_ = other.memory_tag_;
  head_ = other.head_;
  tail_ = other.tail_;

  other.block_size_ = 0;
  other.block_count_ = 0;
  other.memory_tag_ = kEngineMemoryTagUntagged;
  other.head_ = nullptr;
  other.tail_ = nullptr;
  return *this;
}

void* FiberFreeListAllocator::AllocateAligned(usize size, Alignment align) {
  COMET_ASSERT(size > 0, "FiberFreeListAllocator::AllocateAligned",
               "allocation size is zero");

  // Add alignment storage + header of first block.
  auto allocation_size{size + align + sizeof(Block)};
  Block* head_block{nullptr};

  fiber::FiberLockGuard lock{mutex_};
  head_block = ReserveBlocks(allocation_size);

  if (head_block == nullptr) {
    head_block = Grow(allocation_size);
  }

  COMET_ASSERT(head_block != nullptr, "FiberFreeListAllocator::AllocateAligned",
               "allocation failed", "requested_size", size, "allocation_size",
               allocation_size);

  auto* ptr{reinterpret_cast<u8*>(head_block) + sizeof(Block)};
  auto* cursor{head_block->next};
  head_block->is_free = false;

  // Remove header of first block.
  auto new_block_size{block_size_};

  while (new_block_size < allocation_size) {
    if (!cursor->is_free) {
      head_block->is_free = true;
    }

    COMET_ASSERT(cursor->is_free, "FiberFreeListAllocator::AllocateAligned",
                 "encountered non-free block during allocation");
    new_block_size += block_size_;
    --block_count_;
    cursor = cursor->next;
  }

  head_block->next = cursor;
  head_block->size = new_block_size;

  COMET_ASSERT(head_block->size > 0, "FiberFreeListAllocator::AllocateAligned",
               "invalid block size after allocation", "block_size",
               head_block->size);
  COMET_ASSERT(head_block->size % block_size_ == 0,
               "FiberFreeListAllocator::AllocateAligned",
               "block size is not aligned to allocator block size",
               "block_size", head_block->size, "allocator_block_size",
               block_size_);

  return StoreShiftAndReturnAligned(ptr, size, new_block_size, align);
}

void FiberFreeListAllocator::Deallocate(void* ptr) {
  auto* head_block{reinterpret_cast<Block*>(
      static_cast<u8*>(ResolveNonAligned(static_cast<u8*>(ptr))) -
      sizeof(Block))};
  auto saved_block_size{head_block->size};

  COMET_ASSERT(saved_block_size > 0, "FiberFreeListAllocator::Deallocate",
               "invalid block size during deallocation", "block_size",
               saved_block_size);

  auto* block{head_block};

  fiber::FiberLockGuard lock{mutex_};
  block->is_free = true;
  auto* saved_next{block->next};

  while (saved_block_size > block_size_) {
    block->next =
        reinterpret_cast<Block*>(reinterpret_cast<u8*>(block) + block_size_);
    block = block->next;
    block->is_free = true;
    block->size = block_size_;
    head_block->size -= block_size_;
    saved_block_size -= block_size_;
    ++block_count_;
  }

  block->next = saved_next;
}

void FiberFreeListAllocator::DeallocateAll() {
  fiber::FiberLockGuard lock{mutex_};
  TaggedHeap::Get().DeallocateAll(memory_tag_);
  head_ = nullptr;
  tail_ = nullptr;
  block_count_ = 0;
}

void FiberFreeListAllocator::OnInitialize() {
  fiber::FiberLockGuard lock{mutex_};
  const auto min_block_count{block_count_};
  block_count_ = 0;
  head_ = Grow(block_size_ * min_block_count);
}

void FiberFreeListAllocator::OnDestroy() {
  DeallocateAll();
  memory_tag_ = kEngineMemoryTagUntagged;
  block_size_ = 0;
  block_count_ = 0;
}

FiberFreeListAllocator::Block* FiberFreeListAllocator::Grow(usize size) {
  size = memory::RoundUpToMultiple(size, block_size_);

  auto* head_block{static_cast<Block*>(TaggedHeap::Get().AllocateAligned(
      size, alignof(Block), memory_tag_, &size))};
  COMET_ASSERT(head_block != nullptr, "FiberFreeListAllocator::Grow",
               "allocation failed", "size", size, "block_size", block_size_);

  auto* cursor{head_block};
  const auto block_count{static_cast<usize>(size / block_size_)};

  for (usize i{0}; i < block_count; ++i) {
    cursor->is_free = true;
    cursor->size = block_size_;

    auto* next{
        reinterpret_cast<Block*>(reinterpret_cast<u8*>(cursor) + block_size_)};

    cursor->next =
        (i + 1 < block_count) ? AlignPointer(next, alignof(Block)) : nullptr;

    cursor = cursor->next;
    ++block_count_;
  }

  if (tail_ != nullptr) {
    tail_->next = head_block;
  }

  tail_ = reinterpret_cast<Block*>(reinterpret_cast<u8*>(head_block) +
                                   (block_count - 1) * block_size_);
  return head_block;
}

FiberFreeListAllocator::Block* FiberFreeListAllocator::ReserveBlocks(
    usize size) {
  if (head_ == nullptr) {
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
    return nullptr;
  }

  return head_block;
}
}  // namespace memory
}  // namespace comet
