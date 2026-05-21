// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RUNTIME_MEMORY_ALLOCATOR_PLATFORM_ALLOCATOR_H_
#define COMET_RUNTIME_MEMORY_ALLOCATOR_PLATFORM_ALLOCATOR_H_

#include "comet/core/essentials.h"
#include "comet/core/memory/allocator/allocator.h"
#include "comet/core/memory/allocator/stateful_allocator.h"
#include "comet/core/memory/memory.h"

namespace comet {
namespace memory {
class PlatformAllocator : public Allocator {
 public:
  PlatformAllocator(MemoryTag memory_tag);
  PlatformAllocator(const PlatformAllocator&) = default;
  PlatformAllocator(PlatformAllocator&&) = default;
  PlatformAllocator& operator=(const PlatformAllocator&) = default;
  PlatformAllocator& operator=(PlatformAllocator&&) = default;
  ~PlatformAllocator() override = default;

  virtual void* AllocateAligned(usize size, Alignment align) override;
  virtual void Deallocate(void* ptr) override;

 private:
  MemoryTag memory_tag_{kEngineMemoryTagUntagged};
};

class PlatformStackAllocator : public StatefulAllocator {
 public:
  PlatformStackAllocator() = default;
  PlatformStackAllocator(usize capacity, MemoryTag memory_tag);
  PlatformStackAllocator(const PlatformStackAllocator&) = delete;
  PlatformStackAllocator(PlatformStackAllocator&& other) noexcept;
  PlatformStackAllocator& operator=(const PlatformStackAllocator&) = delete;
  PlatformStackAllocator& operator=(PlatformStackAllocator&& other) noexcept;

  void* AllocateAligned(usize size, Alignment align) override;
  void Deallocate(void*) override;

  // These functions are not thread-safe and must only be called during specific
  // synchronization points.
  void Clear();

 protected:
  void OnInitialize() override;
  void OnDestroy() override;

 private:
  using PlatformStackAllocatorMarker = u8*;

  MemoryTag memory_tag_{kEngineMemoryTagUntagged};
  usize capacity_{0};
  u8* root_{nullptr};
  PlatformStackAllocatorMarker marker_{nullptr};
};
}  // namespace memory
}  // namespace comet

#endif  // COMET_RUNTIME_MEMORY_ALLOCATOR_PLATFORM_ALLOCATOR_H_
