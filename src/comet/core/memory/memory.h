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

#ifdef COMET_ALLOW_CUSTOM_MEMORY_TAG_LABELS
using GetCustomMemoryTagLabelFunc = const schar* (*)(MemoryTag);

namespace internal {
extern GetCustomMemoryTagLabelFunc get_custom_memory_tag_label_func;
}  // namespace internal

void AttachGetCustomMemoryTagLabelFunc(GetCustomMemoryTagLabelFunc func);
void DetachGetCustomMemoryTagLabelFunc();
#endif  // COMET_ALLOW_CUSTOM_MEMORY_TAG_LABELS

const schar* GetMemoryTagLabel(MemoryTag tag);
void* CopyMemory(void* dst, const void* src, usize size);
void ClearMemory(void* ptr, usize size);

inline uptr AlignAddress(uptr address, Alignment align) {
  if (align == 0) {
    return address;
  }

  const usize mask{static_cast<usize>(align) - 1};
  COMET_ASSERT((align & mask) == 0, "Bad alignment provided for address ",
               reinterpret_cast<void*>(address), ": ", align,
               "! Must be a power of 2.");
  return (address + mask) & ~mask;
}

inline usize AlignSize(usize size, Alignment align) {
  const auto mask{static_cast<usize>(align) - 1};
  COMET_ASSERT((align & mask) == 0, "Bad alignment provided for size ", size,
               ": ", align, "! Must be a power of 2.");
  return (size + mask) & ~mask;
}

template <typename T>
inline T* AlignPointer(T* ptr, Alignment align) {
  return reinterpret_cast<T*>(AlignAddress(reinterpret_cast<uptr>(ptr), align));
}

template <typename T, typename... Targs>
T* Populate(void* memory, Targs&&... args) {
  COMET_ASSERT(memory != nullptr, "Memory provided is null!");
  return new (memory) T{std::forward<Targs>(args)...};
}

void* StoreShiftAndReturnAligned(u8* ptr, usize data_size,
                                 [[maybe_unused]] usize allocation_size,
                                 Alignment align);
void* ResolveNonAligned(void* ptr);

template <typename T>
inline bool IsAligned(T* ptr, usize align) noexcept {
  auto tmp{reinterpret_cast<uptr>(ptr)};
  return tmp % align == 0;
}

void GetMemorySizeString(ssize size, schar* buffer, usize buffer_len,
                         usize* out_len = nullptr);

#ifdef COMET_POISON_ALLOCATIONS
void Poison(void* ptr, usize size);
#endif  // COMET_POISON_ALLOCATIONS

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

#ifdef COMET_ALLOW_CUSTOM_MEMORY_TAG_LABELS
#define COMET_ATTACH_CUSTOM_MEMORY_LABEL_FUNC(func) \
  comet::memory::AttachGetCustomMemoryTagLabelFunc(func)
#define COMET_DETACH_CUSTOM_MEMORY_LABEL_FUNC() \
  comet::memory::DetachGetCustomMemoryTagLabelFunc()
#else
#define COMET_ATTACH_CUSTOM_MEMORY_LABEL_FUNC(func)
#define COMET_DETACH_CUSTOM_MEMORY_LABEL_FUNC()
#endif  // COMET_ALLOW_CUSTOM_MEMORY_TAG_LABELS

#endif  // COMET_COMET_CORE_MEMORY_MEMORY_H_
