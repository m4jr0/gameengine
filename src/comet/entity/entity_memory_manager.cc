// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "entity_memory_manager.h"
////////////////////////////////////////////////////////////////////////////////

namespace comet {
namespace entity {
EntityMemoryManager& EntityMemoryManager::Get() {
  static EntityMemoryManager singleton{};
  return singleton;
}

EntityMemoryManager::EntityMemoryManager()
    : small_block_allocator_{kSmallAllocatorAllocationUnit_, 512,
                             memory::kEngineMemoryTagEntity},
      medium_block_allocator_{kMediumAllocatorAllocationUnit_, 512,
                              memory::kEngineMemoryTagEntity},
      big_block_allocator_{kBigAllocatorAllocationUnit_, 512,
                           memory::kEngineMemoryTagEntity},
      cmp_small_block_allocator_{kCmpSmallAllocatorAllocationUnit_, 512,
                                 memory::kEngineMemoryTagEntity},
      cmp_medium_block_allocator_{kCmpMediumAllocatorAllocationUnit_, 512,
                                  memory::kEngineMemoryTagEntity},
      cmp_big_block_allocator_{kCmpBigAllocatorAllocationUnit_, 512,
                               memory::kEngineMemoryTagEntity} {}

void EntityMemoryManager::Initialize() {
  Manager::Initialize();
  small_block_allocator_.Initialize();
  medium_block_allocator_.Initialize();
  big_block_allocator_.Initialize();

  cmp_small_block_allocator_.Initialize();
  cmp_medium_block_allocator_.Initialize();
  cmp_big_block_allocator_.Initialize();
}

void EntityMemoryManager::Shutdown() {
  small_block_allocator_.Destroy();
  medium_block_allocator_.Destroy();
  big_block_allocator_.Destroy();

  cmp_small_block_allocator_.Destroy();
  cmp_medium_block_allocator_.Destroy();
  cmp_big_block_allocator_.Destroy();

  Manager::Shutdown();
}

memory::Allocator& EntityMemoryManager::GetRecordAllocator() noexcept {
  return small_block_allocator_;
}

memory::Allocator& EntityMemoryManager::GetComponentArrayAllocator() noexcept {
  return small_block_allocator_;
}

memory::Allocator& EntityMemoryManager::GetEntityIdAllocator() noexcept {
  return small_block_allocator_;
}

memory::Allocator& EntityMemoryManager::GetComponentDescrAllocator() noexcept {
  return medium_block_allocator_;
}

memory::Allocator& EntityMemoryManager::GetRecordsAllocator() noexcept {
  return medium_block_allocator_;
}

memory::Allocator&
EntityMemoryManager::GetRegisteredComponentTypeMapAllocator() noexcept {
  return medium_block_allocator_;
}

memory::Allocator& EntityMemoryManager::GetEntityTypeAllocator() noexcept {
  return medium_block_allocator_;
}

memory::Allocator& EntityMemoryManager::GetArchetypeMapAllocator() noexcept {
  return medium_block_allocator_;
}

memory::Allocator&
EntityMemoryManager::GetArchetypePointerAllocator() noexcept {
  return big_block_allocator_;
}

memory::Allocator& EntityMemoryManager::GetArchetypeAllocator() noexcept {
  return medium_block_allocator_;
}

memory::Allocator& EntityMemoryManager::GetComponentArrayElementsAllocator(
    usize size) noexcept {
  if (size <= kCmpSmallAllocatorAllocationUnit_) {
    return cmp_small_block_allocator_;
  }

  if (size <= kCmpMediumAllocatorAllocationUnit_) {
    return cmp_medium_block_allocator_;
  }

  return cmp_big_block_allocator_;
}
}  // namespace entity
}  // namespace comet
