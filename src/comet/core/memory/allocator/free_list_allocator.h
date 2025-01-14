// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_MEMORY_ALLOCATOR_FREE_LIST_ALLOCATOR_H_
#define COMET_COMET_CORE_MEMORY_ALLOCATOR_FREE_LIST_ALLOCATOR_H_

#include "comet/core/concurrency/fiber/fiber_primitive.h"
#include "comet/core/essentials.h"
#include "comet/core/memory/allocator/allocator.h"
#include "comet/core/memory/memory.h"

namespace comet {
namespace memory {
class FiberFreeListAllocator : public Allocator {
 public:
  FiberFreeListAllocator() = default;
  FiberFreeListAllocator(usize allocation_unit, usize block_count,
                         MemoryTag memory_tag);
  FiberFreeListAllocator(const FiberFreeListAllocator&) = delete;
  FiberFreeListAllocator(FiberFreeListAllocator&&) noexcept;
  FiberFreeListAllocator& operator=(const FiberFreeListAllocator&) = delete;
  FiberFreeListAllocator& operator=(FiberFreeListAllocator&&) noexcept;
  ~FiberFreeListAllocator() = default;

  void Initialize() override;
  void Destroy() override;

  void* AllocateAligned(usize size, Alignment align) override;
  void Deallocate(void* ptr) override;
  void DeallocateAll();

 private:
  struct Block {
    bool is_free{false};
    Block* next{nullptr};
    usize size{0};
  };

  Block* Grow(usize size);
  Block* ReserveBlocks(usize size);

  usize block_size_{0};
  usize block_count_{0};
  MemoryTag memory_tag_{kEngineMemoryTagUntagged};
  fiber::FiberMutex mutex_{};
  Block* head_{nullptr};
  Block* tail_{nullptr};
};
}  // namespace memory
}  // namespace comet

#endif  // COMET_COMET_CORE_MEMORY_ALLOCATOR_FREE_LIST_ALLOCATOR_H_
