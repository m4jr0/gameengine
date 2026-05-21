// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_CORE_CONTAINER_C_ARRAY_H_
#define COMET_CORE_CONTAINER_C_ARRAY_H_

// External. ///////////////////////////////////////////////////////////////////
#include <type_traits>
#include <utility>
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/essentials.h"
#include "comet/core/memory/allocator/allocator.h"
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

  COMET_ASSERT(data != nullptr, "c_array::IsContained", "data is null");

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

  COMET_ASSERT(data != nullptr, "c_array::GetIndex", "data is null");

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

  COMET_ASSERT(data != nullptr, "c_array::Clear", "data is null");

  if constexpr (!std::is_trivially_destructible_v<T>) {
    for (usize i{0}; i < size; ++i) {
      data[i].~T();
    }
  }
}

template <typename T>
T* Reserve(memory::Allocator* allocator, T* data, usize size, usize capacity,
           usize new_capacity) {
  COMET_ASSERT(allocator != nullptr, "c_array::Reserve", "allocator is null");
  COMET_ASSERT(size == 0 || data != nullptr, "c_array::Reserve", "data is null",
               "size", size);

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

template <typename T>
T* TrimCapacity(memory::Allocator* allocator, T* data, usize size,
                usize capacity) {
  COMET_ASSERT(allocator != nullptr, "c_array::TrimCapacity",
               "allocator is null");
  COMET_ASSERT(size == 0 || data != nullptr, "c_array::TrimCapacity",
               "data is null", "size", size);

  if (size == capacity) {
    return data;
  }

  if (size == 0) {
    if (data != nullptr) {
      allocator->Deallocate(data);
    }

    return nullptr;
  }

  auto* new_data{allocator->AllocateMany<T>(size)};

  if constexpr (std::is_trivially_constructible_v<T>) {
    memory::CopyMemory(new_data, data, size * sizeof(T));
  } else {
    for (usize i{0}; i < size; ++i) {
      memory::Populate<T>(&new_data[i], std::move(data[i]));
      data[i].~T();
    }
  }

  allocator->Deallocate(data);
  return new_data;
}

template <typename T,
          typename = std::enable_if_t<
              !std::is_same_v<T, schar> && !std::is_same_v<T, wchar> &&
              !std::is_same_v<std::remove_cv_t<T>, schar> &&
              !std::is_same_v<std::remove_cv_t<T>, wchar>>>
T* Copy(T* dst, [[maybe_unused]] usize dst_size, const T* src,
        [[maybe_unused]] usize src_size, usize count,
        std::size_t dst_offset = 0, std::size_t src_offset = 0) {
  COMET_ASSERT(src != nullptr, "c_array::Copy", "source array is null");

  COMET_ASSERT(dst != nullptr, "c_array::Copy", "destination array is null");

  COMET_ASSERT(src_offset + count <= src_size, "c_array::Copy",
               "source range exceeds bounds", "src_offset", src_offset, "count",
               count, "src_size", src_size);

  COMET_ASSERT(dst_offset + count <= dst_size, "c_array::Copy",
               "destination range exceeds bounds", "dst_offset", dst_offset,
               "count", count, "dst_size", dst_size);

  for (usize i{0}; i < count; ++i) {
    dst[dst_offset + i] = src[src_offset + i];
  }

  return dst;
}
}  // namespace comet

#endif  // COMET_CORE_CONTAINER_C_ARRAY_H_