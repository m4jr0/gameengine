// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_core_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/core/container/region_map.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/memory/memory_utils.h"

namespace comet {
RegionMap::RegionMap(memory::Allocator* allocator, usize block_size, usize size)
    : block_size_{block_size}, block_map_{allocator} {
  COMET_ASSERT(block_size_ > 0, "RegionMap::RegionMap", "block size is zero");
  COMET_ASSERT(size % block_size_ == 0, "RegionMap::RegionMap",
               "size is not divisible by block size", "size", size,
               "block_size", block_size_);
  Grow(size);
}

void RegionMap::Destroy() {
  block_map_.Release();
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
  COMET_ASSERT(size > 0, "RegionMap::Claim", "claimed size is zero");
  size = memory::RoundUpToMultiple(size, block_size_);

  COMET_ASSERT(size % block_size_ == 0, "RegionMap::Claim",
               "claimed size is not divisible by block size", "size", size,
               "block_size", block_size_);

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

    const auto start_index{i - counter + 1};

    for (usize j{start_index}; j <= i; ++j) {
      block_map_.Set(j);
    }

    return start_index * block_size_;
  }

  return kInvalidSize;
}

void RegionMap::Release(usize offset, usize size) {
  COMET_ASSERT(offset % block_size_ == 0, "RegionMap::Release",
               "offset is not divisible by block size", "offset", offset,
               "block_size", block_size_);

  const auto index_offset{offset / block_size_};
  COMET_ASSERT(size > 0, "RegionMap::Release", "released size is zero");
  size = memory::RoundUpToMultiple(size, block_size_);

  COMET_ASSERT(size % block_size_ == 0, "RegionMap::Release",
               "released size is not divisible by block size", "size", size,
               "block_size", block_size_);

  const auto count{size / block_size_};

  for (usize i{0}; i < count; ++i) {
    block_map_.Reset(i + index_offset);
  }
}

usize RegionMap::GetBlockSize() const noexcept { return block_size_; }

usize RegionMap::GetSourceSize() const noexcept { return size_; }

void RegionMap::Grow(usize size) {
  size_ += size;

  COMET_ASSERT(size_ % block_size_ == 0, "RegionMap::Grow",
               "total size is not divisible by block size after growth",
               "growth_size", size, "total_size", size_, "block_size",
               block_size_);

  block_map_.Resize(size_ / block_size_);
}
}  // namespace comet
