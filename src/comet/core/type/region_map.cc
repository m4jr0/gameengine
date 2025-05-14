// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "comet_pch.h"

#include "region_map.h"

#include "comet/core/memory/memory_utils.h"

namespace comet {
RegionMap::RegionMap(memory::Allocator* allocator, usize block_size, usize size)
    : block_size_{block_size}, block_map_{allocator} {
  COMET_ASSERT(block_size_ > 0, "Block size is 0!");
  COMET_ASSERT(size % block_size_ == 0, "Size (", size,
               ") is not divisible by the block size (", block_size_, ")!");
  Grow(size);
}

void RegionMap::Destroy() {
  block_map_.Destroy();
  block_size_ = 0;
  size_ = 0;
}

void RegionMap::Clear() { block_map_.Clear(); }

void RegionMap::Resize(usize size) {
  if (size <= size_) {
    return;
  }

  Grow(size - size_);
}

usize RegionMap::Claim(usize size) {
  COMET_ASSERT(size > 0, "Claimed size is 0!");
  size = memory::RoundUpToMultiple(size, block_size_);
  COMET_ASSERT(size % block_size_ == 0, "Claimed size (", size,
               ") is not divisible by the block size (", block_size_, ")!");
  usize required_count{size / block_size_};
  usize counter{0};

  for (usize i{0}; i < block_map_.GetSize(); ++i) {
    if (block_map_.Test(i)) {
      counter = 0;
      continue;
    }

    ++counter;

    if (counter < required_count) {
      continue;
    }

    auto start_index{i - counter + 1};

    for (usize j{start_index}; j <= i; ++j) {
      block_map_.Set(j);
    }

    return start_index * block_size_;
  }

  return kInvalidSize;
}

void RegionMap::Release(usize offset, usize size) {
  COMET_ASSERT(offset % block_size_ == 0, "Offset (", offset,
               ") provided is not divisible by the block size (", block_size_,
               ")!");
  auto index_offset{offset / block_size_};
  COMET_ASSERT(size > 0, "Released size is 0!");
  size = memory::RoundUpToMultiple(size, block_size_);
  COMET_ASSERT(size % block_size_ == 0, "Claimed size (", size,
               ") is not divisible by the block size (", block_size_, ")!");
  auto count{size / block_size_};

  for (usize i{0}; i < count; ++i) {
    block_map_.Reset(i + index_offset);
  }
}

usize RegionMap::GetBlockSize() const noexcept { return block_size_; }

usize RegionMap::GetSourceSize() const noexcept { return size_; }

void RegionMap::Grow(usize size) {
  size_ += size;
  COMET_ASSERT(size_ % block_size_ == 0, "Tried to grow region map by ", size,
               " bytes, but now the total size (", size_,
               ") is not a multiple of block size (", block_size_, ")!");
  block_map_.Resize(size_ / block_size_);
}
}  // namespace comet
