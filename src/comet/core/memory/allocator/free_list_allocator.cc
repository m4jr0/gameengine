// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "free_list_allocator.h"

#include "comet/core/memory/tagged_heap.h"

namespace comet {
namespace memory {
FiberFreeListAllocator::FiberFreeListAllocator(usize allocation_unit,
                                               usize block_count,
                                               MemoryTag memory_tag)
    : block_size_{allocation_unit + sizeof(Block)},
      block_count_{block_count},
      memory_tag_{memory_tag} {
  COMET_ASSERT(block_size_ > 0, "Invalid block size!");
  COMET_ASSERT(block_count_ > 0, "Invalid block count!");
}

FiberFreeListAllocator::FiberFreeListAllocator(
    FiberFreeListAllocator&& other) noexcept
    : AlignedAllocator{std::move(other)},
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

  AlignedAllocator::operator=(std::move(other));
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

  other.is_initialized_ = false;
  return *this;
}

void FiberFreeListAllocator::Initialize() {
  AlignedAllocator::Initialize();
  head_ = Grow(block_size_ * block_count_);
}

void FiberFreeListAllocator::Destroy() {
  DeallocateAll();
  memory_tag_ = kEngineMemoryTagUntagged;
  block_size_ = 0;
  block_count_ = 0;
  AlignedAllocator::Destroy();
}

void* FiberFreeListAllocator::AllocateAligned(usize size, Alignment align) {
  COMET_ASSERT(size > 0, "Allocation size provided is 0!");
  // Add alignment storage + header of first block.
  usize allocation_size{size + align + sizeof(Block)};

  fiber::FiberLockGuard lock{mutex_};
  auto* head_block{ReserveBlocks(allocation_size)};

  if (head_block == nullptr) {
    head_block = Grow(allocation_size);
  }

  COMET_ASSERT(head_block != nullptr, "Could not allocate memory (size: ", size,
               ")!");
  auto* ptr{reinterpret_cast<u8*>(head_block) + sizeof(Block)};
  head_block->is_free = false;
  auto* cursor{head_block->next};

  // Remove header of first block.
  auto new_block_size{block_size_};

  while (new_block_size < allocation_size) {
    cursor = cursor->next;
    new_block_size += block_size_;
  }

  head_block->next = cursor;
  head_block->size = new_block_size;
  return StoreShiftAndReturnAligned(ptr, size, allocation_size, align);
}

void FiberFreeListAllocator::Deallocate(void* ptr) {
  auto* block{reinterpret_cast<Block*>(
      static_cast<u8*>(ResolveNonAligned(static_cast<u8*>(ptr))) -
      sizeof(Block))};
  auto saved_block_size{block->size};

  fiber::FiberLockGuard lock{mutex_};
  block->is_free = true;

  while (saved_block_size > block_size_) {
    block->next =
        reinterpret_cast<Block*>(reinterpret_cast<u8*>(block) + block_size_);
    block = block->next;
    block->is_free = true;
    block->size = block_size_;
    saved_block_size -= block_size_;
  }
}

void FiberFreeListAllocator::DeallocateAll() {
  fiber::FiberLockGuard lock{mutex_};
  TaggedHeap::Get().DeallocateAll(memory_tag_);
  head_ = nullptr;
  tail_ = nullptr;
  block_count_ = 0;
}

FiberFreeListAllocator::Block* FiberFreeListAllocator::Grow(usize size) {
  size = (size + block_size_ - 1) / block_size_;
  auto* head_block{static_cast<Block*>(TaggedHeap::Get().AllocateAligned(
      size, alignof(Block), memory_tag_, &size))};
  auto* cursor{head_block};

  while (size > block_size_) {
    cursor->is_free = true;
    cursor->size = block_size_;
    cursor->next =
        reinterpret_cast<Block*>(reinterpret_cast<u8*>(cursor) + block_size_);
    cursor = cursor->next;
    size -= block_size_;
  }

  fiber::FiberLockGuard lock{mutex_};

  if (tail_ != nullptr) {
    tail_->next = head_block;
  }

  tail_ = reinterpret_cast<Block*>(reinterpret_cast<u8*>(cursor) - block_size_);
  tail_->next = nullptr;
  return head_block;
}

FiberFreeListAllocator::Block* FiberFreeListAllocator::ReserveBlocks(
    usize size) {
  auto* head_block{head_};
  auto* cursor{head_block};
  usize contiguous_size{0};

  do {
    if (!cursor->is_free) {
      head_block = cursor = cursor->next;
      contiguous_size = 0;
      continue;
    }

    contiguous_size += cursor->size;
    cursor = cursor->next;
  } while (cursor != nullptr && contiguous_size < size);

  return contiguous_size < size ? nullptr : head_block;
}
}  // namespace memory
}  // namespace comet
