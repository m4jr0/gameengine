// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "entity_manager.h"

#include "comet/core/engine.h"
#include "comet/entity/component/transform_component.h"

namespace comet {
namespace entity {
void EntityManager::Initialize() {
  root_archetype_ = GetArchetype(EntityType{});
}

void EntityManager::Destroy() {
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
}

EntityId EntityManager::CreateEntity() {
  EntityId entity_id{entity_id_manager_.CreateBreed()};

  // Special case: adding the entity to the root archetype (which does not
  // contain any components).
  root_archetype_->entity_ids.emplace_back(entity_id);
  records_.emplace(std::make_pair(
      entity_id,
      Record{root_archetype_, root_archetype_->entity_ids.size() - 1}));
  return entity_id;
}

bool EntityManager::IsEntity(const EntityId& entity_id) const {
  return entity_id_manager_.IsAlive(entity_id);
}

void EntityManager::DestroyEntity(EntityId entity_id) {
  COMET_ASSERT(IsEntity(entity_id), "Trying to destroy dead entity #",
               entity_id, "!");

  const auto& record{records_[entity_id]};
  PreRemoveEntityFromArchetype(record.row, record.archetype);
  ResizeArchetype(record.archetype, -1);
  records_.erase(entity_id);
  entity_id_manager_.DestroyBreed(entity_id);
}

void EntityManager::ResizeArchetype(Archetype* archetype, s16 delta) {
  if (delta == 0) {
    return;
  }

  for (uindex i{0}; i < archetype->entity_type.size(); ++i) {
    const auto cmp_type_id{archetype->entity_type[i]};
    auto* old_cmp_elements{archetype->components[i].elements};
    auto old_cmp_size{archetype->components[i].size};
    const auto cmp_size{component_descrs_[cmp_type_id].size};

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
      archetype->components[i].first = utils::memory::AllocAligned(
          &archetype->components[i].elements, new_size,
          component_descrs_[cmp_type_id].alignment);
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

      if (cmp_array.size == 0) {
        continue;
      }

      const auto cmp_size{component_descrs_[archetype->entity_type[i]].size};

      std::memcpy(cmp_array.first + cmp_size * entity_row,
                  cmp_array.first + cmp_size * last_entity_row, cmp_size);
    }

    last_entity_record.row = entity_row;
    archetype->entity_ids[entity_row] = archetype->entity_ids[last_entity_row];
  }
}

bool EntityManager::DoesEntityTypeContain(const EntityType& entity_type,
                                          ComponentTypeId component_type_id) {
  if (std::find(entity_type.cbegin(), entity_type.cend(), component_type_id) !=
      entity_type.cend()) {
    return true;
  }

  return false;
}
}  // namespace entity
}  // namespace comet
