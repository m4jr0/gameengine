// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_MEMORY_TAGGED_HEAP_H_
#define COMET_COMET_CORE_MEMORY_TAGGED_HEAP_H_

#include "comet/core/concurrency/fiber/fiber_primitive.h"
#include "comet/core/conf/configuration_manager.h"
#include "comet/core/conf/configuration_value.h"
#include "comet/core/essentials.h"
#include "comet/core/memory/allocator/platform_allocator.h"
#include "comet/core/memory/memory.h"
#include "comet/core/type/bitset.h"

namespace comet {
namespace memory {
struct TaggedHeapArena {
  TaggedHeapArena* next{nullptr};
  u8* head{nullptr};
  u8* end{nullptr};
  u8 data{0};
};

class TaggedHeap {
 public:
  TaggedHeap() = default;
  TaggedHeap(const TaggedHeap&) = delete;
  TaggedHeap(TaggedHeap&&) = delete;
  TaggedHeap& operator=(const TaggedHeap&) = delete;
  TaggedHeap& operator=(TaggedHeap&&) = delete;
  ~TaggedHeap();

  static TaggedHeap& Get();
  void Initialize();
  void Destroy();

  void* Allocate(usize size, MemoryTag tag, usize* out_size = nullptr);
  void* AllocateAligned(usize size, Alignment align, MemoryTag tag,
                        usize* out_size = nullptr);
  void* AllocateBlock(MemoryTag tag, usize* out_size = nullptr);
  void* AllocateBlockAligned(Alignment align, MemoryTag tag,
                             usize* out_size = nullptr);
  void* AllocateBlocks(usize count, MemoryTag tag, usize* out_size = nullptr);
  void* AllocateBlocksAligned(usize count, Alignment align, MemoryTag tag,
                              usize* out_size = nullptr);
  void DeallocateAll(MemoryTag tag);

  bool IsInitialized() const noexcept;
  usize GetBlockSize() const noexcept;

 private:
  usize test{0};
  static inline constexpr usize kBucketCount_{100};
  static inline constexpr usize kTagsPerBucketCount_{10};
  static inline constexpr usize kMaxTagCount_{kBucketCount_ *
                                              kTagsPerBucketCount_};

  struct TagBlockMap {
    MemoryTag tag{kEngineMemoryTagUntagged};
    FixedBitset block_map{};
  };

  void* AllocateInternal(usize size, MemoryTag tag, usize& block_count);
  usize ResolveFreeBlocks(usize block_count) const;
  TagBlockMap* FindOrAddTag(MemoryTag tag);

  bool is_initialized_{false};
  usize total_block_count_{0};
  usize block_size_{0};
  usize capacity_{COMET_CONF_U32(conf::kCoreTaggedHeapCapacity)};
  MemoryDescr memory_descr_{GetMemoryDescr()};
  FixedBitset global_block_map_{};
  TagBlockMap tag_block_maps_[kBucketCount_][kTagsPerBucketCount_]{};
  PlatformStackAllocator bitset_allocator_{};
  mutable fiber::FiberMutex mutex_{};
  void* memory_{nullptr};
};
}  // namespace memory
}  // namespace comet

#endif  // COMET_COMET_CORE_MEMORY_TAGGED_HEAP_H_
