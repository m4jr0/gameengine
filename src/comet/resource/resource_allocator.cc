// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "resource_allocator.h"

#include <type_traits>

namespace comet {
namespace resource {
namespace internal {
void ResourceAllocator::Initialize() {
  small_allocator_ = memory::FiberFreeListAllocator{
      kSmallAllocatorAllocationUnitSize_,
      kSmallAllocatorCapacity_ / kSmallAllocatorAllocationUnitSize_,
      memory::kEngineMemoryTagResource};
  small_allocator_.Initialize();
}

void ResourceAllocator::Destroy() { small_allocator_.Destroy(); }

void* ResourceAllocator::AllocateAligned(usize size, memory::Alignment align) {
  auto total_size{size + align + kHeaderSize_};

  u8* raw{nullptr};
  u8 tag{0};

  if (total_size <= kAllocationThresholdSize_) {
    raw = static_cast<u8*>(small_allocator_.Allocate(total_size));
    tag = static_cast<std::underlying_type_t<AllocatorType>>(
        AllocatorType::Small);
  } else {
    raw = static_cast<u8*>(big_allocator_.Allocate(total_size));
    tag =
        static_cast<std::underlying_type_t<AllocatorType>>(AllocatorType::Big);
  }

  raw[0] = tag;
  auto* ptr{static_cast<u8*>(memory::StoreShiftAndReturnAligned(
      raw + kHeaderSize_, size, total_size - kHeaderSize_, align))};

  return ptr;
}

void ResourceAllocator::Deallocate(void* ptr) {
  COMET_ASSERT(
      ptr != nullptr,
      "Tried to deallocate from resource allocator, but pointer is null!");

  auto* offset{static_cast<u8*>(memory::ResolveNonAligned(ptr))};
  auto* raw{offset - kHeaderSize_};
  auto tag{static_cast<AllocatorType>(raw[0])};

  if (tag == AllocatorType::Small) {
    small_allocator_.Deallocate(raw);
  } else {
    big_allocator_.Deallocate(raw);
  }
}
}  // namespace internal
}  // namespace resource
}  // namespace comet
