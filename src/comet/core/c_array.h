// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_C_ARRAY_H_
#define COMET_COMET_CORE_C_ARRAY_H_

#include "comet/core/essentials.h"
#include "comet/core/memory/allocator/allocator.h"
#include "comet/core/memory/memory.h"
#include "comet/core/memory/memory_utils.h"

namespace comet {
template <typename T>
bool IsContained(const T* data, usize size, const T& value) {
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
  COMET_ASSERT(new_capacity > 0, "Capacity is 0!");

  if (new_capacity <= capacity) {
    return data;
  }

  auto* new_data{allocator->AllocateMany<T>(new_capacity)};

  if (data != nullptr) {
    for (usize i{0}; i < size; ++i) {
      memory::Populate<T>(&new_data[i], std::move(data[i]));
      data[i].~T();
    }

    allocator->Deallocate(data);
  }

  return new_data;
}
}  // namespace comet

#endif  // COMET_COMET_CORE_C_ARRAY_H_