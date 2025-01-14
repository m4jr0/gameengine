// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_ENTITY_ENTITY_MANAGER_H_
#define COMET_COMET_ENTITY_ENTITY_MANAGER_H_

#include <utility>

#include "comet/core/concurrency/fiber/fiber_primitive.h"
#include "comet/core/concurrency/job/job.h"
#include "comet/core/concurrency/job/job_utils.h"
#include "comet/core/essentials.h"
#include "comet/core/frame/frame_utils.h"
#include "comet/core/hash.h"
#include "comet/core/manager.h"
#include "comet/core/memory/memory.h"
#include "comet/core/type/array.h"
#include "comet/entity/archetype.h"
#include "comet/entity/component.h"
#include "comet/entity/entity_id.h"
#include "comet/entity/entity_type.h"
#include "comet/event/event.h"

namespace comet {
namespace entity {
namespace internal {
struct DeferredEntity {
  bool is_destroyed{false};
  EntityId id{kInvalidEntityId};
  frame::FrameArray<ComponentDescr> added_cmps{};
  frame::FrameArray<EntityId> removed_cmps{};
  job::Counter* global_counter{nullptr};
};

HashValue GenerateHash(const ComponentTypeDescr& descr);

struct ComponentTypeDescrHashLogic {
  using Value = ComponentTypeDescr;
  using Hashable = ComponentTypeDescr;

  static const Hashable& GetHashable(const Value& value) { return value; }

  static HashValue Hash(const Hashable& hashable) {
    return GenerateHash(hashable);
  }

  static bool AreEqual(const Hashable& a, const Hashable& b) {
    return a.id == b.id;
  }
};

struct MovedEntities {
  using MoveMap =
      frame::FrameMap<Archetype*,
                      frame::FrameArray<const internal::DeferredEntity*>>;
  MoveMap map{};

  void Add(Archetype* archetype, const internal::DeferredEntity& entity) {
    auto* entities{map.TryGet(archetype)};

    if (entities == nullptr) {
      entities =
          &map.Emplace(archetype,
                       frame::FrameArray<const internal::DeferredEntity*>{})
               .value;
    }

    entities->PushBack(&entity);
  }
};

struct DeferredChanges {
  using ArchetypeDeltas = frame::FrameMap<Archetype*, s16>;
  using DestroyedEntityIds = frame::FrameArray<EntityId>;

  ArchetypeDeltas archetype_size_deltas{};
  MovedEntities added_to{};
  MovedEntities removed_from{};
  DestroyedEntityIds destroyed_ids{static_cast<usize>(16)};
};
}  // namespace internal

class EntityManager : public Manager {
 public:
  static EntityManager& Get();

  EntityManager() = default;
  EntityManager(const EntityManager&) = delete;
  EntityManager(EntityManager&&) = delete;
  EntityManager& operator=(const EntityManager&) = delete;
  EntityManager& operator=(EntityManager&&) = delete;
  virtual ~EntityManager() = default;

  void Initialize() override;
  void Shutdown() override;

  void DispatchComponentChanges();

  void WaitForEntityUpdates();

  EntityId Generate();

  bool IsEntity(const EntityId& entity_id) const;
  void Destroy(EntityId entity_id);

  bool HasComponent(EntityId entity_id, EntityId component_id) const;

  template <typename ComponentType>
  bool HasComponent(EntityId entity_id) const {
    const auto component_type_id{
        ComponentTypeDescrGetter<ComponentType>::Get().id};
    return HasComponent(entity_id, component_type_id);
  }

  template <typename... ComponentTypes>
  void AddComponents(EntityId entity_id, const ComponentTypes&... components) {
    COMET_ASSERT(IsEntity(entity_id), "Entity #", entity_id,
                 " does not exist!");
    fiber::FiberLockGuard lock{deferred_mutex_};
    auto* entity{deferred_entities_->TryGet(entity_id)};

    if (entity == nullptr) {
      entity =
          &deferred_entities_
               ->Emplace(entity_id, internal::DeferredEntity{false, entity_id})
               .value;
    }

    COMET_ASSERT(!entity->is_destroyed, "Entity #", entity_id,
                 " is scheduled to be destroyed!");

    (DeferAddingComponent(entity, components), ...);
  }

  void RemoveComponents(EntityId entity_id,
                        const Array<EntityId>& component_ids);

  template <typename... ComponentIds>
  void RemoveComponents(EntityId entity_id, ComponentIds&&... component_ids) {
    COMET_ASSERT(IsEntity(entity_id), "Entity #", entity_id,
                 " does not exist!");
    fiber::FiberLockGuard lock{deferred_mutex_};
    auto* entity{deferred_entities_->TryGet(entity_id)};

    if (entity == nullptr) {
      entity =
          &deferred_entities_
               ->Emplace(entity_id, internal::DeferredEntity{false, entity_id})
               .value;
    }

    COMET_ASSERT(!entity->is_destroyed, "Entity #", entity_id,
                 " is scheduled to be destroyed!");
    (DeferRemovingComponent(entity->removed_cmps,
                            std::forward<ComponentIds>(component_ids)),
     ...);
  }

  template <typename... ComponentTypes>
  void RemoveComponents(EntityId entity_id) {
    COMET_ASSERT(IsEntity(entity_id), "Entity #", entity_id,
                 " does not exist!");
    fiber::FiberLockGuard lock{deferred_mutex_};
    auto* entity{deferred_entities_->TryGet(entity_id)};

    if (entity == nullptr) {
      entity =
          &deferred_entities_
               ->Emplace(entity_id, internal::DeferredEntity{false, entity_id})
               .value;
    }

    COMET_ASSERT(!entity->is_destroyed, "Entity #", entity_id,
                 " is scheduled to be destroyed!");
    (DeferRemovingComponent(entity,
                            ComponentTypeDescrGetter<ComponentTypes>::Get().id),
     ...);
  }

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
    const auto& archetype_record{
        registered_component_types_.Get(component_type_id)
            .archetype_map.Get(record.archetype->id)};

    return reinterpret_cast<ComponentType*>(
               record.archetype->components[archetype_record.cmp_array_index]
                   .elements) +
           record.row;
  }

  void AddParent(EntityId entity_id, EntityId parent_id);
  bool HasParent(EntityId entity_id, EntityId parent_id);

  template <typename... ComponentTypes, typename Function,
            typename... ComponentTypeIds>
  void Each(const Function& func, ComponentTypeIds... component_type_ids) {
    constexpr auto component_type_count{sizeof...(ComponentTypes)};
    constexpr auto component_type_ids_count{sizeof...(ComponentTypeIds)};
    constexpr auto component_id_count{component_type_count +
                                      component_type_ids_count};

    usize i{0};
    StaticArray<EntityId, component_id_count> all_ids{};
    (void(all_ids[i++] = component_type_ids), ...);
    (void(all_ids[i++] = ComponentTypeDescrGetter<ComponentTypes>::Get().id),
     ...);

    if (all_ids.IsEmpty()) {
      for (const auto& archetype : archetypes_) {
        for (auto entity_id : archetype->entity_ids) {
          func(entity_id);
        }
      }

      return;
    }

    std::sort(all_ids.begin(), all_ids.end());

    for (auto& archetype : archetypes_) {
      if (archetype->entity_type.GetSize() < all_ids.GetSize()) {
        continue;
      }

      usize count{0};

      for (auto component_type_id : archetype->entity_type) {
        if (component_type_id == all_ids[count]) {
          ++count;
        }

        if (count == all_ids.GetSize()) {
          for (usize entity_index{0}; entity_index < archetype->size;
               ++entity_index) {
            func(archetype->entity_ids[entity_index]);
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
  inline static constexpr usize kDeferredEntityInitialCount_{128};

  template <typename EntityType>
  Archetype* GetArchetype(EntityType&& entity_type) {
    for (auto& archetype : archetypes_) {
      if (archetype->entity_type == entity_type) {
        return archetype.get();
      }
    }

    auto archetype{GenerateArchetype()};
    archetype->entity_type = std::forward<EntityType>(entity_type);
    ArchetypeId archetype_id{0};

    for (const auto component_type_id : archetype->entity_type) {
      archetype_id = HashCombine(archetype_id, component_type_id);
      archetype->components.EmplaceBack(ComponentArray{nullptr, 0});
    }

    archetype->id = archetype_id;
    usize i{0};

    for (const auto component_type_id : archetype->entity_type) {
      registered_component_types_[component_type_id]
          .archetype_map[archetype->id]
          .cmp_array_index = i++;
    }

    auto* archetype_p{archetype.get()};
    archetypes_.PushBack(std::move(archetype));
    return archetype_p;
  }

  void RegisterComponentType(const ComponentTypeDescr& type_descr);
  void RegisterComponentTypes(
      const Array<ComponentDescr>& component_type_descrs);
  void UnregisterComponentType(EntityId component_type_id);
  void ResizeArchetype(Archetype* archetype, s16 delta);
  void ReserveArchetypeCapacity(Archetype* archetype, usize capacity);
  bool DoesEntityTypeContain(const EntityType& entity_type,
                             EntityId component_type_id);

  template <typename... ComponentTypes>
  void DeferAddingComponents(internal::DeferredEntity* entity,
                             const ComponentTypes&... component) {
    (DeferAddingComponent(entity, component), ...);
  }

  template <typename ComponentType>
  void DeferAddingComponent(internal::DeferredEntity* entity,
                            const ComponentType& component) {
    const auto& type_descr{ComponentTypeDescrGetter<ComponentType>::Get()};
    entity->removed_cmps.RemoveFromValue(type_descr.id);

    if (type_descr.size == 0) {
      entity->added_cmps.EmplaceBack(type_descr, nullptr);
      return;
    }

    auto& frame_allocator{frame::GetFrameAllocator()};

    auto* data{static_cast<u8*>(
        frame_allocator.AllocateAligned(type_descr.size, type_descr.align))};
    memory::CopyMemory(data, reinterpret_cast<const u8*>(&component),
                       type_descr.size);
    entity->added_cmps.EmplaceBack(type_descr, data);
  }

  void DeferRemovingComponents(internal::DeferredEntity* entity,
                               const Array<EntityId>& src);
  void DeferRemovingComponent(internal::DeferredEntity* entity,
                              const EntityId& id);

  void DeferAddingParent(internal::DeferredEntity* entity, EntityId parent_id);

  // Deferred operations.
  void ProcessDeferredOperations();
  void RegisterDeferredComponentTypes();
  internal::DeferredChanges PopulateChanges();
  void PrepareDeferredDestroyedEntity(internal::DeferredChanges& changes,
                                      const internal::DeferredEntity& entity);
  void PrepareDeferredEntity(internal::DeferredChanges& changes,
                             const internal::DeferredEntity& entity);
  void AddDeferredEntitiesToNewArchetypes(
      const internal::DeferredChanges& changes);
  void RemoveDeferredEntitiesFromOldArchetypes(
      internal::DeferredChanges& changes);
  void ProcessDeferredDestructions(const internal::DeferredChanges& changes);
  void TransferComponents(Archetype* new_archetype, usize new_entity_index,
                          Archetype* old_archetype, usize old_entity_index,
                          const frame::FrameArray<ComponentDescr>& added_cmps);
  void CopyExistingComponent(Archetype* old_archetype, usize old_cmp_index,
                             usize old_entity_index, u8* new_cmp_elements,
                             usize new_cmp_offset, usize cmp_size);
  void CopyNewComponent(const frame::FrameArray<ComponentDescr>& added_cmps,
                        EntityId component_type_id, u8* new_cmp_elements,
                        usize new_cmp_offset, usize cmp_size);
  void ResizeDeferredArchetypes(const internal::DeferredChanges& changes,
                                bool is_growth);
  void PrepareNewFrame();
  void OnEvent(const event::Event& event);

  using DeferredEntities = frame::FrameMap<EntityId, internal::DeferredEntity>;

  bool is_update_{false};
  fiber::FiberMutex deferred_mutex_{};
  fiber::FiberMutex update_mutex_{};
  fiber::FiberCV update_cv_{};
  Archetype* root_archetype_{nullptr};
  Array<ArchetypePtr> archetypes_{};
  gid::BreedHandler entity_id_handler_{};
  gid::BreedHandler component_id_handler_{};
  Records records_{};
  RegisteredComponentTypeMap registered_component_types_{};
  DeferredEntities* deferred_entities_{};
};
}  // namespace entity
}  // namespace comet

#endif  // COMET_COMET_ENTITY_ENTITY_MANAGER_H_
