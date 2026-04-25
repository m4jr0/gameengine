// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/entity/entity_manager.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/frame/frame_event.h"
#include "comet/core/manager.h"
#include "comet/core/type/array.h"
#include "comet/entity/entity_memory_context.h"
#include "comet/entity/entity_type.h"
#include "comet/entity/factory/entity_factory_manager.h"
#include "comet/entity/type/archetype.h"
#include "comet/entity/type/entity_id.h"
#include "comet/entity/type/pending_entity.h"
#include "comet/event/event.h"

namespace comet {
namespace entity {
EntityManager& EntityManager::Get() {
  static EntityManager singleton{};
  return singleton;
}

void EntityManager::OnInitialize() {
  auto& memory_context{EntityMemoryContext::Get()};
  memory_context.Initialize();

  records_ = Records{&memory_context.GetRecordAllocator()};

  known_component_types_ = RegisteredComponentTypeMap{
      &memory_context.GetRegisteredComponentTypeMapAllocator()};

  // TODO(m4jr0): Use configuration?
  // Tags: configuration entity memory
  archetypes_ =
      Array<ArchetypePtr>{&memory_context.GetArchetypePointerAllocator()};

  root_archetype_ = GetOrGenerateArchetype(EntityType{});

  for (auto& allocator : pending_allocators_) {
    allocator.Initialize();
  }

  for (usize i{0}; i < 2; ++i) {
    pending_entities_[i] = Map<EntityId, internal::PendingEntity>::WithCapacity(
        &pending_allocators_[i], kPendingEntityInitialCount_);
  }

  EntityFactoryManager::Get().Initialize();
  RegisterEvents();
}

void EntityManager::OnShutdown() {
  EntityFactoryManager::Get().Shutdown();

  UnregisterEvents();

  auto& memory_context{EntityMemoryContext::Get()};

  for (auto& archetype : archetypes_) {
    for (auto& cmp_array : archetype->components) {
      if (cmp_array.elements != nullptr) {
        memory_context.GetComponentArrayElementsAllocator(cmp_array.size)
            .Deallocate(cmp_array.elements);
        cmp_array = {nullptr, 0};
      }
    }
  }

  archetypes_.Release();
  root_archetype_ = nullptr;
  entity_id_handler_.Shutdown();
  records_.Release();
  known_component_types_.Release();

  for (auto& map : pending_entities_) {
    map.Release();
  }

  for (auto& allocator : pending_allocators_) {
    allocator.Destroy();
  }

  memory_context.Destroy();
}

void EntityManager::OnEvent(const event::Event& event) {
  const auto& event_type{event.GetType()};

  if (event_type == frame::EndFrameEvent::kStaticType_) {
    Flush();
  }
}

void EntityManager::RegisterEvents() {
  const auto on_event{[this](const event::Event& event) { OnEvent(event); }};

  end_frame_listener_id_ = event::EventManager::Get().Register(
      on_event, frame::EndFrameEvent::kStaticType_);
  COMET_ASSERT(end_frame_listener_id_ != event::kInvalidEventListenerId,
               "EntityManager::RegisterEvents",
               "end frame listener registration failed");
}

void EntityManager::UnregisterEvents() {
  auto& event_manager{event::EventManager::Get()};

  if (end_frame_listener_id_ != event::kInvalidEventListenerId) {
    event_manager.Unregister(end_frame_listener_id_);
    end_frame_listener_id_ = event::kInvalidEventListenerId;
  }
}
}  // namespace entity
}  // namespace comet