// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "tagged_heap.h"
////////////////////////////////////////////////////////////////////////////////

// External. ///////////////////////////////////////////////////////////////////
#include <new>
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/memory/allocation_tracking.h"
#include "comet/core/memory/memory_label.h"
#include "comet/core/memory/memory_utils.h"
#include "comet/core/memory/virtual_memory.h"

namespace comet {
namespace memory {
TaggedHeap::~TaggedHeap() {
  COMET_ASSERT(!is_initialized_, "TaggedHeap::~TaggedHeap",
               "tagged heap is still initialized");
}

TaggedHeap& TaggedHeap::Get() {
  static TaggedHeap singleton{};
  return singleton;
}

void TaggedHeap::Initialize() {
  COMET_ASSERT(!is_initialized_, "TaggedHeap::Initialize",
               "tagged heap is already initialized");

  block_size_ = memory_descr_.large_page_size != 0
                    ? memory_descr_.large_page_size
                    : memory_descr_.page_size;

  COMET_ASSERT(block_size_ > kMaxAlignment, "TaggedHeap::Initialize",
               "block size is invalid", "block_size", block_size_,
               "required_min", kMaxAlignment + 1);

  total_block_count_ = capacity_ / block_size_;

  COMET_ASSERT(total_block_count_ > 0, "TaggedHeap::Initialize",
               "total block count is zero", "capacity", capacity_, "block_size",
               block_size_);

  memory_ = ReserveVirtualMemory(capacity_);
  memory_ = CommitVirtualMemory(memory_, capacity_);

  COMET_ASSERT(memory_ != nullptr, "TaggedHeap::Initialize",
               "virtual memory allocation failed", "capacity", capacity_);

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
  COMET_ASSERT(is_initialized_, "TaggedHeap::Destroy",
               "tagged heap is not initialized");

  global_block_map_.Destroy();

  for (auto& bucket : tag_block_maps_) {
    for (auto& entry : bucket) {
      entry.block_map.Destroy();
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
  auto final_size{size + align};
  usize block_count;
  auto* ptr{static_cast<u8*>(AllocateInternal(final_size, tag, block_count))};
  final_size = block_count * block_size_;

  if (out_size != nullptr) {
    *out_size = final_size;
  }

  if (ptr == nullptr) {
    COMET_ASSERT(false, "TaggedHeap::AllocateAligned", "allocation failed",
                 "size", size, "alignment", align, "tag",
                 GetMemoryTagLabel(tag));
    throw std::bad_alloc();
  }

  COMET_REGISTER_TAGGED_HEAP_ALLOCATION(final_size, tag);
  auto* aligned_ptr{AlignPointer(static_cast<u8*>(ptr), align)};

  // Case: pointer is already aligned. We have a minimal shift of 1 byte, so we
  // move the pointer to "align" bytes as a convention.
  if (aligned_ptr == ptr) {
    aligned_ptr += align;
  }

  const auto shift{aligned_ptr - ptr};

  COMET_ASSERT(shift > 0 && shift <= kMaxAlignment,
               "TaggedHeap::AllocateAligned", "alignment shift is invalid",
               "shift", shift, "max_alignment", kMaxAlignment);

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
  COMET_ASSERT(memory_ != nullptr, "TaggedHeap::AllocateInternal",
               "memory is not initialized");

  block_count = (size + block_size_ - 1) / block_size_;

  COMET_ASSERT(block_count <= total_block_count_,
               "TaggedHeap::AllocateInternal",
               "requested block count exceeds capacity", "block_count",
               block_count, "total_block_count", total_block_count_);

  usize free_blocks_index;

  {
    fiber::FiberLockGuard lock{mutex_};
    free_blocks_index = ResolveFreeBlocks(block_count);

    if (free_blocks_index == kInvalidIndex) {
      COMET_ASSERT(false, "TaggedHeap::AllocateInternal",
                   "no contiguous block range is available", "size", size,
                   "tag", GetMemoryTagLabel(tag), "block_count", block_count);

      throw std::bad_alloc();
    }

    auto* block_map{FindOrAddTag(tag)};

    for (usize i{0}; i < block_count; ++i) {
      const auto map_index{free_blocks_index + i};
      global_block_map_.Set(map_index);
      block_map->block_map.Set(map_index);
    }
  }

  auto* ptr{static_cast<u8*>(memory_) + free_blocks_index * block_size_};
  COMET_POISON(ptr, block_count * block_size_);
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
  const auto bucket_index{static_cast<usize>(tag % kBucketCount_)};
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

  COMET_ASSERT(false, "TaggedHeap::FindOrAddTag", "tag bucket is full", "tag",
               GetMemoryTagLabel(tag), "bucket_index", bucket_index);

  return nullptr;
}
}  // namespace memory
}  // namespace comet
