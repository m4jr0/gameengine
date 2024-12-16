// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "entity_component.h"

namespace comet {
namespace entity {
EntityComponentGenerator EntityComponentGenerator::Generate(
    EntityId entity_id) {
  return EntityComponentGenerator{entity_id};
}

EntityComponentGenerator::EntityComponentGenerator(EntityId entity_id)
    : entity_id_{entity_id} {}

EntityComponentGenerator& EntityComponentGenerator::Reserve(usize size) {
  component_descrs_.Reserve(size);
  return *this;
}

EntityComponentGenerator& EntityComponentGenerator::AddParent(
    EntityId parent_id) {
  COMET_ASSERT(EntityManager::Get().IsEntity(parent_id),
               "Trying to add a dead parent entity #", parent_id, "!");
  return Add(Tag(EntityIdTag::Child, parent_id));
}

void EntityComponentGenerator::Reset() { component_descrs_.Clear(); }

EntityId EntityComponentGenerator::Submit() {
  if (entity_id_ == kInvalidEntityId) {
    return EntityManager::Get().Generate(component_descrs_);
  }

  EntityManager::Get().AddComponents(entity_id_, component_descrs_);
  Reset();
  return entity_id_;
}

EntityComponentGenerator& EntityComponentGenerator::Add(
    EntityId component_type_id) {
  ComponentDescr component_descr{};
  component_descr.type_descr.id = component_type_id;
  component_descr.type_descr.size = 0;
  component_descr.data = nullptr;
  component_descrs_.PushBack(component_descr);
  return *this;
}

EntityComponentDestroyer EntityComponentDestroyer::Generate(
    EntityId entity_id) {
  return EntityComponentDestroyer{entity_id};
}

EntityComponentDestroyer::EntityComponentDestroyer(EntityId entity_id)
    : entity_id_{entity_id} {
  COMET_ASSERT(EntityManager::Get().IsEntity(entity_id),
               "Entity does not exist!");
}

EntityComponentDestroyer& EntityComponentDestroyer::Reserve(usize size) {
  component_ids_.Reserve(size);
  return *this;
}

EntityComponentDestroyer& EntityComponentDestroyer::RemoveParent(
    EntityId parent_id) {
  return Remove(Tag(EntityIdTag::Child, parent_id));
}

void EntityComponentDestroyer::Reset() { component_ids_.Clear(); }

void EntityComponentDestroyer::Submit() {
  EntityManager::Get().RemoveComponents(entity_id_, component_ids_);
  Reset();
}

EntityComponentDestroyer& EntityComponentDestroyer::Remove(
    EntityId component_type_id) {
  component_ids_.PushBack(component_type_id);
  return *this;
}
}  // namespace entity
}  // namespace comet