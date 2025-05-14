// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_MEMORY_ALLOCATOR_ALLOCATOR_H_
#define COMET_COMET_CORE_MEMORY_ALLOCATOR_ALLOCATOR_H_

#include <utility>

#include "comet/core/essentials.h"
#include "comet/core/memory/memory.h"
#include "comet/core/memory/memory_utils.h"

namespace comet {
namespace memory {
class Allocator {
 public:
  virtual ~Allocator();

  virtual void Initialize();
  virtual void Destroy();

  void* Allocate(usize size);

  template <typename T>
  T* AllocateMany(usize count) {
    return static_cast<T*>(AllocateAligned(sizeof(T) * count, alignof(T)));
  }

  template <typename T>
  T* AllocateOne() {
    return AllocateMany<T>(1);
  }

  template <typename T, typename... Targs>
  T* AllocateOneAndPopulate(Targs... args) {
    auto* ptr{AllocateOne<T>()};
    return Populate<T>(ptr, std::forward<Targs>(args)...);
  }

  virtual void* AllocateAligned(usize size, Alignment align) = 0;
  virtual void Deallocate(void* ptr) = 0;

  bool IsInitialized() const noexcept;

 protected:
  bool is_initialized_{false};
};
}  // namespace memory
}  // namespace comet

#endif  // COMET_COMET_CORE_MEMORY_ALLOCATOR_ALLOCATOR_H_
