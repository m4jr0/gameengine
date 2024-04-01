// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_MEMORY_TAGGED_HEAP_H_
#define COMET_COMET_CORE_MEMORY_TAGGED_HEAP_H_

#include <vector>

#include "comet/core/conf/configuration_manager.h"
#include "comet/core/conf/configuration_value.h"
#include "comet/core/essentials.h"
#include "comet/core/memory/memory.h"

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

  void* Allocate(usize size, MemoryTag tag);
  void Deallocate(MemoryTag tag);

  bool IsInitialized() const noexcept;

 private:
  usize ResolveFreeBlocks(usize block_count) const;
  bool is_initialized_{false};
  usize total_block_count_{0};
  usize block_size_{0};
  usize capacity_{COMET_CONF_U32(conf::kCoreTwoFrameAllocatorCapacity)};
  MemoryDescr memory_descr_{GetMemoryDescr()};
  std::vector<bool> free_blocks{};
  std::vector<std::vector<bool>> tag_bitlist;
  void* memory_{nullptr};
};
}  // namespace memory
}  // namespace comet

#endif  // COMET_COMET_CORE_MEMORY_TAGGED_HEAP_H_
