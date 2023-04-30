// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_ENTITY_ENTITY_MANAGER_H_
#define COMET_COMET_ENTITY_ENTITY_MANAGER_H_

#include "comet_precompile.h"

#include "comet/core/hash.h"
#include "comet/core/manager.h"
#include "comet/core/memory/memory.h"
#include "comet/entity/archetype.h"
#include "comet/entity/component.h"
#include "comet/entity/entity_id.h"
#include "comet/entity/entity_type.h"

namespace comet {
namespace entity {
using EntityManagerDescr = ManagerDescr;

class EntityManager : public Manager {
 public:
  EntityManager() = delete;
  explicit EntityManager(const EntityManagerDescr& descr);
  EntityManager(const EntityManager&) = delete;
  EntityManager(EntityManager&&) = delete;
  EntityManager& operator=(const EntityManager&) = delete;
  EntityManager& operator=(EntityManager&&) = delete;
  virtual ~EntityManager() = default;

  void Initialize() override;
  void Shutdown() override;

  EntityId Generate();
  EntityId Generate(const std::vector<ComponentDescr>& component_descrs);

  bool IsEntity(const EntityId& entity) const;
  void Destroy(EntityId entity);

  bool HasComponent(EntityId entity_id, EntityId component_id) const;
  void AddComponents(EntityId entity_id,
                     const std::vector<ComponentDescr>& component_descrs);
  void RemoveComponents(EntityId entity_id,
                        const std::vector<EntityId>& component_ids);

  template <typename ComponentType>
  ComponentType* GetComponent(EntityId entity_id) {
    const auto component_type_id{
        ComponentTypeDescrGetter<ComponentType>::Get().id};
    COMET_ASSERT(IsEntity(entity_id), "Trying to get a ", component_type_id,
                 " component from a dead entity #", entity_id, "!");

    if (!HasComponent(entity_id, component_type_id)) {
      return nullptr;
    }

    const auto& record{records_[entity_id]};
    const auto& entity_type{record.archetype->entity_type};
    const auto& archetype_record{
        registered_component_types_.at(component_type_id)
            .archetype_map.at(record.archetype->id)};

    return reinterpret_cast<ComponentType*>(
               record.archetype->components[archetype_record.cmp_array_index]
                   .first) +
           record.row;
  }

  bool HasParent(EntityId entity_id, EntityId parent_id);

  template <typename... ComponentTypes, typename Function,
            typename... ComponentTypeIds>
  void Each(const Function& func, ComponentTypeIds... component_type_ids) {
    constexpr auto component_type_count{sizeof...(ComponentTypes)};
    constexpr auto component_type_ids_count{sizeof...(ComponentTypeIds)};
    constexpr auto component_id_count{component_type_count +
                                      component_type_ids_count};

    uindex i{0};
    std::array<EntityId, component_id_count> all_ids{};
    (void(all_ids[i++] = component_type_ids), ...);
    (void(all_ids[i++] = ComponentTypeDescrGetter<ComponentTypes>::Get().id),
     ...);

    if (all_ids.size() == 0) {
      for (const auto* archetype : archetypes_) {
        for (auto entity_id : archetype->entity_ids) {
          func(entity_id);
        }
      }

      return;
    }

    std::sort(all_ids.begin(), all_ids.end());

    for (Archetype* archetype : archetypes_) {
      if (archetype->entity_type.size() < all_ids.size()) {
        continue;
      }

      uindex count{0};

      for (auto component_type_id : archetype->entity_type) {
        if (component_type_id == all_ids[count]) {
          ++count;
        }

        if (count == all_ids.size()) {
          for (auto entity_id : archetype->entity_ids) {
            func(entity_id);
          }

          break;
        }
      }
    }
  }

  template <typename... ComponentTypes, typename Function>
  void EachChild(const Function& func, EntityId parent_id) {
    Each<ComponentTypes...>(func, Tag(EntityIdTag::Child, parent_id));
  }

 private:
  template <typename EntityType>
  Archetype* GetArchetype(EntityType&& entity_type) {
    for (auto* archetype : archetypes_) {
      if (archetype->entity_type == entity_type) {
        return archetype;
      }
    }

    auto* archetype{new Archetype{}};
    archetype->entity_type = std::forward<EntityType>(entity_type);
    archetypes_.push_back(archetype);
    ArchetypeId archetype_id{0};

    for (const auto component_type_id : archetype->entity_type) {
      archetype_id = HashCombine(archetype_id, component_type_id);
      archetype->components.push_back(ComponentArray{nullptr, nullptr, 0});
    }

    archetype->id = archetype_id;
    uindex i{0};

    for (const auto component_type_id : archetype->entity_type) {
      registered_component_types_[component_type_id]
          .archetype_map[archetype->id]
          .cmp_array_index = i++;
    }

    return archetype;
  }

  void RegisterComponentType(const ComponentDescr& component_descr);
  void RegisterComponentTypes(
      const std::vector<ComponentDescr>& component_type_descrs);
  void UnregisterComponentType(EntityId component_type_id);
  void ResizeArchetype(Archetype* archetype, s16 delta);
  void PreRemoveEntityFromArchetype(uindex entity_row, Archetype* archetype);
  bool DoesEntityTypeContain(const EntityType& entity_type,
                             EntityId component_type_id);
  void CopyComponent(EntityId entity_id, Archetype* new_archetype,
                     uindex new_entity_index,
                     const ComponentDescr& component_descr);

  Archetype* root_archetype_{nullptr};
  std::vector<Archetype*> archetypes_{};
  gid::BreedHandler entity_id_handler_{};
  gid::BreedHandler component_id_handler_{};
  Records records_{};
  RegisteredComponentTypeMap registered_component_types_{};
};
}  // namespace entity
}  // namespace comet

#endif  // COMET_COMET_ENTITY_ENTITY_MANAGER_H_
