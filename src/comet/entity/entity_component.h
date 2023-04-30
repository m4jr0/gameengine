// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_ENTITY_ENTITY_COMPONENT_H_
#define COMET_COMET_ENTITY_ENTITY_COMPONENT_H_

#include "comet_precompile.h"

#include "comet/entity/component.h"
#include "comet/entity/entity_id.h"
#include "comet/entity/entity_manager.h"

namespace comet {
namespace entity {
class EntityComponentGenerator {
 public:
  EntityComponentGenerator() = delete;
  EntityComponentGenerator(const EntityComponentGenerator&) = delete;
  EntityComponentGenerator(EntityComponentGenerator&&) = delete;
  EntityComponentGenerator& operator=(const EntityComponentGenerator&) = delete;
  EntityComponentGenerator& operator=(EntityComponentGenerator&&) = delete;
  ~EntityComponentGenerator() = default;

  static EntityComponentGenerator Get(EntityManager* entity_manager,
                                      EntityId entity_id = kInvalidEntityId);
  EntityComponentGenerator& Reserve(uindex size);
  EntityComponentGenerator& ShrinkToFit();
  EntityComponentGenerator& AddParent(EntityId parent_id);

  template <typename ComponentType>
  EntityComponentGenerator& AddComponent(const ComponentType& component) {
    const auto& component_type_descr{
        ComponentTypeDescrGetter<ComponentType>::Get()};

    ComponentDescr component_descr{};
    component_descr.type_descr = component_type_descr;
    component_descr.data = reinterpret_cast<const u8*>(&component);

    component_descrs_.push_back(component_descr);
    return *this;
  }

  void Reset();
  EntityId Submit();

 private:
  EntityComponentGenerator(EntityManager* entity_manager,
                           EntityId entity_id = kInvalidEntityId);
  EntityComponentGenerator& Add(EntityId component_type_id);

  EntityManager* entity_manager_{nullptr};
  EntityId entity_id_{kInvalidEntityId};
  std::vector<ComponentDescr> component_descrs_{};
};

class EntityComponentDestroyer {
 public:
  EntityComponentDestroyer() = delete;
  EntityComponentDestroyer(const EntityComponentDestroyer&) = delete;
  EntityComponentDestroyer(EntityComponentDestroyer&&) = delete;
  EntityComponentDestroyer& operator=(const EntityComponentDestroyer&) = delete;
  EntityComponentDestroyer& operator=(EntityComponentDestroyer&&) = delete;
  ~EntityComponentDestroyer() = default;

  static EntityComponentDestroyer Get(EntityManager* entity_manager,
                                      EntityId entity_id);
  EntityComponentDestroyer& Reserve(uindex size);
  EntityComponentDestroyer& ShrinkToFit();
  EntityComponentDestroyer& RemoveParent(EntityId parent_id);

  template <typename ComponentType>
  EntityComponentDestroyer& RemoveComponent() {
    component_ids_.push_back(ComponentTypeDescrGetter<ComponentType>::Get().id);
    return *this;
  }

  void Reset();
  void Submit();

 private:
  EntityComponentDestroyer(EntityManager* entity_manager, EntityId entity_id);
  EntityComponentDestroyer& Remove(EntityId component_type_id);

  EntityManager* entity_manager_{nullptr};
  EntityId entity_id_{kInvalidEntityId};
  std::vector<EntityId> component_ids_{};
};
}  // namespace entity
}  // namespace comet
#endif  // COMET_COMET_ENTITY_ENTITY_COMPONENT_H_
