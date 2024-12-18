// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_ENTITY_ENTITY_COMPONENT_H_
#define COMET_COMET_ENTITY_ENTITY_COMPONENT_H_

#include "comet/core/essentials.h"
#include "comet/core/type/array.h"
#include "comet/entity/component.h"
#include "comet/entity/entity_id.h"
#include "comet/entity/entity_manager.h"
#include "comet/entity/entity_memory_manager.h"

namespace comet {
namespace entity {
class EntityComponentGenerator {
 public:
  static EntityComponentGenerator Generate(
      EntityId entity_id = kInvalidEntityId);

  EntityComponentGenerator() = delete;
  EntityComponentGenerator(EntityId entity_id);
  EntityComponentGenerator(const EntityComponentGenerator&) = delete;
  EntityComponentGenerator(EntityComponentGenerator&&) = delete;
  EntityComponentGenerator& operator=(const EntityComponentGenerator&) = delete;
  EntityComponentGenerator& operator=(EntityComponentGenerator&&) = delete;
  ~EntityComponentGenerator() = default;

  EntityComponentGenerator& Reserve(usize size);
  EntityComponentGenerator& AddParent(EntityId parent_id);

  template <typename ComponentType>
  EntityComponentGenerator& AddComponent(const ComponentType& component) {
    const auto& component_type_descr{
        ComponentTypeDescrGetter<ComponentType>::Get()};

    ComponentDescr component_descr{};
    component_descr.type_descr = component_type_descr;
    component_descr.data = reinterpret_cast<const u8*>(&component);

    component_descrs_.PushBack(component_descr);
    return *this;
  }

  void Reset();
  EntityId Submit();

 private:
  EntityComponentGenerator& Add(EntityId component_type_id);

  EntityId entity_id_{kInvalidEntityId};
  Array<ComponentDescr> component_descrs_{
      &EntityMemoryManager::Get().GetComponentDescrAllocator()};
};

class EntityComponentDestroyer {
 public:
  static EntityComponentDestroyer Generate(EntityId entity_id);

  EntityComponentDestroyer() = delete;
  EntityComponentDestroyer(EntityId entity_id);
  EntityComponentDestroyer(const EntityComponentDestroyer&) = delete;
  EntityComponentDestroyer(EntityComponentDestroyer&&) = delete;
  EntityComponentDestroyer& operator=(const EntityComponentDestroyer&) = delete;
  EntityComponentDestroyer& operator=(EntityComponentDestroyer&&) = delete;
  ~EntityComponentDestroyer() = default;

  EntityComponentDestroyer& Reserve(usize size);
  EntityComponentDestroyer& RemoveParent(EntityId parent_id);

  template <typename ComponentType>
  EntityComponentDestroyer& RemoveComponent() {
    component_ids_.PushBack(ComponentTypeDescrGetter<ComponentType>::Get().id);
    return *this;
  }

  void Reset();
  void Submit();

 private:
  EntityComponentDestroyer& Remove(EntityId component_type_id);

  EntityId entity_id_{kInvalidEntityId};
  Array<EntityId> component_ids_{
      &EntityMemoryManager::Get().GetEntityIdAllocator()};
};
}  // namespace entity
}  // namespace comet
#endif  // COMET_COMET_ENTITY_ENTITY_COMPONENT_H_
