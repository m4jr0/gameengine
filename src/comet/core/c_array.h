// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_C_ARRAY_H_
#define COMET_COMET_CORE_C_ARRAY_H_

#include <type_traits>

#include "comet/core/essentials.h"
#include "comet/core/memory/allocator/allocator.h"
#include "comet/core/memory/memory.h"
#include "comet/core/memory/memory_utils.h"

namespace comet {
template <typename T, usize size,
          typename = std::enable_if_t<
              !std::is_same_v<T, schar> && !std::is_same_v<T, wchar> &&
              !std::is_same_v<std::remove_cv_t<T>, schar> &&
              !std::is_same_v<std::remove_cv_t<T>, wchar>>>
constexpr usize GetLength(const T (&)[size]) noexcept {
  return size;
}

template <typename T>
bool IsContained(const T* data, usize size, const T& value) {
  if (size == 0) {
    return false;
  }

  COMET_ASSERT(data != nullptr, "Data provided is null!");

  for (usize i{0}; i < size; ++i) {
    if (data[i] == value) {
      return true;
    }
  }

  return false;
}

template <typename T>
usize GetIndex(const T* data, usize size, const T& value) {
  if (size == 0) {
    return kInvalidIndex;
  }

  COMET_ASSERT(data != nullptr, "Data provided is null!");

  for (usize i{0}; i < size; ++i) {
    if (data[i] == value) {
      return i;
    }
  }

  return kInvalidIndex;
}

template <typename T>
void Clear(T* data, usize size) {
  if (size == 0) {
    return;
  }

  COMET_ASSERT(data != nullptr, "Data provided is null!");

  for (usize i{0}; i < size; ++i) {
    data[i].~T();
  }
}

template <typename T>
T* Reserve(memory::Allocator* allocator, T* data, usize size, usize capacity,
           usize new_capacity) {
  COMET_ASSERT(allocator != nullptr, "Allocator provided is null!");
  COMET_ASSERT(size == 0 || data != nullptr, "Data provided is null!");

  if (new_capacity <= capacity) {
    return data;
  }

  auto* new_data{allocator->AllocateMany<T>(new_capacity)};

  if (data != nullptr) {
    if constexpr (std::is_trivially_constructible_v<T>) {
      memory::CopyMemory(new_data, data, size * sizeof(T));
    } else {
      for (usize i{0}; i < size; ++i) {
        memory::Populate<T>(&new_data[i], std::move(data[i]));
        data[i].~T();
      }
    }

    allocator->Deallocate(data);
  }

  return new_data;
}

template <typename T,
          typename = std::enable_if_t<
              !std::is_same_v<T, schar> && !std::is_same_v<T, wchar> &&
              !std::is_same_v<std::remove_cv_t<T>, schar> &&
              !std::is_same_v<std::remove_cv_t<T>, wchar>>>
T* Copy(T* dst, usize dst_size, const T* src, usize src_size, usize count,
        std::size_t dst_offset = 0, std::size_t src_offset = 0) {
  COMET_ASSERT(src != nullptr, "Source array provided is null!");
  COMET_ASSERT(dst != nullptr, "Destination array provided is null!");

  COMET_ASSERT(src_offset + count <= src_size,
               "Source range exceeds the source array bounds!");
  COMET_ASSERT(dst_offset + count <= dst_size,
               "Destination range exceeds the destination array bounds!");

  for (usize i{0}; i < count; ++i) {
    dst[dst_offset + i] = src[src_offset + i];
  }

  return dst;
}
}  // namespace comet

#endif  // COMET_COMET_CORE_C_ARRAY_H_