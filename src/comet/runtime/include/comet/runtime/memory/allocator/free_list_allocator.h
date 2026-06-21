// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RUNTIME_MEMORY_ALLOCATOR_FREE_LIST_ALLOCATOR_H_
#define COMET_RUNTIME_MEMORY_ALLOCATOR_FREE_LIST_ALLOCATOR_H_

#include "comet/core/fiber/fiber_primitive.h"
#include "comet/core/essentials.h"
#include "comet/core/memory/allocator/stateful_allocator.h"
#include "comet/runtime/memory/tagged_memory.h"
#include "comet/core/memory/memory.h"
#include "comet/runtime/memory/memory_tag.h"

namespace comet {
namespace memory {
class FiberFreeListAllocator : public StatefulAllocator {
 public:
  FiberFreeListAllocator() = default;
  FiberFreeListAllocator(usize allocation_unit_size, usize block_count,
                         MemoryTag memory_tag);
  FiberFreeListAllocator(const FiberFreeListAllocator&) = delete;
  FiberFreeListAllocator(FiberFreeListAllocator&&) noexcept;
  FiberFreeListAllocator& operator=(const FiberFreeListAllocator&) = delete;
  FiberFreeListAllocator& operator=(FiberFreeListAllocator&&) noexcept;
  ~FiberFreeListAllocator() override = default;

  void* AllocateAligned(usize size, Alignment align) override;
  void Deallocate(void* ptr) override;
  void DeallocateAll();

 protected:
  void OnInitialize() override;
  void OnDestroy() override;

 private:
  struct Block {
    bool is_free{false};
    Block* next{nullptr};
    usize size{0};
#ifdef COMET_DEBUG_FREE_LIST_ALLOCATOR
    usize requested_size{0};
    usize region_id{0};
    usize block_index{0};
    u32 magic{0};
#endif  // COMET_DEBUG_FREE_LIST_ALLOCATOR
  };

  Block* Grow(usize size);
  Block* ReserveBlocks(usize size);

#ifdef COMET_DEBUG_FREE_LIST_ALLOCATOR
  static inline constexpr u32 kMagicAllocated_{0xC0FFEE42};
  static inline constexpr u32 kMagicDeallocated_{0xDEADDEAD};

#if defined(COMET_DEBUG_FREE_LIST_ALLOCATOR) && \
    defined(COMET_VERBOSE_ALLOCATOR_LOGS)
  void DebugLogStats(const schar* context) const;
#endif  // defined(COMET_DEBUG_FREE_LIST_ALLOCATOR) &&
        // defined(COMET_VERBOSE_ALLOCATOR_LOGS)
  void DebugValidate(const schar* context) const;

  struct DebugStats {
    usize free_blocks{0};
    usize allocated_blocks{0};
    usize largest_free_run{0};
  };

  DebugStats GetDebugStatsUnlocked() const;
#endif  // COMET_DEBUG_FREE_LIST_ALLOCATOR

  usize block_size_{0};
  usize initial_block_count_{0};
  usize free_block_count_{0};
  MemoryTag memory_tag_{kEngineMemoryTagUntagged};
  mutable fiber::FiberMutex mutex_{};
  Block* head_{nullptr};
  Block* tail_{nullptr};

#ifdef COMET_DEBUG_FREE_LIST_ALLOCATOR
  usize live_allocation_count_{0};
  usize live_requested_size_{0};
  usize live_reserved_size_{0};
  usize grow_count_{0};
  usize total_grown_size_{0};
  usize next_region_id_{0};
#endif  // COMET_DEBUG_FREE_LIST_ALLOCATOR
};
}  // namespace memory
}  // namespace comet

#endif  // COMET_RUNTIME_MEMORY_ALLOCATOR_FREE_LIST_ALLOCATOR_H_
