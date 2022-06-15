// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_ENTITY_ENTITY_MANAGER_H_
#define COMET_COMET_ENTITY_ENTITY_MANAGER_H_

#include "comet_precompile.h"

#include "comet/entity/component/component.h"
#include "comet/entity/component_view.h"
#include "comet/entity/entity.h"
#include "comet/entity/entity_misc_types.h"
#include "comet/utils/memory/memory.h"

namespace comet {
namespace entity {
class EntityManager {
 public:
  EntityManager() = default;
  EntityManager(const EntityManager&) = delete;
  EntityManager(EntityManager&&) = delete;
  EntityManager& operator=(const EntityManager&) = delete;
  EntityManager& operator=(EntityManager&&) = delete;
  ~EntityManager() = default;

  void Initialize();
  void Destroy();

  EntityId CreateEntity();

  template <typename... ComponentTypes>
  EntityId CreateEntity(ComponentTypes&&... components) {
    static_assert((std::is_trivially_copyable<ComponentTypes>::value, ...),
                  "Component type must be trivially copyable.");

    EntityId entity_id{entity_id_manager_.CreateBreed()};

    (RegisterComponentIfNeeded<ComponentTypes>(), ...);
    auto* archetype{GetArchetype(GenerateEntityType<ComponentTypes...>())};
    ResizeArchetype(archetype, 1);

    auto entity_index{archetype->entity_ids.size() - 1};
    archetype->entity_ids[entity_index] = entity_id;

    records_.emplace(
        std::make_pair(entity_id, Record{archetype, entity_index}));

    (CopyComponent(entity_id, archetype, entity_index,
                   std::forward<ComponentTypes>(components)),
     ...);

    return entity_id;
  }

  bool IsEntity(const EntityId& entity) const;
  void DestroyEntity(EntityId entity);

  template <typename ComponentTypeId>
  bool HasComponent(EntityId entity_id) {
    const auto kComponentTypeId{ComponentTypeId::kComponentTypeId};
    const auto& entity_type = records_[entity_id].archetype->entity_type;

    for (const auto& other_component_id : entity_type) {
      if (kComponentTypeId == other_component_id) {
        return true;
      }
    }

    return false;
  }

  template <typename ComponentType>
  bool IsComponentRegistered() {
    return component_descrs_.find(ComponentType::kComponentTypeId) !=
           component_descrs_.cend();
  }

  template <typename ComponentType>
  void RegisterComponent() {
    component_descrs_[ComponentType::kComponentTypeId] = {
        sizeof(ComponentType), alignof(ComponentType)};
  }

  template <typename ComponentType>
  void RegisterComponentIfNeeded() {
    if (!IsComponentRegistered<ComponentType>()) {
      RegisterComponent<ComponentType>();
    }
  }

  template <typename ComponentType, typename... Args>
  void AddComponent(EntityId entity_id, Args&&... args) {
    AddComponents<ComponentType>(entity_id,
                                 ComponentType{std::forward<Args>(args)...});
  }

  template <typename... ComponentTypes>
  void const AddComponents(EntityId entity_id, ComponentTypes&&... components) {
    static_assert((std::is_trivially_copyable<ComponentTypes>::value, ...),
                  "Component type must be trivially copyable.");

    // Step 1: apply safety checks + handle new archetype.
    COMET_ASSERT(IsEntity(entity_id),
                 "Trying to add component to dead entity #", entity_id, "!");

    (RegisterComponentIfNeeded<ComponentTypes>(), ...);
    auto added_entity_type{entity::GenerateEntityType<ComponentTypes...>()};
    constexpr auto kAddedComponentCount{sizeof...(ComponentTypes)};

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
      const auto component_type_id{new_entity_type[i]};
      const auto cmp_size{component_descrs_[component_type_id].size};
      const auto new_cmp_offset{
          static_cast<sptrdiff>(cmp_size * new_entity_index)};
      const auto old_cmp_offset{
          static_cast<sptrdiff>(cmp_size * old_entity_index)};

      if (new_cmp_cursor < kAddedComponentCount &&
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
        std::memcpy(
            new_cmp_array.first + new_cmp_offset,
            old_archetype->components[old_cmp_cursor].first + old_cmp_offset,
            cmp_size);
        ++old_cmp_cursor;
      }
    }

    // Copy new components.
    (CopyComponent(entity_id, new_archetype, new_entity_index,
                   std::forward<ComponentTypes>(components)),
     ...);

    // Step 4: copy last entity inplace of current entity, if necessary.
    PreRemoveEntityFromArchetype(entity_record.row, old_archetype);

    // Step 5: decrease old archetype's size.
    ResizeArchetype(old_archetype, -1);

    new_archetype->entity_ids[new_entity_index] = entity_id;
    entity_record.archetype = new_archetype;
    entity_record.row = new_entity_index;
  }

  template <typename ComponentType>
  void RemoveComponent(EntityId entity_id) {
    RemoveComponents<ComponentType>(entity_id);
  }

  template <typename... ComponentTypes>
  void const RemoveComponents(EntityId entity_id) {
    auto removed_entity_type{GenerateEntityType<ComponentTypes...>()};
    auto removed_component_count{removed_entity_type.size()};
    COMET_ASSERT(removed_component_count != 0,
                 "Trying to remove 0 component for entity #", entity_id, "!");
    COMET_ASSERT(IsEntity(entity_id),
                 "Trying to remove component from dead entity #", entity_id,
                 "!");

    auto& entity_record{records_[entity_id]};
    auto* old_archetype{entity_record.archetype};
    const auto& old_entity_type{old_archetype->entity_type};
    const auto old_component_count{old_entity_type.size()};

    for (uindex i{0}; i < removed_component_count; ++i) {
      COMET_ASSERT(
          DoesEntityTypeContain(old_entity_type, removed_entity_type[i]),
          "Entity #", entity_id, " does not contain component ",
          COMET_STRING_ID_LABEL(removed_entity_type[i]), "!");
    }

    COMET_ASSERT(removed_component_count <= old_component_count,
                 "Component count to remove for entity #", entity_id,
                 ": is too high (", removed_component_count, " > ",
                 old_component_count, ")!");

    auto* new_archetype{GetArchetype(
        RemoveFromEntityType(old_entity_type, removed_entity_type))};

    // Step 2: increase new archetype's size.
    ResizeArchetype(new_archetype, 1);

    // Step 3: copy entity's components into new archetype.
    const uindex new_entity_index{new_archetype->entity_ids.size() - 1};
    const auto old_entity_index{entity_record.row};

    for (uindex i{0}; i < new_archetype->entity_type.size(); ++i) {
      auto& new_cmp_array{new_archetype->components[i]};
      const auto cmp_size{
          component_descrs_[new_archetype->entity_type[i]].size};
      const auto new_cmp_offset{
          static_cast<sptrdiff>(cmp_size * new_entity_index)};
      const auto old_cmp_offset{
          static_cast<sptrdiff>(cmp_size * old_entity_index)};

      // Copy non-removed components.
      std::memcpy(new_cmp_array.first + new_cmp_offset,
                  old_archetype->components[i].first + old_cmp_offset,
                  cmp_size);
    }

    // Step 4: copy last entity inplace of current entity, if necessary.
    PreRemoveEntityFromArchetype(entity_record.row, old_archetype);

    // Step 5: decrease old archetype's size.
    ResizeArchetype(old_archetype, -1);

    new_archetype->entity_ids[new_entity_index] = entity_id;
    entity_record.archetype = new_archetype;
    entity_record.row = new_entity_index;
  }

  template <typename ComponentType>
  ComponentType* GetComponent(EntityId entity_id) {
    COMET_ASSERT(IsEntity(entity_id), "Trying to get a ",
                 COMET_STRING_ID_LABEL(ComponentType::kComponentTypeId),
                 " component from a dead entity #", entity_id, "!");

    const auto& record{records_[entity_id]};
    const auto& entity_type{record.archetype->entity_type};

    for (uindex i{0}; i < entity_type.size(); ++i) {
      if (ComponentType::kComponentTypeId == entity_type[i]) {
        return reinterpret_cast<ComponentType*>(
                   record.archetype->components[i].first) +
               record.row;
      }
    }

    return nullptr;
  }

  template <typename... ComponentTypes>
  ComponentView GetView() {
    ComponentTypeId component_type_ids[] = {
        ComponentTypes::kComponentTypeId...};
    return ComponentView{component_type_ids, sizeof...(ComponentTypes),
                         archetypes_};
  }

  ComponentView GetView() { return ComponentView{nullptr, 0, archetypes_}; }

 private:
  Archetype* root_archetype_{nullptr};
  std::unordered_map<ComponentTypeId, ComponentDescr> component_descrs_{};
  std::vector<Archetype*> archetypes_{};
  gid::BreedManager entity_id_manager_{};
  gid::BreedManager component_id_manager_{};
  std::unordered_map<EntityId, Record> records_;

  template <typename EntityType>
  Archetype* GetArchetype(EntityType&& entity_type) {
    for (auto* archetype : archetypes_) {
      if (archetype->entity_type == entity_type) {
        return archetype;
      }
    }

    auto* archetype{new Archetype{}};
    archetype->entity_type = std::forward<EntityType>(entity_type);
    archetypes_.emplace_back(archetype);

    for (const auto component_type_id : archetype->entity_type) {
      archetype->components.emplace_back(ComponentArray{nullptr, nullptr, 0});
    }

    return archetype;
  }

  void ResizeArchetype(Archetype* archetype, s16 delta);
  void PreRemoveEntityFromArchetype(uindex entity_row, Archetype* archetype);
  bool DoesEntityTypeContain(const EntityType& entity_type,
                             ComponentTypeId component_type_id);

 private:
  template <typename ComponentType>
  void CopyComponent(EntityId entity_id, Archetype* new_archetype,
                     uindex new_entity_index, ComponentType&& component) {
    const auto kAddedComponentTypeId{ComponentType::kComponentTypeId};
    uindex component_index{0};

    while (component_index < new_archetype->entity_type.size() &&
           new_archetype->entity_type[component_index] !=
               kAddedComponentTypeId) {
      ++component_index;
    }

    COMET_ASSERT(component_index < new_archetype->entity_type.size(),
                 "Component ", COMET_STRING_ID_LABEL(kAddedComponentTypeId),
                 " has not been found in the entity #", entity_id,
                 "! What happened?");

    auto& new_cmp_array{new_archetype->components[component_index]};
    const auto cmp_size{component_descrs_[kAddedComponentTypeId].size};
    const auto new_cmp_offset{
        static_cast<sptrdiff>(cmp_size * new_entity_index)};

    auto new_cmp{ComponentType{std::forward<ComponentType>(component)}};

    // Add new component.
    std::memcpy(new_cmp_array.first + new_cmp_offset,
                reinterpret_cast<void*>(&new_cmp), cmp_size);
  }
};
}  // namespace entity
}  // namespace comet

#endif  // COMET_COMET_ENTITY_ENTITY_MANAGER_H_
