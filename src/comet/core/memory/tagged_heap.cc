// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "tagged_heap.h"

#include <new>

#include "comet/core/memory/allocation_tracking.h"
#include "comet/core/memory/memory_utils.h"
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
  block_size_ = memory_descr_.large_page_size != 0
                    ? memory_descr_.large_page_size
                    : memory_descr_.page_size;

  total_block_count_ = capacity_ / block_size_;
  COMET_ASSERT(total_block_count_ > 0,
               "Total capacity must be at least one block.");

  memory_ = ReserveVirtualMemory(capacity_);
  memory_ = CommitVirtualMemory(memory_, capacity_);

  COMET_ASSERT(memory_ != nullptr, "Failed to allocate memory!");
  COMET_REGISTER_TAGGED_HEAP_POOL_ALLOCATION(capacity_);

  // Set the capacity for all bitsets (including the global one), with
  // additional alignment overhead.
  bitset_allocator_ = PlatformStackAllocator{
      Bitset::GetWordCountFromBitCount(total_block_count_) *
              sizeof(Bitset::Word) * (kMaxTagCount_ + 1) +
          alignof(Bitset::Word),
      kEngineMemoryTagTaggedHeap};
  bitset_allocator_.Initialize();
  global_block_map_ = Bitset{&bitset_allocator_, total_block_count_};

  for (auto& bucket : tag_block_maps_) {
    for (auto& entry : bucket) {
      entry.block_map = Bitset{&bitset_allocator_, total_block_count_};
      entry.tag = kEngineMemoryTagInvalid;
    }
  }

  is_initialized_ = true;
}

void TaggedHeap::Destroy() {
  COMET_ASSERT(is_initialized_,
               "Tried to destroy tagged heap, but it is not initialized!");

  global_block_map_.Clear();

  for (auto& bucket : tag_block_maps_) {
    for (auto& entry : bucket) {
      entry.block_map.Clear();
      entry.tag = kEngineMemoryTagInvalid;
    }
  }

  // Free all bitsets at once.
  bitset_allocator_.Destroy();

  if (memory_ != nullptr) {
    FreeVirtualMemory(memory_, capacity_);
    COMET_REGISTER_TAGGED_HEAP_POOL_DEALLOCATION(capacity_);
  }

  is_initialized_ = false;
}

void* TaggedHeap::Allocate(usize size, MemoryTag tag, usize* out_size) {
  return AllocateAligned(size, 1, tag, out_size);
}

void* TaggedHeap::AllocateAligned(usize size, Alignment align, MemoryTag tag,
                                  usize* out_size) {
  usize final_size{size + align};
  usize block_count;
  auto* ptr{static_cast<u8*>(AllocateInternal(final_size, tag, block_count))};
  final_size = block_count * block_size_;

  if (out_size != nullptr) {
    *out_size = final_size;
  }

  if (ptr == nullptr) {
    COMET_ASSERT(false, "Unable to allocate with size ", size, ", alignment ",
                 align, " and tag ", GetMemoryTagLabel(tag), "!");
    throw std::bad_alloc();
  }

  COMET_REGISTER_TAGGED_HEAP_ALLOCATION(final_size, tag);
  auto* aligned_ptr{AlignPointer(static_cast<u8*>(ptr), align)};

  // Case: pointer is already aligned. We have a minimal shift of 1 byte, so we
  // move the pointer to "align" bytes as a convention.
  if (aligned_ptr == ptr) {
    aligned_ptr += align;
  }

  auto shift{aligned_ptr - ptr};
  COMET_ASSERT(shift > 0 && shift <= kMaxAlignment,
               "Invalid shift in memory allocation! Shift is ", shift,
               ", but it must be between ", 0, " and ", kMaxAlignment, ".");

#ifdef COMET_GCC
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Warray-bounds"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstringop-overflow"
#endif  // COMET_GCC
  // Set shift to 0 if it equals kMaxAlignment.
  aligned_ptr[-1] = shift & (static_cast<u8>(kMaxAlignment - 1));
#ifdef COMET_GCC
#pragma GCC diagnostic pop
#pragma GCC diagnostic pop
#endif  // COMET_GCC
  return aligned_ptr;
}

void* TaggedHeap::AllocateBlock(MemoryTag tag, usize* out_size) {
  return AllocateBlockAligned(1, tag, out_size);
}

void* TaggedHeap::AllocateBlockAligned(Alignment align, MemoryTag tag,
                                       usize* out_size) {
  return AllocateAligned(block_size_ - align, align, tag, out_size);
}

void* TaggedHeap::AllocateBlocks(usize count, MemoryTag tag, usize* out_size) {
  return AllocateBlocksAligned(count, 1, tag, out_size);
}

void* TaggedHeap::AllocateBlocksAligned(usize count, Alignment align,
                                        MemoryTag tag, usize* out_size) {
  return AllocateAligned(block_size_ * count - align, align, tag, out_size);
}

void TaggedHeap::DeallocateAll(MemoryTag tag) {
  {
    fiber::FiberLockGuard lock{mutex_};
    auto* block_map{FindOrAddTag(tag)};

    if (block_map == nullptr) {
      return;
    }

    for (usize i{0}; i < total_block_count_; ++i) {
      if (!block_map->block_map.Test(i)) {
        continue;
      }

      block_map->block_map.Reset(i);
      global_block_map_.Reset(i);
    }
  }

  COMET_REGISTER_TAGGED_HEAP_DEALLOCATION(tag);
}

bool TaggedHeap::IsInitialized() const noexcept { return is_initialized_; }

usize TaggedHeap::GetBlockSize() const noexcept { return block_size_; }

void* TaggedHeap::AllocateInternal(usize size, MemoryTag tag,
                                   usize& block_count) {
  COMET_ASSERT(memory_ != nullptr,
               "Cannot allocate! No memory is available...");
  block_count = (size + block_size_ - 1) / block_size_;
  test += block_count;
  COMET_ASSERT(size <= capacity_, "Max capacity reached!");
  usize free_blocks_index;

  {
    fiber::FiberLockGuard lock{mutex_};
    free_blocks_index = ResolveFreeBlocks(block_count);

    if (free_blocks_index == kInvalidIndex) {
      COMET_ASSERT(false,
                   "No sufficient memory available for allocation with size ",
                   size, " and tag ", GetMemoryTagLabel(tag), "!");
      throw std::bad_alloc();
    }

    auto* block_map{FindOrAddTag(tag)};

    for (usize i{0}; i < block_count; ++i) {
      auto map_index{free_blocks_index + i};
      global_block_map_.Set(map_index);
      block_map->block_map.Set(map_index);
    }
  }

  auto* ptr{static_cast<u8*>(memory_) + free_blocks_index * block_size_};
  COMET_POISON(ptr, size);
  return ptr;
}

usize TaggedHeap::ResolveFreeBlocks(usize block_count) const {
  usize contiguous_block_count{0};

  for (usize i{0}; i < total_block_count_; ++i) {
    if (global_block_map_.Test(i)) {
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

TaggedHeap::TagBlockMap* TaggedHeap::FindOrAddTag(MemoryTag tag) {
  auto bucket_index{static_cast<usize>(tag % kBucketCount_)};
  auto& bucket{tag_block_maps_[bucket_index]};

  for (auto& entry : bucket) {
    if (entry.tag == tag) {
      return &entry;
    }

    if (entry.tag == kEngineMemoryTagInvalid) {
      entry.tag = tag;
      return &entry;
    }
  }

  COMET_ASSERT(false, "No entry found: the bucket seems to be full!");
  return nullptr;
}
}  // namespace memory
}  // namespace comet
