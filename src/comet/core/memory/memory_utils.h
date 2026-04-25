// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_MEMORY_MEMORY_UTILS_H_
#define COMET_COMET_CORE_MEMORY_MEMORY_UTILS_H_

// External. ///////////////////////////////////////////////////////////////////
#include <type_traits>
#include <utility>
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/essentials.h"
#include "comet/core/memory/memory.h"

namespace comet {
namespace memory {
void* CopyMemory(void* dst, const void* src, usize size);
void* MoveMemory(void* dst, const void* src, usize size);
void* CopyOrMoveMemory(void* dst, const void* src, usize size);

void Memset(void* ptr, u8 value, usize size);
void AVXMemset(void* ptr, u8 value, usize size);
void FastMemset(void* ptr, u8 value, usize size);
void ClearMemory(void* ptr, usize size);

inline uptr AlignAddress(uptr address, Alignment align) {
  if (align == 0) {
    return address;
  }

  const usize mask{static_cast<usize>(align) - 1};
  COMET_ASSERT((align & mask) == 0, "memory_utils::AlignAddress",
               "alignment is not a power of 2", "address",
               reinterpret_cast<void*>(address), "alignment", align);

  return (address + mask) & ~mask;
}

inline usize AlignSize(usize size, Alignment align) {
  const auto mask{static_cast<usize>(align) - 1};
  COMET_ASSERT((align & mask) == 0, "memory_utils::AlignSize",
               "alignment is not a power of 2", "size", size, "alignment",
               align);

  return (size + mask) & ~mask;
}

constexpr usize RoundUpToMultiple(usize value, usize multiple) {
  COMET_ASSERT(multiple > 0, "memory_utils::RoundUpToMultiple",
               "multiple must be greater than zero");

  return ((value + multiple - 1) / multiple) * multiple;
}

constexpr usize RoundDownToMultiple(usize value, usize multiple) {
  COMET_ASSERT(multiple > 0, "RoundDownToMultiple",
               "multiple must be greater than zero", "multiple", multiple);

  return value - (value % multiple);
}

constexpr usize RoundDownToMultiplePow2(usize value, usize multiple) {
  COMET_ASSERT(multiple > 0, "memory_utils::RoundDownToMultiplePow2",
               "multiple must be greater than zero", "multiple", multiple);
  COMET_ASSERT((multiple & (multiple - 1)) == 0,
               "memory_utils::RoundDownToMultiplePow2",
               "multiple is not a power of 2", "multiple", multiple);

  return value & ~(multiple - 1);
}

template <typename T>
inline T* AlignPointer(T* ptr, Alignment align) {
  return reinterpret_cast<T*>(AlignAddress(reinterpret_cast<uptr>(ptr), align));
}

class Allocator;

template <typename T, typename... Targs>
T* Populate(void* memory, Targs&&... args) {
  COMET_ASSERT(memory != nullptr, "memory_utils::Populate", "memory is null");
  return new (memory) T(std::forward<Targs>(args)...);
}

template <typename T, typename... Targs>
T* Populate(void* memory, memory::Allocator* allocator, Targs&&... args) {
  COMET_ASSERT(memory != nullptr, "memory_utils::Populate", "memory is null");

  if constexpr (!std::is_aggregate_v<T> && sizeof...(Targs) == 0 &&
                std::is_constructible_v<T, memory::Allocator*>) {
    return new (memory) T(allocator);
  } else if constexpr (!std::is_aggregate_v<T> &&
                       std::is_constructible_v<T, memory::Allocator*,
                                               Targs...>) {
    return new (memory) T(allocator, std::forward<Targs>(args)...);
  } else {
    return new (memory) T(std::forward<Targs>(args)...);
  }
}

void* StoreShiftAndReturnAligned(void* ptr, usize data_size,
                                 [[maybe_unused]] usize allocation_size,
                                 Alignment align);
void* ResolveNonAligned(void* ptr);

template <typename T>
inline bool IsAligned(T* ptr, usize align) noexcept {
  const auto tmp{reinterpret_cast<uptr>(ptr)};
  return tmp % align == 0;
}

void GetMemorySizeString(ssize size, schar* buffer, usize buffer_len,
                         usize* out_len = nullptr);

#ifdef COMET_POISON_ALLOCATIONS
void Poison(void* ptr, usize size);
#endif  // COMET_POISON_ALLOCATIONS

MemoryDescr GetMemoryDescr();

constexpr auto kHexAddressLength{18};  // "0x" + 16 hex digits.
constexpr auto kHexAddressBufferLen{kHexAddressLength + 1};

void ConvertAddressToHex(uptr address, schar* buffer, usize buffer_len);
void ConvertAddressToHex(const void* address, schar* buffer, usize buffer_len);

void* Allocate(usize size, MemoryTag tag = kEngineMemoryTagUntagged);
void* AllocateAligned(usize size, Alignment align, MemoryTag tag);

template <typename T>
T* AllocateMany(usize count, MemoryTag tag) {
  return static_cast<T*>(AllocateAligned(sizeof(T) * count, alignof(T), tag));
}

template <typename T>
T* AllocateOne(MemoryTag tag) {
  return AllocateMany<T>(1, tag);
}

template <typename T, typename... Targs>
T* AllocateOneAndPopulate(MemoryTag tag, Targs&&... args) {
  auto* ptr{AllocateOne<T>(tag)};
  return Populate<T>(ptr, std::forward<Targs>(args)...);
}

void Deallocate(void* ptr);
}  // namespace memory
}  // namespace comet

#ifdef COMET_POISON_ALLOCATIONS
#define COMET_POISON(ptr, size) comet::memory::Poison(ptr, size)
#else
#define COMET_POISON(ptr, size)
#endif  // COMET_POISON_ALLOCATIONS

#endif  // COMET_COMET_CORE_MEMORY_MEMORY_UTILS_H_
