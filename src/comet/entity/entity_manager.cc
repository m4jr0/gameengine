// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "entity_manager.h"

namespace comet {
namespace entity {
EntityManager& EntityManager::Get() {
  static EntityManager singleton{};
  return singleton;
}

void EntityManager::Initialize() {
  Manager::Initialize();
  root_archetype_ = GetArchetype(EntityType{});
}

void EntityManager::Shutdown() {
  for (auto* archetype : archetypes_) {
    for (auto& cmp_array : archetype->components) {
      if (cmp_array.size == 0) {
        continue;
      }

      delete[] cmp_array.elements;
      cmp_array.elements = nullptr;
    }

    delete archetype;
    archetype = nullptr;
  }

  archetypes_.clear();
  component_id_handler_.Shutdown();
  root_archetype_ = nullptr;
  entity_id_handler_.Shutdown();
  records_.clear();
  Manager::Shutdown();
}

EntityId EntityManager::Generate() {
  auto entity_id{entity_id_handler_.Generate()};

  // Special case: adding the entity to the root archetype (which does not
  // contain any components).
  root_archetype_->entity_ids.push_back(entity_id);

  records_.emplace(std::make_pair(
      entity_id,
      Record{root_archetype_, root_archetype_->entity_ids.size() - 1}));
  return entity_id;
}

EntityId EntityManager::Generate(
    const std::vector<ComponentDescr>& component_descrs) {
  EntityId entity_id{entity_id_handler_.Generate()};
  RegisterComponentTypes(component_descrs);
  auto* archetype{GetArchetype(GenerateEntityType(component_descrs))};
  ResizeArchetype(archetype, 1);

  auto entity_index{archetype->entity_ids.size() - 1};
  archetype->entity_ids[entity_index] = entity_id;

  records_.emplace(std::make_pair(entity_id, Record{archetype, entity_index}));

  for (const auto& component_descr : component_descrs) {
    CopyComponent(entity_id, archetype, entity_index, component_descr);
  }

  return entity_id;
}

bool EntityManager::IsEntity(const EntityId& entity_id) const {
  return entity_id_handler_.IsAlive(entity_id);
}

void EntityManager::Destroy(EntityId entity_id) {
  COMET_ASSERT(IsEntity(entity_id), "Trying to destroy dead entity #",
               entity_id, "!");

  const auto& record{records_[entity_id]};
  PreRemoveEntityFromArchetype(record.row, record.archetype);
  ResizeArchetype(record.archetype, -1);
  records_.erase(entity_id);
  entity_id_handler_.Destroy(entity_id);
}

bool EntityManager::HasComponent(EntityId entity_id,
                                 EntityId component_id) const {
  const auto& record{records_.at(entity_id)};
  const auto* archetype{record.archetype};
  const auto& archetype_map{
      registered_component_types_.at(component_id).archetype_map};
  return archetype_map.find(archetype->id) != archetype_map.cend();
}

void EntityManager::AddComponents(
    EntityId entity_id, const std::vector<ComponentDescr>& component_descrs) {
  {
    // Step 1: apply safety checks + handle new archetype.
    COMET_ASSERT(IsEntity(entity_id),
                 "Trying to add component to dead entity #", entity_id, "!");
    RegisterComponentTypes(component_descrs);

    auto added_entity_type{GenerateEntityType(component_descrs)};
    auto& entity_record{records_[entity_id]};
    auto* old_archetype{entity_record.archetype};
    const auto& old_entity_type{old_archetype->entity_type};
    auto* new_archetype{
        GetArchetype(AddToEntityType(old_entity_type, added_entity_type))};

    // Step 2: increase new archetype's size.
    ResizeArchetype(new_archetype, 1);

    // Step 3: copy entity's components into new archetype.
    const auto& new_entity_type{new_archetype->entity_type};
    const uindex new_entity_index{new_archetype->entity_ids.size() - 1};
    const auto old_entity_index{entity_record.row};
    uindex old_cmp_cursor{0};
    uindex new_cmp_cursor{0};

    for (uindex i{0}; i < new_entity_type.size(); ++i) {
      auto& new_cmp_array{new_archetype->components[i]};

      if (new_cmp_cursor < component_descrs.size() &&
          added_entity_type[new_cmp_cursor] == new_entity_type[i]) {
        // Check new components if assertion is enabled.
        COMET_ASSERT(!DoesEntityTypeContain(old_entity_type,
                                            added_entity_type[new_cmp_cursor]),
                     "Entity #", entity_id, " already contains component ",
                     COMET_STRING_ID_LABEL(added_entity_type[new_cmp_cursor]),
                     "!");

        // New components are added later.
        ++new_cmp_cursor;  // Added component IDs are sorted, so we can optimize
                           // the loop.
      } else {
        // Copy existing components.
        const auto component_type_id{new_entity_type[i]};
        const auto cmp_size{
            registered_component_types_[component_type_id].type_descr.size};

        if (cmp_size > 0) {
          const auto new_cmp_offset{
              static_cast<sptrdiff>(cmp_size * new_entity_index)};
          const auto old_cmp_offset{
              static_cast<sptrdiff>(cmp_size * old_entity_index)};

          std::memcpy(
              new_cmp_array.first + new_cmp_offset,
              old_archetype->components[old_cmp_cursor].first + old_cmp_offset,
              cmp_size);
        }

        ++old_cmp_cursor;
      }
    }

    // Copy new components.
    for (const auto& component_descr : component_descrs) {
      CopyComponent(entity_id, new_archetype, new_entity_index,
                    component_descr);
    }

    // Step 4: copy last entity inplace of current entity, if necessary.
    PreRemoveEntityFromArchetype(entity_record.row, old_archetype);

    // Step 5: decrease old archetype's size.
    ResizeArchetype(old_archetype, -1);

    new_archetype->entity_ids[new_entity_index] = entity_id;
    entity_record.archetype = new_archetype;
    entity_record.row = new_entity_index;
  }
}

void EntityManager::RemoveComponents(
    EntityId entity_id, const std::vector<EntityId>& component_ids) {
  auto removed_entity_type{GenerateEntityType(component_ids)};
  auto removed_component_count{removed_entity_type.size()};
  COMET_ASSERT(removed_component_count != 0,
               "Trying to remove 0 component for entity #", entity_id, "!");
  COMET_ASSERT(IsEntity(entity_id),
               "Trying to remove component from dead entity #", entity_id, "!");

  auto& entity_record{records_[entity_id]};
  auto* old_archetype{entity_record.archetype};
  const auto& old_entity_type{old_archetype->entity_type};
  const auto old_component_count{old_entity_type.size()};

  for (uindex i{0}; i < removed_component_count; ++i) {
    COMET_ASSERT(DoesEntityTypeContain(old_entity_type, removed_entity_type[i]),
                 "Entity #", entity_id, " does not contain component ",
                 COMET_STRING_ID_LABEL(removed_entity_type[i]), "!");
  }

  COMET_ASSERT(removed_component_count <= old_component_count,
               "Component count to remove for entity #", entity_id,
               ": is too high (", removed_component_count, " > ",
               old_component_count, ")!");

  auto* new_archetype{
      GetArchetype(RemoveFromEntityType(old_entity_type, removed_entity_type))};

  // Step 2: increase new archetype's size.
  ResizeArchetype(new_archetype, 1);

  // Step 3: copy entity's components into new archetype.
  const uindex new_entity_index{new_archetype->entity_ids.size() - 1};
  const auto old_entity_index{entity_record.row};

  for (uindex i{0}; i < new_archetype->entity_type.size(); ++i) {
    const auto cmp_size{
        registered_component_types_[new_archetype->entity_type[i]]
            .type_descr.size};

    if (cmp_size == 0) {
      continue;
    }

    auto& new_cmp_array{new_archetype->components[i]};
    const auto new_cmp_offset{
        static_cast<sptrdiff>(cmp_size * new_entity_index)};
    const auto old_cmp_offset{
        static_cast<sptrdiff>(cmp_size * old_entity_index)};

    // Copy non-removed components.
    std::memcpy(new_cmp_array.first + new_cmp_offset,
                old_archetype->components[i].first + old_cmp_offset, cmp_size);
  }

  // Step 4: copy last entity inplace of current entity, if necessary.
  PreRemoveEntityFromArchetype(entity_record.row, old_archetype);

  // Step 5: decrease old archetype's size.
  ResizeArchetype(old_archetype, -1);

  new_archetype->entity_ids[new_entity_index] = entity_id;
  entity_record.archetype = new_archetype;
  entity_record.row = new_entity_index;
}

void EntityManager::RegisterComponentType(
    const ComponentDescr& component_descr) {
  auto it{registered_component_types_.find(component_descr.type_descr.id)};

  if (it != registered_component_types_.cend()) {
    ++it->second.use_count;
    return;
  }

  RegisteredComponentType registered{};
  registered.type_descr = component_descr.type_descr;
  registered.use_count = 1;

  const auto insert_pair{registered_component_types_.emplace(
      component_descr.type_descr.id, registered)};
  COMET_ASSERT(insert_pair.second,
               "Could not insert registered component type: ",
               registered.type_descr.id, "!");
}

void EntityManager::RegisterComponentTypes(
    const std::vector<ComponentDescr>& component_descrs) {
  for (const auto& component_descr : component_descrs) {
    RegisterComponentType(component_descr);
  }
}

void EntityManager::UnregisterComponentType(EntityId component_type_id) {
  auto it{registered_component_types_.find(component_type_id)};

  if (it == registered_component_types_.cend()) {
    return;
  }

  if (--it->second.use_count == 0) {
    registered_component_types_.erase(it);
  }
}

void EntityManager::ResizeArchetype(Archetype* archetype, s16 delta) {
  if (delta == 0) {
    return;
  }

  for (uindex i{0}; i < archetype->entity_type.size(); ++i) {
    const auto cmp_type_id{archetype->entity_type[i]};
    const auto& old_cmp_array{archetype->components[i]};
    auto* old_cmp_elements{old_cmp_array.elements};
    auto old_cmp_size{old_cmp_array.size};
    const auto& cmp_type_descr{
        registered_component_types_[cmp_type_id].type_descr};
    const auto cmp_size{cmp_type_descr.size};
    const auto cmp_alignment{cmp_type_descr.alignment};

    uindex new_size{0};
    auto tmp{static_cast<s32>(old_cmp_size) +
             static_cast<s32>(cmp_size) * delta};

    if (tmp > 0) {
      new_size = tmp;
    }

    // If new_size < copy_size, the last existing components will be
    // truncated.
    const auto copy_size{std::min(old_cmp_size, new_size)};
    archetype->components[i] = {nullptr, nullptr, new_size};

    if (new_size > 0) {
      archetype->components[i].first = AllocAligned(
          &archetype->components[i].elements, new_size, cmp_alignment);
    }

    if (copy_size != 0) {
      std::memcpy(archetype->components[i].elements, old_cmp_elements,
                  copy_size);
    }

    if (old_cmp_size != 0) {
      delete[] old_cmp_elements;
      old_cmp_elements = nullptr;
    }
  }

  archetype->entity_ids.resize(
      std::max(uindex{0}, archetype->entity_ids.size() + delta));
}

void EntityManager::PreRemoveEntityFromArchetype(uindex entity_row,
                                                 Archetype* archetype) {
  const auto last_entity_row{archetype->entity_ids.size() - 1};

  if (last_entity_row == 0) {
    return;
  }

  if (entity_row != last_entity_row) {
    auto& last_entity_record{records_[archetype->entity_ids[last_entity_row]]};

    for (uindex i{0}; i < archetype->entity_type.size(); ++i) {
      auto& cmp_array{archetype->components[i]};
      const auto cmp_type_id{archetype->entity_type[i]};
      const auto cmp_size{
          registered_component_types_[cmp_type_id].type_descr.size};

      if (cmp_array.size == 0 || cmp_size == 0) {
        continue;
      }

      std::memcpy(cmp_array.first + cmp_size * entity_row,
                  cmp_array.first + cmp_size * last_entity_row, cmp_size);
    }

    last_entity_record.row = entity_row;
    archetype->entity_ids[entity_row] = archetype->entity_ids[last_entity_row];
  }
}

bool EntityManager::DoesEntityTypeContain(const EntityType& entity_type,
                                          EntityId component_type_id) {
  if (std::find(entity_type.cbegin(), entity_type.cend(), component_type_id) !=
      entity_type.cend()) {
    return true;
  }

  return false;
}

void EntityManager::CopyComponent(EntityId entity_id, Archetype* new_archetype,
                                  uindex new_entity_index,
                                  const ComponentDescr& component_descr) {
  uindex component_index{0};

  while (component_index < new_archetype->entity_type.size() &&
         new_archetype->entity_type[component_index] !=
             component_descr.type_descr.id) {
    ++component_index;
  }

  COMET_ASSERT(
      component_index < new_archetype->entity_type.size(), "Component ",
      COMET_STRING_ID_LABEL(component_descr.type_descr.id),
      " has not been found in the entity #", entity_id, "! What happened?");

  auto& new_cmp_array{new_archetype->components[component_index]};

  if (component_descr.type_descr.size == 0) {
    return;
  }

  const auto new_cmp_offset{static_cast<sptrdiff>(
      component_descr.type_descr.size * new_entity_index)};

  // Add new component.
  std::memcpy(new_cmp_array.first + new_cmp_offset, component_descr.data,
              component_descr.type_descr.size);
}

bool EntityManager::HasParent(EntityId entity_id, EntityId parent_id) {
  COMET_ASSERT(IsEntity(entity_id), "Trying to check if a dead entity #",
               entity_id, " has a parent entity #", parent_id, "!");
  COMET_ASSERT(IsEntity(entity_id), "Trying to check if an  entity #",
               entity_id, " has a dead parent entity #", parent_id, "!");
  const auto& record{records_[entity_id]};
  const auto& entity_type{record.archetype->entity_type};
  const auto component_type_id{Tag(EntityIdTag::Child, parent_id)};

  for (uindex i{0}; i < entity_type.size(); ++i) {
    if (entity_type[i] == component_type_id) {
      return true;
    }
  }

  return false;
}
}  // namespace entity
}  // namespace comet
