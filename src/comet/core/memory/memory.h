// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_MEMORY_MEMORY_H_
#define COMET_COMET_CORE_MEMORY_MEMORY_H_

#include "comet/core/essentials.h"

namespace comet {
namespace memory {
constexpr auto kMaxAlignment{256};
static_assert((kMaxAlignment & (kMaxAlignment - 1)) == 0,
              "kMaxAlignment must be a power of 2!");
constexpr auto kStackAlignment{16};
static_assert((kStackAlignment & (kStackAlignment - 1)) == 0,
              "kStackAlignment must be a power of 2!");

struct MemoryDescr {
  usize total_memory_size;
  usize page_size{0};
};

enum class MemoryTag {
  Untagged = 0,
  StringId = 1,
  OneFrame = 2,
  TwoFrames = 3,
  TString = 4,
  Entity = 5,
  Fiber = 6
};

constexpr auto kMaxMemoryTagCount{7};

constexpr std::underlying_type_t<MemoryTag> GetMemoryTagIndex(
    MemoryTag tag) noexcept {
  return static_cast<std::underlying_type_t<MemoryTag>>(tag);
}

const schar* GetMemoryTagLabel(MemoryTag tag);
void* CopyMemory(void* dst, const void* src, usize size);

inline uptr AlignAddress(uptr address, usize alignment) {
  if (alignment == 0) {
    return address;
  }

  const usize mask{alignment - 1};
  COMET_ASSERT((alignment & mask) == 0, "Bad alignment provided for address ",
               reinterpret_cast<void*>(address), ": ", alignment,
               "! Must be a power of 2.");
  return (address + mask) & ~mask;
}

inline usize AlignSize(usize size, usize alignment) {
  const usize mask{alignment - 1};
  COMET_ASSERT((alignment & mask) == 0, "Bad alignment provided for size ",
               size, ": ", alignment, "! Must be a power of 2.");
  return (size + mask) & ~mask;
}

template <typename T>
inline T* AlignPointer(T* ptr, usize alignment) {
  return reinterpret_cast<T*>(
      AlignAddress(reinterpret_cast<uptr>(ptr), alignment));
}

void* Allocate(usize size, MemoryTag tag = MemoryTag::Untagged);
void* AllocateAligned(usize size, u16 alignment, MemoryTag tag);

template <typename T>
T* AllocateAligned(MemoryTag tag) {
  return AllocateAligned<T>(1, tag);
}

template <typename T>
T* AllocateAligned(usize count, MemoryTag tag) {
  return reinterpret_cast<T*>(
      AllocateAligned(sizeof(T) * count, alignof(T), tag));
}

void Deallocate(void* p);

template <typename T>
inline bool IsAligned(T* ptr, usize alignment) noexcept {
  auto tmp{reinterpret_cast<uptr>(ptr)};
  return tmp % alignment == 0;
}

void GetMemorySizeString(usize size, schar* buffer, usize buffer_len,
                         usize* out_len = nullptr);

void Poison(void* ptr, usize size);

MemoryDescr GetMemoryDescr();

constexpr auto kHexAddressLength{18};  // "0x" + 16 hex digits.
void ConvertAddressToHex(uptr address, schar* buffer, usize buffer_len);
void ConvertAddressToHex(void* address, schar* buffer, usize buffer_len);
}  // namespace memory
}  // namespace comet

#ifdef COMET_POISON_ALLOCATIONS
#define COMET_POISON(ptr, size) comet::memory::Poison(ptr, size)
#else
#define COMET_POISON(ptr, size)
#endif  // COMET_POISON_ALLOCATIONS

#endif  // COMET_COMET_CORE_MEMORY_MEMORY_H_
