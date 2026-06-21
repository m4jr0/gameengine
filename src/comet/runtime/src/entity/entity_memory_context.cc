// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_runtime_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/runtime/entity/entity_memory_context.h"
////////////////////////////////////////////////////////////////////////////////

namespace comet {
namespace entity {
EntityMemoryContext& EntityMemoryContext::Get() {
  static EntityMemoryContext singleton{};
  return singleton;
}

EntityMemoryContext::EntityMemoryContext()
    : small_block_allocator_{kSmallBlockSize_, kDefaultBlockCount_,
                             kEngineMemoryTagEntity},
      medium_block_allocator_{kMediumBlockSize_, kDefaultBlockCount_,
                              kEngineMemoryTagEntity},
      big_block_allocator_{kBigBlockSize_, kDefaultBlockCount_,
                           kEngineMemoryTagEntity},
      small_component_block_allocator_{kSmallComponentBlockSize_,
                                       kDefaultBlockCount_,
                                       kEngineMemoryTagEntity},
      medium_component_block_allocator_{kMediumComponentBlockSize_,
                                        kDefaultBlockCount_,
                                        kEngineMemoryTagEntity},
      big_component_block_allocator_{kBigComponentBlockSize_,
                                     kDefaultBlockCount_,
                                     kEngineMemoryTagEntity} {}

EntityMemoryContext::~EntityMemoryContext() {
  COMET_ASSERT(!is_initialized_, "EntityMemoryContext::~EntityMemoryContext",
               "entity memory context is still initialized");
}

void EntityMemoryContext::Initialize() {
  COMET_ASSERT(!is_initialized_, "EntityMemoryContext::Initialize",
               "entity memory context is already initialized");

  small_block_allocator_.Initialize();
  medium_block_allocator_.Initialize();
  big_block_allocator_.Initialize();

  small_component_block_allocator_.Initialize();
  medium_component_block_allocator_.Initialize();
  big_component_block_allocator_.Initialize();

  is_initialized_ = true;
}

void EntityMemoryContext::Destroy() {
  COMET_ASSERT(is_initialized_, "EntityMemoryContext::Destroy",
               "entity memory context is not initialized");

  small_block_allocator_.Destroy();
  medium_block_allocator_.Destroy();
  big_block_allocator_.Destroy();

  small_component_block_allocator_.Destroy();
  medium_component_block_allocator_.Destroy();
  big_component_block_allocator_.Destroy();

  is_initialized_ = false;
}

bool EntityMemoryContext::IsInitialized() const noexcept {
  return is_initialized_;
}

memory::Allocator& EntityMemoryContext::GetRecordAllocator() noexcept {
  return small_block_allocator_;
}

memory::Allocator& EntityMemoryContext::GetComponentArrayAllocator() noexcept {
  return small_block_allocator_;
}

memory::Allocator& EntityMemoryContext::GetEntityIdAllocator() noexcept {
  return small_block_allocator_;
}

memory::Allocator& EntityMemoryContext::GetComponentDescrAllocator() noexcept {
  return medium_block_allocator_;
}

memory::Allocator& EntityMemoryContext::GetRecordsAllocator() noexcept {
  return medium_block_allocator_;
}

memory::Allocator&
EntityMemoryContext::GetRegisteredComponentTypeMapAllocator() noexcept {
  return medium_block_allocator_;
}

memory::Allocator& EntityMemoryContext::GetEntityTypeAllocator() noexcept {
  return medium_block_allocator_;
}

memory::Allocator& EntityMemoryContext::GetArchetypeMapAllocator() noexcept {
  return medium_block_allocator_;
}

memory::Allocator&
EntityMemoryContext::GetArchetypePointerAllocator() noexcept {
  return big_block_allocator_;
}

memory::Allocator& EntityMemoryContext::GetArchetypeAllocator() noexcept {
  return medium_block_allocator_;
}

memory::Allocator& EntityMemoryContext::GetComponentArrayElementsAllocator(
    usize size) noexcept {
  if (size <= kSmallComponentBlockSize_) {
    return small_component_block_allocator_;
  }

  if (size <= kMediumComponentBlockSize_) {
    return medium_component_block_allocator_;
  }

  return big_component_block_allocator_;
}
}  // namespace entity
}  // namespace comet