// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RESOURCE_RESOURCE_ALLOCATOR_H_
#define COMET_COMET_RESOURCE_RESOURCE_ALLOCATOR_H_

#include "comet/core/essentials.h"
#include "comet/core/memory/allocator/free_list_allocator.h"
#include "comet/core/memory/allocator/platform_allocator.h"
#include "comet/core/memory/allocator/stateful_allocator.h"
#include "comet/core/memory/memory.h"

namespace comet {
namespace resource {
namespace internal {
enum class AllocatorType : u8 { Unknown = 0, Small = 1, Big = 2 };

class ResourceAllocator : public memory::StatefulAllocator {
 public:
  ResourceAllocator() = default;
  ResourceAllocator(const ResourceAllocator&) = delete;
  ResourceAllocator(ResourceAllocator&& other) noexcept = delete;
  ResourceAllocator& operator=(const ResourceAllocator&) = delete;
  ResourceAllocator& operator=(ResourceAllocator&& other) noexcept = delete;
  ~ResourceAllocator() = default;

  void Initialize() override;
  void Destroy() override;

  void* AllocateAligned(usize size, memory::Alignment align) override;
  void Deallocate(void* ptr) override;

 private:
  inline static constexpr usize kHeaderSize_{sizeof(AllocatorType)};
  inline static constexpr usize kSmallAllocatorAllocationUnitSize_{
      8192};                                                         // 64 KiB.
  inline static constexpr usize kSmallAllocatorCapacity_{16777216};  // 128 MiB.
  inline static constexpr usize kAllocationThresholdSize_{
      20 * kSmallAllocatorAllocationUnitSize_};
  memory::FiberFreeListAllocator small_allocator_{};
  memory::PlatformAllocator big_allocator_{memory::kEngineMemoryTagResource};
};
}  // namespace internal
}  // namespace resource
}  // namespace comet

#endif  // COMET_COMET_RESOURCE_RESOURCE_ALLOCATOR_H_