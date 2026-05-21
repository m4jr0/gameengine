// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_ENTITY_ENTITY_MANAGER_H_
#define COMET_COMET_ENTITY_ENTITY_MANAGER_H_

// External. ///////////////////////////////////////////////////////////////////
#ifdef COMET_DEBUG
#include <atomic>
#endif  // COMET_DEBUG

#include <utility>
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/concurrency/fiber/fiber_primitive.h"
#include "comet/core/essentials.h"
#include "comet/core/manager.h"
#include "comet/core/memory/memory.h"
#include "comet/core/type/array.h"
#include "comet/entity/component.h"
#include "comet/entity/entity_type.h"
#include "comet/entity/type/archetype.h"
#include "comet/entity/type/entity_flush.h"
#include "comet/entity/type/entity_id.h"
#include "comet/entity/type/pending_entity.h"
#include "comet/event/event.h"
#include "comet/event/event_manager.h"

namespace comet {
namespace entity {
class EntityManager : public Manager {
 private:
  // Private constants.
  static inline constexpr f32 kGrowthThreshold_{.1f};
  static inline constexpr f32 kShrinkThreshold_{.25f};
  static inline constexpr usize kMinCapacity_{16};
  inline static constexpr usize kPendingAllocatorCapacity_{262144};  // 256 KiB.

 public:
  static EntityManager& Get();

  EntityManager() = default;
  EntityManager(const EntityManager&) = delete;
  EntityManager(EntityManager&&) = delete;
  EntityManager& operator=(const EntityManager&) = delete;
  EntityManager& operator=(EntityManager&&) = delete;
  ~EntityManager() override = default;

  // Lifecycle. | entity_manager_lifecycle.cc
 protected:
  void OnInitialize() override;
  void OnShutdown() override;

 private:
  void OnEvent(const event::Event& event);
  void RegisterEvents();
  void UnregisterEvents();

  event::EventListenerId end_frame_listener_id_{event::kInvalidEventListenerId};
  using PendingEntities = Map<EntityId, internal::PendingEntity>;

  // Flushing. | entity_manager_flush.cc
 public:
  void Flush();

  usize GetUpdateGeneration() const;
  void WaitForUpdateGeneration(usize generation);
  void WaitForEntityChanges();
  bool HasPendingStructuralChanges() const;

 private:
  void ProcessPendingOperations();

  Map<EntityId, internal::PendingEntity>& GetWritePendingEntities();
  memory::Allocator* GetWritePendingAllocator();
  u8 SwapPendingEntityBuffers();

#ifdef COMET_DEBUG_ENTITY
  void ValidatePendingOperations(
      const Map<EntityId, internal::PendingEntity>& pending_entities) const;
#endif  // COMET_DEBUG_ENTITY

  void EnsurePendingComponentTypesKnown(
      const Map<EntityId, internal::PendingEntity>& pending_entities);

  internal::FlushPlan BuildFlushPlan(
      const Map<EntityId, internal::PendingEntity>& pending_entities);

  void ReserveTargetArchetypes(const internal::FlushPlan& plan);
  void CopyIntoTargetArchetypes(const internal::FlushPlan& plan);
  void CommitIncomingRecordsToStagingRows(const internal::FlushPlan& plan);
  void ApplyInPlaceUpdates(const internal::FlushPlan& plan);
  void CompactSourceArchetypes(internal::FlushPlan& plan);
  void CommitArchetypeSizes(const internal::FlushPlan& plan);
  void DestroyDeadEntities(const internal::FlushPlan& plan);
  void ShrinkEmptyOrSparseArchetypes(const internal::FlushPlan& plan);

  void EnsureComponentTypeKnown(const ComponentTypeDescr& type_descr);

  internal::ArchetypePlan& GetOrCreateArchetypePlan(internal::FlushPlan& plan,
                                                    Archetype* archetype);
  EntityType GenerateFinalEntityType(const Archetype* old_archetype,
                                     const internal::PendingEntity& pending);

  void CopyExistingComponent(Archetype* old_archetype, usize old_cmp_index,
                             usize old_entity_index, u8* new_cmp_elements,
                             usize new_cmp_offset, usize cmp_size);
  void CopyNewComponent(const Array<internal::AddedComponent>& added_cmps,
                        EntityId component_type_id, u8* new_cmp_elements,
                        usize new_cmp_offset, usize cmp_size);

  void ResizeArchetype(Archetype* archetype, s16 delta);
  void ReserveArchetypeCapacity(Archetype* archetype, usize capacity);

  void DestroyEmptyArchetype(Archetype* archetype);
  void ReleaseArchetypeElements(Archetype* archetype);
  void RemoveArchetypeFromList(Archetype* archetype);
  void ReleaseComponentTypeArchetypeRef(EntityId component_type_id);

  usize flush_generation_{0};
  mutable fiber::FiberMutex flush_mutex_{};
  fiber::FiberCV flush_cv_{};

  // Read operations. | entity_manager_read_operations.cc
 public:
  bool IsEntity(const EntityId& entity_id) const;

  bool HasComponent(EntityId entity_id, EntityId component_id) const;

  template <typename ComponentType>
  bool HasComponent(EntityId entity_id) const;

  template <typename ComponentType>
  ComponentType* GetComponent(EntityId entity_id);

  template <typename ComponentType>
  const ComponentType* GetComponent(EntityId entity_id) const;

  EntityId GetParentId(EntityId entity_id) const;
  bool HasParent(EntityId entity_id, EntityId parent_id) const;
  bool HasAnyParent(EntityId entity_id) const;

  EntityId FindParentId(const EntityType* entity_type) const;
  bool IsDescendant(EntityId entity_id, EntityId potential_ancestor_id) const;

  const EntityType* TryGetEntityType(EntityId entity_id) const;

  usize GetEntityCount() const;
  usize GetEntityCapacity() const;

 private:
  bool IsEntityUnlocked(const EntityId& entity_id) const;

  bool HasComponentUnlocked(EntityId entity_id, EntityId component_id) const;

  template <typename ComponentType>
  bool HasComponentUnlocked(EntityId entity_id) const;

  template <typename ComponentType>
  ComponentType* GetComponentUnlocked(EntityId entity_id);

  template <typename ComponentType>
  const ComponentType* GetComponentUnlocked(EntityId entity_id) const;

  EntityId GetParentIdUnlocked(EntityId entity_id) const;
  bool HasParentUnlocked(EntityId entity_id, EntityId parent_id) const;
  bool HasAnyParentUnlocked(EntityId entity_id) const;

  EntityId FindParentIdUnlocked(const EntityType* entity_type) const;
  bool IsDescendantUnlocked(EntityId entity_id,
                            EntityId potential_ancestor_id) const;

  const EntityType* TryGetEntityTypeUnlocked(EntityId entity_id) const;

  usize GetEntityCountUnlocked() const;
  usize GetEntityCapacityUnlocked() const;

  template <typename ComponentType>
  ComponentType* GetArchetypeComponentData(Archetype& archetype);

  template <typename ComponentType>
  const ComponentType* GetArchetypeComponentData(
      const Archetype& archetype) const;

  template <usize N>
  static bool DoesArchetypeMatch(const Archetype& archetype,
                                 const StaticArray<EntityId, N>& all_ids);

  // Write operations. | entity_manager_write_operations.cc
 public:
  EntityId Generate();
  void Destroy(EntityId entity_id);

  template <typename... ComponentTypes>
  void AddComponents(EntityId entity_id, const ComponentTypes&... components);

  template <typename... ComponentTypes>
  void AddChildComponents(EntityId entity_id, EntityId parent_id,
                          const ComponentTypes&... components);

  void RemoveComponents(EntityId entity_id,
                        const Array<EntityId>& component_ids);

  template <typename... ComponentIds>
  void RemoveComponents(EntityId entity_id, ComponentIds&&... component_ids);

  template <typename... ComponentTypes>
  void RemoveComponents(EntityId entity_id);

  void AddParent(EntityId entity_id, EntityId parent_id);

 private:
  template <typename EntityType>
  Archetype* GetOrGenerateArchetype(EntityType&& entity_type);

  // Read iterations. | entity_manager_read_iteration.h
 public:
  // Public API.
  template <typename... ComponentTypes, typename Fn,
            typename... ComponentTypeIds>
  void ForEach(const Fn& fn, ComponentTypeIds... component_type_ids);

  template <typename... ComponentTypes, typename Fn,
            typename... ComponentTypeIds>
  void ForEach(const Fn& fn, ComponentTypeIds... component_type_ids) const;

  template <typename... ComponentTypes, typename Fn,
            typename... ComponentTypeIds>
  void ForEachId(const Fn& fn, ComponentTypeIds... component_type_ids) const;

  template <typename... ComponentTypes, typename Fn>
  void ForEachChild(const Fn& fn, EntityId parent_id);

  template <typename... ComponentTypes, typename Fn>
  void ForEachChild(const Fn& fn, EntityId parent_id) const;

  template <typename... ComponentTypes, typename Fn>
  void ForEachChildId(const Fn& fn, EntityId parent_id) const;

 private:
  // Unprotected API.
  template <typename... ComponentTypes, typename Fn,
            typename... ComponentTypeIds>
  void ForEachUnlocked(const Fn& fn, ComponentTypeIds... component_type_ids);

  template <typename... ComponentTypes, typename Fn,
            typename... ComponentTypeIds>
  void ForEachUnlocked(const Fn& fn,
                       ComponentTypeIds... component_type_ids) const;

  template <typename... ComponentTypes, typename Fn,
            typename... ComponentTypeIds>
  void ForEachIdUnlocked(const Fn& fn,
                         ComponentTypeIds... component_type_ids) const;

  template <typename... ComponentTypes, typename Fn>
  void ForEachChildUnlocked(const Fn& fn, EntityId parent_id);

  template <typename... ComponentTypes, typename Fn>
  void ForEachChildUnlocked(const Fn& fn, EntityId parent_id) const;

  template <typename... ComponentTypes, typename Fn>
  void ForEachChildIdUnlocked(const Fn& fn, EntityId parent_id) const;

  // Internal helpers.
  template <typename Fn>
  static void ForEachEntityIdInArchetype(const Archetype& archetype,
                                         const Fn& fn);

  template <typename... ComponentTypes, typename Fn,
            typename... ComponentTypeIds>
  void ForEachArchetypeMatching(const Fn& fn,
                                ComponentTypeIds... component_type_ids);

  template <typename... ComponentTypes, typename Fn,
            typename... ComponentTypeIds>
  void ForEachArchetypeMatching(const Fn& fn,
                                ComponentTypeIds... component_type_ids) const;

  // Pending. | entity_manager_pending.cc
 public:
  usize GetPendingEntityCount() const;

 private:
  inline static constexpr usize kPendingEntityInitialCount_{128};

  internal::PendingEntity& GetOrCreatePendingEntity(EntityId entity_id);

  bool HasPendingComponentAdd(const internal::PendingEntity* pending_entity,
                              EntityId component_type_id) const;
  bool HasPendingComponentRemoval(const internal::PendingEntity* pending_entity,
                                  EntityId component_type_id) const;

  const ComponentDescr* FindAddedComponentDescr(
      const Array<internal::AddedComponent>& added_cmps,
      EntityId component_type_id) const;

  EntityId FindPendingParentId(
      const internal::PendingEntity* pending_entity) const;

  template <typename... ComponentTypes>
  void DeferAddingComponents(internal::PendingEntity* entity,
                             const ComponentTypes&... components);

  template <typename ComponentType>
  void DeferAddingComponent(internal::PendingEntity* entity,
                            const ComponentType& component);

  void DeferRemovingComponents(internal::PendingEntity* entity,
                               const Array<EntityId>& src);
  void DeferRemovingComponent(internal::PendingEntity* entity,
                              const EntityId& id);

  void DeferAddingParent(internal::PendingEntity* entity, EntityId parent_id);

  mutable fiber::FiberMutex pending_mutex_{};

  internal::EntityPendingAllocator pending_allocators_[2]{
      {kPendingAllocatorCapacity_, memory::kEngineMemoryTagPendingEntity1,
       memory::kEngineMemoryTagPendingEntity1Extended},
      {kPendingAllocatorCapacity_, memory::kEngineMemoryTagPendingEntity2,
       memory::kEngineMemoryTagPendingEntity2Extended}};

  PendingEntities pending_entities_[2]{};
  u8 write_pending_index_{0};

  // Reads during flush. | entity_manager_reads_during_flush.cc
 private:
#ifdef COMET_DEBUG
  void DiagnosePublicReadDuringFlush(const schar* context) const;
#endif  // COMET_DEBUG

  template <typename Fn>
  decltype(auto) ReadSnapshot(const schar* context, Fn&& fn) const;

  std::atomic_bool is_flushing_snapshot_{false};
  mutable fiber::FiberSharedMutex snapshot_mutex_{};

  // Data. | entity_manager_data.cc
 private:
  template <typename... ComponentTypes, typename... ComponentTypeIds>
  static auto BuildSortedComponentIdArray(
      ComponentTypeIds... component_type_ids);

  mutable fiber::FiberSharedMutex entity_id_mutex_{};
  Archetype* root_archetype_{nullptr};
  Array<ArchetypePtr> archetypes_{};
  gid::BreedHandler entity_id_handler_{};
  Records records_{};
  RegisteredComponentTypeMap known_component_types_{};
};
}  // namespace entity
}  // namespace comet

#include "comet/entity/manager_impl/entity_manager_data_template.h"
#include "comet/entity/manager_impl/entity_manager_pending_template.h"
#include "comet/entity/manager_impl/entity_manager_read_iteration_template.h"
#include "comet/entity/manager_impl/entity_manager_read_operation_template.h"
#include "comet/entity/manager_impl/entity_manager_reads_during_flush_template.h"
#include "comet/entity/manager_impl/entity_manager_write_operation_template.h"

#endif  // COMET_COMET_ENTITY_ENTITY_MANAGER_H_