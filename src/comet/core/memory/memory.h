// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_MEMORY_MEMORY_H_
#define COMET_COMET_CORE_MEMORY_MEMORY_H_

#include "comet/core/debug.h"
#include "comet/core/define.h"
#include "comet/core/logger.h"
#include "comet/core/memory/allocation_tracking.h"
#include "comet/core/type/primitive.h"

namespace comet {
constexpr auto kMaxAlignment{256};

enum class MemoryTag {
  Untagged = 0,
  StringId,
  OneFrame,
  TwoFrames,
  TString,
  Entity
};

const schar* GetMemoryTagLabel(MemoryTag tag);
void* CopyMemory(void* dst, const void* src, uindex size);

inline uptr AlignAddress(uptr address, uindex alignment) {
  if (alignment == 0) {
    return address;
  }

  const uindex mask{alignment - 1};
  COMET_ASSERT((alignment & mask) == 0, "Bad alignment provided for address ",
               reinterpret_cast<void*>(address), ": ", alignment,
               "! Must be a power of 2.");
  return (address + mask) & ~mask;
}

inline uindex AlignSize(uindex size, uindex alignment) {
  const uindex mask{alignment - 1};
  COMET_ASSERT((alignment & mask) == 0, "Bad alignment provided for size ",
               size, ": ", alignment, "! Must be a power of 2.");
  return (size + mask) & ~mask;
}

template <typename T>
inline T* AlignPointer(T* ptr, uindex alignment) {
  return reinterpret_cast<T*>(
      AlignAddress(reinterpret_cast<uptr>(ptr), alignment));
}

void* Allocate(uindex size, MemoryTag tag = MemoryTag::Untagged);
void* AllocateAligned(uindex size, u16 alignment, MemoryTag tag);

template <typename T>
T* AllocateAligned(MemoryTag tag) {
  return AllocateAligned<T>(1, tag);
}

template <typename T>
T* AllocateAligned(uindex count, MemoryTag tag) {
  return reinterpret_cast<T*>(
      AllocateAligned(sizeof(T) * count, alignof(T), tag));
}

void Deallocate(void* p);

template <typename T>
inline bool IsAligned(T* ptr, uindex alignment) noexcept {
  auto tmp{reinterpret_cast<uptr>(ptr)};
  return tmp % alignment == 0;
}

void GetMemorySizeString(uindex size, schar* buffer, uindex buffer_len,
                         uindex* out_len = nullptr);
}  // namespace comet

#endif  // COMET_COMET_CORE_MEMORY_MEMORY_H_
