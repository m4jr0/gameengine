// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "entity_component.h"

namespace comet {
namespace entity {
EntityComponentGenerator EntityComponentGenerator::Get(
    EntityManager* entity_manager, EntityId entity_id) {
  return EntityComponentGenerator{entity_manager, entity_id};
}

EntityComponentGenerator& EntityComponentGenerator::Reserve(uindex size) {
  component_descrs_.reserve(size);
  return *this;
}

EntityComponentGenerator& EntityComponentGenerator::ShrinkToFit() {
  component_descrs_.shrink_to_fit();
  return *this;
}

EntityComponentGenerator& EntityComponentGenerator::AddParent(
    EntityId parent_id) {
  COMET_ASSERT(entity_manager_->IsEntity(parent_id),
               "Trying to add a dead parent entity #", parent_id, "!");
  return Add(Tag(EntityIdTag::Child, parent_id));
}

void EntityComponentGenerator::Reset() { component_descrs_.clear(); }

EntityId EntityComponentGenerator::Submit() {
  if (entity_id_ == kInvalidEntityId) {
    return entity_manager_->Generate(component_descrs_);
  }

  entity_manager_->AddComponents(entity_id_, component_descrs_);
  Reset();
  return entity_id_;
}

EntityComponentGenerator::EntityComponentGenerator(
    EntityManager* entity_manager, EntityId entity_id)
    : entity_manager_{entity_manager}, entity_id_{entity_id} {
  COMET_ASSERT(entity_manager_ != nullptr, "Entity manager is null!");
}

EntityComponentGenerator& EntityComponentGenerator::Add(
    EntityId component_type_id) {
  ComponentDescr component_descr{};
  component_descr.type_descr.id = component_type_id;
  component_descr.type_descr.size = 0;
  component_descr.data = nullptr;
  component_descrs_.push_back(component_descr);
  return *this;
}

EntityComponentDestroyer EntityComponentDestroyer::Get(
    EntityManager* entity_manager, EntityId entity_id) {
  return EntityComponentDestroyer{entity_manager, entity_id};
}

EntityComponentDestroyer& EntityComponentDestroyer::Reserve(uindex size) {
  component_ids_.reserve(size);
  return *this;
}

EntityComponentDestroyer& EntityComponentDestroyer::ShrinkToFit() {
  component_ids_.shrink_to_fit();
  return *this;
}

EntityComponentDestroyer& EntityComponentDestroyer::RemoveParent(
    EntityId parent_id) {
  return Remove(Tag(EntityIdTag::Child, parent_id));
}

void EntityComponentDestroyer::Reset() { component_ids_.clear(); }

void EntityComponentDestroyer::Submit() {
  entity_manager_->RemoveComponents(entity_id_, component_ids_);
  Reset();
}

EntityComponentDestroyer::EntityComponentDestroyer(
    EntityManager* entity_manager, EntityId entity_id)
    : entity_manager_{entity_manager}, entity_id_{entity_id} {
  COMET_ASSERT(entity_manager_ != nullptr, "Entity manager is null!");
  COMET_ASSERT(entity_manager_->IsEntity(entity_id), "Entity does not exist!");
}

EntityComponentDestroyer& EntityComponentDestroyer::Remove(
    EntityId component_type_id) {
  component_ids_.push_back(component_type_id);
  return *this;
}
}  // namespace entity
}  // namespace comet