// Copyright 2024 m4jr0. All Rights Reserved.
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
template <template <typename, typename...> class Vector = std::vector>
class EntityComponentGenerator {
 public:
  using VectorItem = ComponentDescr;

  EntityComponentGenerator() = delete;

  EntityComponentGenerator(EntityId entity_id) : entity_id_{entity_id} {
    static_assert(std::is_same<typename Vector<ComponentDescr>::value_type,
                               ComponentDescr>::value,
                  "Expected item type is supposed to be ComponentDescr!");
  }

  EntityComponentGenerator(const EntityComponentGenerator&) = delete;
  EntityComponentGenerator(EntityComponentGenerator&&) = delete;
  EntityComponentGenerator& operator=(const EntityComponentGenerator&) = delete;
  EntityComponentGenerator& operator=(EntityComponentGenerator&&) = delete;
  ~EntityComponentGenerator() = default;

  EntityComponentGenerator& Reserve(uindex size) {
    component_descrs_.reserve(size);
    return *this;
  }

  EntityComponentGenerator& ShrinkToFit() {
    component_descrs_.shrink_to_fit();
    return *this;
  }

  EntityComponentGenerator& AddParent(EntityId parent_id) {
    COMET_ASSERT(EntityManager::Get().IsEntity(parent_id),
                 "Trying to add a dead parent entity #", parent_id, "!");
    return Add(Tag(EntityIdTag::Child, parent_id));
  }

  template <typename ComponentType>
  EntityComponentGenerator& AddComponent(const ComponentType& component) {
    const auto& component_type_descr{
        ComponentTypeDescrGetter<ComponentType>::Get()};

    VectorItem component_descr{};
    component_descr.type_descr = component_type_descr;
    component_descr.data = reinterpret_cast<const u8*>(&component);

    component_descrs_.push_back(component_descr);
    return *this;
  }

  void Reset() { component_descrs_.clear(); }

  EntityId Submit() {
    if (entity_id_ == kInvalidEntityId) {
      return EntityManager::Get().Generate(component_descrs_);
    }

    EntityManager::Get().AddComponents(entity_id_, component_descrs_);
    Reset();
    return entity_id_;
  }

 private:
  EntityComponentGenerator& Add(EntityId component_type_id) {
    ComponentDescr component_descr{};
    component_descr.type_descr.id = component_type_id;
    component_descr.type_descr.size = 0;
    component_descr.data = nullptr;
    component_descrs_.push_back(component_descr);
    return *this;
  }

  EntityId entity_id_{kInvalidEntityId};
  std::vector<VectorItem> component_descrs_{};
};

template <template <typename, typename...> class Vector = std::vector>
auto GetEntityComponentGenerator(EntityId entity_id = kInvalidEntityId) {
  return EntityComponentGenerator<Vector>{entity_id};
}

template <template <typename, typename...> class Vector = std::vector>
class EntityComponentDestroyer {
 public:
  using VectorItem = ComponentDescr;

  EntityComponentDestroyer() = delete;

  EntityComponentDestroyer(EntityId entity_id) : entity_id_{entity_id} {
    COMET_ASSERT(EntityManager::Get().IsEntity(entity_id),
                 "Entity does not exist!");
  }

  EntityComponentDestroyer(const EntityComponentDestroyer&) = delete;
  EntityComponentDestroyer(EntityComponentDestroyer&&) = delete;
  EntityComponentDestroyer& operator=(const EntityComponentDestroyer&) = delete;
  EntityComponentDestroyer& operator=(EntityComponentDestroyer&&) = delete;
  ~EntityComponentDestroyer() = default;

  EntityComponentDestroyer& Reserve(uindex size) {
    component_ids_.reserve(size);
    return *this;
  }

  EntityComponentDestroyer& ShrinkToFit() {
    component_ids_.shrink_to_fit();
    return *this;
  }

  EntityComponentDestroyer& RemoveParent(EntityId parent_id) {
    return Remove(Tag(EntityIdTag::Child, parent_id));
  }

  template <typename ComponentType>
  EntityComponentDestroyer& RemoveComponent() {
    component_ids_.push_back(ComponentTypeDescrGetter<ComponentType>::Get().id);
    return *this;
  }

  void Reset() { component_ids_.clear(); }

  void Submit() {
    EntityManager::Get().RemoveComponents(entity_id_, component_ids_);
    Reset();
  }

 private:
  EntityComponentDestroyer& Remove(EntityId component_type_id) {
    component_ids_.push_back(component_type_id);
    return *this;
  }

  EntityId entity_id_{kInvalidEntityId};
  std::vector<EntityId> component_ids_{};
};

template <template <typename, typename...> class Vector = std::vector>
auto GetEntityComponentDestroyer(EntityId entity_id) {
  return EntityComponentDestroyer<Vector>{entity_id};
}
}  // namespace entity
}  // namespace comet
#endif  // COMET_COMET_ENTITY_ENTITY_COMPONENT_H_
