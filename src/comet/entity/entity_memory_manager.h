// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_ENTITY_ENTITY_MEMORY_MANAGER_H_
#define COMET_COMET_ENTITY_ENTITY_MEMORY_MANAGER_H_

#include "comet/core/essentials.h"
#include "comet/core/manager.h"
#include "comet/core/memory/allocator/allocator.h"
#include "comet/core/memory/allocator/free_list_allocator.h"
#include "comet/entity/archetype.h"

namespace comet {
namespace entity {
class EntityMemoryManager : public Manager {
 public:
  static EntityMemoryManager& Get();

  EntityMemoryManager();
  EntityMemoryManager(const EntityMemoryManager&) = delete;
  EntityMemoryManager(EntityMemoryManager&&) = delete;
  EntityMemoryManager& operator=(const EntityMemoryManager&) = delete;
  EntityMemoryManager& operator=(EntityMemoryManager&&) = delete;
  virtual ~EntityMemoryManager() = default;

  void Initialize() override;
  void Shutdown() override;

  memory::Allocator& GetRecordAllocator() noexcept;
  memory::Allocator& GetComponentArrayAllocator() noexcept;
  memory::Allocator& GetEntityIdAllocator() noexcept;
  memory::Allocator& GetComponentDescrAllocator() noexcept;
  memory::Allocator& GetRecordsAllocator() noexcept;
  memory::Allocator& GetRegisteredComponentTypeMapAllocator() noexcept;
  memory::Allocator& GetEntityTypeAllocator() noexcept;
  memory::Allocator& GetArchetypeMapAllocator() noexcept;
  memory::Allocator& GetArchetypePointerAllocator() noexcept;
  memory::Allocator& GetArchetypeAllocator() noexcept;

  memory::Allocator& GetComponentArrayElementsAllocator(usize size) noexcept;

 private:
  static inline constexpr usize kSmallAllocatorAllocationUnit_{16};
  static inline constexpr usize kMediumAllocatorAllocationUnit_{64};
  static inline constexpr usize kBigAllocatorAllocationUnit_{sizeof(Archetype)};

  static inline constexpr usize kCmpSmallAllocatorAllocationUnit_{128};
  static inline constexpr usize kCmpMediumAllocatorAllocationUnit_{1024};
  static inline constexpr usize kCmpBigAllocatorAllocationUnit_{16384};

  memory::FiberFreeListAllocator small_block_allocator_{};
  memory::FiberFreeListAllocator medium_block_allocator_{};
  memory::FiberFreeListAllocator big_block_allocator_{};

  memory::FiberFreeListAllocator cmp_small_block_allocator_{};
  memory::FiberFreeListAllocator cmp_medium_block_allocator_{};
  memory::FiberFreeListAllocator cmp_big_block_allocator_{};
};
}  // namespace entity
}  // namespace comet

#endif  // COMET_COMET_ENTITY_ENTITY_MEMORY_MANAGER_H_
