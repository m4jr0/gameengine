// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_TYPE_REGION_MAP_H_
#define COMET_COMET_CORE_TYPE_REGION_MAP_H_

#include "comet/core/essentials.h"
#include "comet/core/memory/allocator/allocator.h"
#include "comet/core/type/bitset.h"

namespace comet {
class RegionMap {
 public:
  RegionMap() = default;
  RegionMap(memory::Allocator* allocator, usize block_size, usize size);
  RegionMap(const RegionMap& other) = default;
  RegionMap(RegionMap&& other) noexcept = default;
  RegionMap& operator=(const RegionMap& other) = default;
  RegionMap& operator=(RegionMap&& other) noexcept = default;
  ~RegionMap() = default;

  void Destroy();

  void Clear();
  void Resize(usize new_size);

  usize Claim(usize size);
  void Release(usize block_index, usize size);

  usize GetBlockSize() const noexcept;
  usize GetSourceSize() const noexcept;

 private:
  void Grow(usize size);

  usize block_size_{0};
  usize size_{0};
  Bitset block_map_{};
};
}  // namespace comet

#endif  // COMET_COMET_CORE_TYPE_REGION_MAP_H_
