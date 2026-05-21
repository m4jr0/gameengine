// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_ENTITY_ENTITY_MEMORY_CONTEXT_H_
#define COMET_COMET_ENTITY_ENTITY_MEMORY_CONTEXT_H_

#include "comet/core/essentials.h"
#include "comet/core/memory/allocator/allocator.h"
#include "comet/core/memory/allocator/free_list_allocator.h"
#include "comet/entity/type/archetype.h"

namespace comet {
namespace entity {
class EntityMemoryContext {
 public:
  static EntityMemoryContext& Get();

  EntityMemoryContext();
  EntityMemoryContext(const EntityMemoryContext&) = delete;
  EntityMemoryContext(EntityMemoryContext&&) = delete;
  EntityMemoryContext& operator=(const EntityMemoryContext&) = delete;
  EntityMemoryContext& operator=(EntityMemoryContext&&) = delete;
  ~EntityMemoryContext();

  void Initialize();
  void Destroy();

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

  bool IsInitialized() const noexcept;

 private:
  bool is_initialized_{false};

  static inline constexpr usize kSmallBlockSize_{16};
  static inline constexpr usize kMediumBlockSize_{64};
  static inline constexpr usize kBigBlockSize_{sizeof(Archetype)};

  static inline constexpr usize kSmallComponentBlockSize_{128};
  static inline constexpr usize kMediumComponentBlockSize_{1024};
  static inline constexpr usize kBigComponentBlockSize_{16384};

  static inline constexpr usize kDefaultBlockCount_{512};

  memory::FiberFreeListAllocator small_block_allocator_{};
  memory::FiberFreeListAllocator medium_block_allocator_{};
  memory::FiberFreeListAllocator big_block_allocator_{};

  memory::FiberFreeListAllocator small_component_block_allocator_{};
  memory::FiberFreeListAllocator medium_component_block_allocator_{};
  memory::FiberFreeListAllocator big_component_block_allocator_{};
};
}  // namespace entity
}  // namespace comet

#endif  // COMET_COMET_ENTITY_ENTITY_MEMORY_CONTEXT_H_