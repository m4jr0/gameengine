// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/runtime/resource/resource_allocator.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/debug_label.h"
#include "comet/core/type_trait.h"

namespace comet {
namespace resource {
namespace internal {
const schar* GetAllocatorTypeLabel(AllocatorType type) {
  switch (type) {
    case AllocatorType::Unknown:
      return "unknown";
    case AllocatorType::Small:
      return "small";
    case AllocatorType::Big:
      return "big";
    default:
      return kUnknownLabel;
  }
}

void* ResourceAllocator::AllocateAligned(usize size, memory::Alignment align) {
  COMET_ASSERT(size > 0, "ResourceAllocator::AllocateAligned",
               "allocation size is zero");
  COMET_ASSERT(align != 0, "ResourceAllocator::AllocateAligned",
               "alignment is zero");
  const auto total_size{size + align + kHeaderSize_};

  u8* raw{nullptr};
  u8 tag{0};

  if (total_size <= kAllocationThresholdSize_) {
    raw = static_cast<u8*>(small_allocator_.Allocate(total_size));
    tag = ToUnderlying(AllocatorType::Small);
  } else {
    raw = static_cast<u8*>(big_allocator_.Allocate(total_size));
    tag = ToUnderlying(AllocatorType::Big);
  }

  COMET_ASSERT(raw != nullptr, "ResourceAllocator::AllocateAligned",
               "allocator returned null", "size", size, "alignment", align,
               "total_size", total_size);

  raw[0] = tag;
  auto* ptr{static_cast<u8*>(memory::StoreShiftAndReturnAligned(
      raw + kHeaderSize_, size, total_size - kHeaderSize_, align))};

  return ptr;
}

void ResourceAllocator::Deallocate(void* ptr) {
  COMET_ASSERT(ptr != nullptr, "ResourceAllocator::Deallocate",
               "pointer is null");

  auto* offset{static_cast<u8*>(memory::ResolveNonAligned(ptr))};
  auto* raw{offset - kHeaderSize_};
  const auto tag{static_cast<AllocatorType>(raw[0])};

  if (tag == AllocatorType::Small) {
    small_allocator_.Deallocate(raw);
  } else if (tag == AllocatorType::Big) {
    big_allocator_.Deallocate(raw);
  } else {
    COMET_ASSERT(false, "ResourceAllocator::Deallocate",
                 "allocator tag is invalid", "tag", GetAllocatorTypeLabel(tag),
                 "tag_value", ToUnderlying(tag));
  }
}

void ResourceAllocator::OnInitialize() {
  small_allocator_ = memory::FiberFreeListAllocator{
      kSmallAllocatorAllocationUnitSize_,
      kSmallAllocatorCapacity_ / kSmallAllocatorAllocationUnitSize_,
      memory::kEngineMemoryTagResource};
  small_allocator_.Initialize();

  COMET_ASSERT(small_allocator_.IsInitialized(),
               "ResourceAllocator::OnInitialize",
               "small allocator failed to initialize");
}

void ResourceAllocator::OnDestroy() { small_allocator_.Destroy(); }
}  // namespace internal
}  // namespace resource
}  // namespace comet
