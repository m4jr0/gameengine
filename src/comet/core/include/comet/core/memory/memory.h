// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_CORE_MEMORY_MEMORY_H_
#define COMET_CORE_MEMORY_MEMORY_H_

// External. ///////////////////////////////////////////////////////////////////
#include <cstddef>
#include <functional>
#include <memory>
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/essentials.h"

namespace comet {
namespace memory {
using Alignment = u16;
constexpr auto kInvalidAlignment{static_cast<Alignment>(-1)};

constexpr u16 kTrivialTypeMaxAlignment{alignof(std::max_align_t)};

constexpr u16 kMaxAlignment{256};
static_assert((kMaxAlignment & (kMaxAlignment - 1)) == 0,
              "kMaxAlignment must be a power of 2");

constexpr Alignment kStackAlignment{16};
static_assert((kStackAlignment & (kStackAlignment - 1)) == 0,
              "kStackAlignment must be a power of 2");

struct MemoryDescr {
  usize total_memory_size{0};
  usize page_size{0};
  usize large_page_size{0};
};

template <typename T>
using UniquePtr = std::unique_ptr<T>;

template <typename T>
using CustomUniquePtr = std::unique_ptr<T, std::function<void(T*)>>;
}  // namespace memory
}  // namespace comet

#endif  // COMET_CORE_MEMORY_MEMORY_H_
