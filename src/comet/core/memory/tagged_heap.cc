// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "tagged_heap.h"

#include "comet/core/memory/virtual_memory.h"

namespace comet {
namespace memory {
TaggedHeap::~TaggedHeap() {
  COMET_ASSERT(
      !is_initialized_,
      "Destructor called for tagged heap, but it is still initialized!");
}

TaggedHeap& TaggedHeap::Get() {
  static TaggedHeap singleton{};
  return singleton;
}

void TaggedHeap::Initialize() {
  COMET_ASSERT(!is_initialized_,
               "Tried to initialize tagged heap, but it is already done!");
  block_size_ = memory_descr_.page_size;
  total_block_count_ = capacity_ / block_size_;
  COMET_ASSERT(total_block_count_ > 0,
               "Total capacity must be at least one block.");

  memory_ = ReserveVirtualMemory(capacity_);
  memory_ = CommitVirtualMemory(memory_, capacity_);
  COMET_ASSERT(memory_ != nullptr, "Failed to allocate memory!");
  free_blocks.resize(total_block_count_, true);
  tag_bitlist.resize(kMaxMemoryTagCount);

  for (usize i{0}; i < kMaxMemoryTagCount; ++i) {
    tag_bitlist[i].resize(total_block_count_, false);
  }

  is_initialized_ = true;
}

void TaggedHeap::Destroy() {
  COMET_ASSERT(is_initialized_,
               "Tried to destroy tagged heap, but it is not initialized!");

  if (memory_ != nullptr) {
    FreeVirtualMemory(memory_, capacity_);
  }

  is_initialized_ = false;
}

void* TaggedHeap::Allocate(usize size, MemoryTag tag) {
  COMET_ASSERT(memory_ != nullptr,
               "Cannot allocate! No memory is available...");
  auto block_count{(size + block_size_ - 1) / block_size_};
  COMET_ASSERT(size <= capacity_, "Max capacity reached!");
  auto tag_index{GetMemoryTagIndex(tag)};
  COMET_ASSERT(tag_index >= 0 && tag_index < kMaxMemoryTagCount,
               "Invalid tag: ", GetMemoryTagLabel(tag), "!");

  auto free_blocks_index{ResolveFreeBlocks(block_count)};
  COMET_ASSERT(free_blocks_index != kInvalidIndex,
               "No sufficient memory available for allocation with size ", size,
               " and tag ", GetMemoryTagLabel(tag), "!");

  for (usize i{0}; i < block_count; ++i) {
    free_blocks[free_blocks_index + i] = false;
    tag_bitlist[tag_index][free_blocks_index + i] = true;
  }

  return static_cast<u8*>(memory_) + free_blocks_index * block_size_;
}

void TaggedHeap::Deallocate(MemoryTag tag) {
  auto tag_index{GetMemoryTagIndex(tag)};
  COMET_ASSERT(tag_index >= 0 && tag_index < kMaxMemoryTagCount,
               "Invalid tag: ", GetMemoryTagLabel(tag), "!");

  for (usize i{0}; i < free_blocks.size(); ++i) {
    if (!tag_bitlist[tag_index][i]) {
      continue;
    }

    free_blocks[i] = true;
    tag_bitlist[tag_index][i] = false;
  }
}

bool TaggedHeap::IsInitialized() const noexcept { return is_initialized_; }

usize TaggedHeap::ResolveFreeBlocks(usize block_count) const {
  usize contiguous_block_count{0};

  for (usize i{0}; i < total_block_count_; ++i) {
    if (!free_blocks[i]) {
      contiguous_block_count = 0;
      continue;
    }

    ++contiguous_block_count;

    if (contiguous_block_count == block_count) {
      return i - (block_count - 1);
    }
  }

  return kInvalidIndex;
}
}  // namespace memory
}  // namespace comet
