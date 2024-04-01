// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_MEMORY_ALLOCATOR_TSTRING_ALLOCATOR_H_
#define COMET_COMET_CORE_MEMORY_ALLOCATOR_TSTRING_ALLOCATOR_H_

#include "comet/core/essentials.h"

namespace comet {
namespace memory {
using TStringAllocatorMarker = u8*;

class TStringAllocator {
 public:
  TStringAllocator() = delete;
  explicit TStringAllocator(usize capacity);
  TStringAllocator(const TStringAllocator&) = delete;
  TStringAllocator(TStringAllocator&&) = delete;
  TStringAllocator& operator=(const TStringAllocator&) = delete;
  TStringAllocator& operator=(TStringAllocator&&) = delete;
  ~TStringAllocator();

  void Initialize();
  void Destroy();
  void* Allocate(usize size);
  void Deallocate(void* p);
  void Clear();
  bool IsInitialized() const noexcept;
  usize GetSize() const noexcept;

 private:
  bool is_initialized_{false};
};
}  // namespace memory
}  // namespace comet

#endif  // COMET_COMET_CORE_MEMORY_ALLOCATOR_TSTRING_ALLOCATOR_H_
