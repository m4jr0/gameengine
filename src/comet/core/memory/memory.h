// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_MEMORY_MEMORY_H_
#define COMET_COMET_CORE_MEMORY_MEMORY_H_

#include "comet/core/essentials.h"

namespace comet {
namespace memory {
using Alignment = u16;
constexpr auto kInvalidAlignment{static_cast<Alignment>(-1)};

constexpr u16 kMaxAlignment{256};
static_assert((kMaxAlignment & (kMaxAlignment - 1)) == 0,
              "kMaxAlignment must be a power of 2!");
constexpr Alignment kStackAlignment{16};
static_assert((kStackAlignment & (kStackAlignment - 1)) == 0,
              "kStackAlignment must be a power of 2!");

struct MemoryDescr {
  usize total_memory_size;
  usize page_size{0};
  usize large_page_size{0};
};

using MemoryTag = u64;

enum EngineMemoryTag : MemoryTag {
  kEngineMemoryTagUntagged = 0,
  kEngineMemoryTagTaggedHeap,
  kEngineMemoryTagStringId,
  kEngineMemoryTagFrame0,
  kEngineMemoryTagFrame1,
  kEngineMemoryTagFrame2,
  kEngineMemoryTagDoubleFrame0,
  kEngineMemoryTagDoubleFrame1,
  kEngineMemoryTagDoubleFrame2,
  kEngineMemoryTagRendering,
  kEngineMemoryTagRenderingInternal,
  kEngineMemoryTagRenderingDevice,
  kEngineMemoryTagTString,
  kEngineMemoryTagEntity,
  kEngineMemoryTagFiber,
  kEngineMemoryTagThreadProvider,
  kEngineMemoryTagEvent,
  kEngineMemoryTagUserBase = kU32Max,
  kEngineMemoryTagInvalid = kU64Max
};
}  // namespace memory
}  // namespace comet

#endif  // COMET_COMET_CORE_MEMORY_MEMORY_H_
