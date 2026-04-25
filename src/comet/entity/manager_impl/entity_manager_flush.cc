// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/entity/entity_manager.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/concurrency/fiber/fiber_primitive.h"
#include "comet/core/concurrency/job/job_utils.h"
#include "comet/core/concurrency/job/scheduler.h"
#include "comet/core/frame/frame_allocator.h"
#include "comet/core/frame/frame_container.h"
#include "comet/core/memory/memory_utils.h"
#include "comet/core/type/array.h"
#include "comet/entity/component.h"
#include "comet/entity/entity_memory_context.h"
#include "comet/entity/entity_type.h"
#include "comet/entity/type/archetype.h"
#include "comet/entity/type/entity_flush.h"
#include "comet/entity/type/entity_id.h"
#include "comet/entity/type/pending_entity.h"
#include "comet/profiler/profiler.h"

namespace comet {
namespace entity {
void EntityManager::Flush() {
  COMET_PROFILE("EntityManager::Flush");
  ProcessPendingOperations();

  {
    fiber::FiberUniqueLock lock{flush_mutex_};
    ++flush_generation_;
  }

  flush_cv_.NotifyAll();
}

usize EntityManager::GetUpdateGeneration() const {
  fiber::FiberUniqueLock lock{flush_mutex_};
  return flush_generation_;
}

void EntityManager::WaitForUpdateGeneration(usize generation) {
  COMET_PROFILE("EntityManager::WaitForUpdateGeneration");
  fiber::FiberUniqueLock lock{flush_mutex_};
  flush_cv_.Wait(
      lock, [this, generation] { return flush_generation_ >= generation; });
}

void EntityManager::WaitForEntityChanges() {
  COMET_PROFILE("EntityManager::WaitForEntityChanges");
  const auto generation{GetUpdateGeneration()};
  WaitForUpdateGeneration(generation + 1);
}

bool EntityManager::HasPendingStructuralChanges() const {
  fiber::FiberLockGuard lock{pending_mutex_};
  return !pending_entities_[write_pending_index_].IsEmpty();
}

void EntityManager::ProcessPendingOperations() {
  COMET_PROFILE("EntityManager::ProcessPendingOperations");

  Map<EntityId, internal::PendingEntity>* read_pending{nullptr};
  memory::FiberStackAllocator* read_allocator{nullptr};

  {
    fiber::FiberLockGuard lock{pending_mutex_};

    const auto read_index{write_pending_index_};
    const auto new_write_index{SwapPendingEntityBuffers()};

    pending_entities_[new_write_index].Release();
    pending_allocators_[new_write_index].Clear();
    pending_entities_[new_write_index] =
        Map<EntityId, internal::PendingEntity>::WithCapacity(
            &pending_allocators_[new_write_index], kPendingEntityInitialCount_);

    read_pending = &pending_entities_[read_index];
    read_allocator = &pending_allocators_[read_index];
  }

  if (read_pending->IsEmpty()) {
    read_pending->Release();
    read_allocator->Clear();
    *read_pending = Map<EntityId, internal::PendingEntity>::WithCapacity(
        read_allocator, kPendingEntityInitialCount_);
    return;
  }

  is_flushing_snapshot_.store(true, std::memory_order_release);

  {
    fiber::FiberSharedLockGuard read_lock{
        snapshot_mutex_, fiber::FiberSharedLockType::Exclusive};

#ifdef COMET_DEBUG_ENTITY
    ValidatePendingOperations(*read_pending);
#endif  // FCOMET_DEBUG_ENTITY

    EnsurePendingComponentTypesKnown(*read_pending);

    auto plan{BuildFlushPlan(*read_pending)};

    if (!plan.IsEmpty()) {
      ReserveTargetArchetypes(plan);
      CopyIntoTargetArchetypes(plan);
      CommitIncomingRecordsToStagingRows(plan);
      ApplyInPlaceUpdates(plan);
      CompactSourceArchetypes(plan);
      CommitArchetypeSizes(plan);
      DestroyDeadEntities(plan);
      ShrinkEmptyOrSparseArchetypes(plan);
    }
  }

  is_flushing_snapshot_.store(false, std::memory_order_release);

  read_pending->Release();
  read_allocator->Clear();
  *read_pending = Map<EntityId, internal::PendingEntity>::WithCapacity(
      read_allocator, kPendingEntityInitialCount_);
}

Map<EntityId, internal::PendingEntity>&
EntityManager::GetWritePendingEntities() {
  return pending_entities_[write_pending_index_];
}

memory::Allocator* EntityManager::GetWritePendingAllocator() {
  return &pending_allocators_[write_pending_index_];
}

u8 EntityManager::SwapPendingEntityBuffers() {
  write_pending_index_ = static_cast<u8>(!write_pending_index_);
  return write_pending_index_;
}

#ifdef COMET_DEBUG_ENTITY
void EntityManager::ValidatePendingOperations(
    const Map<EntityId, internal::PendingEntity>& pending_entities) const {
  auto validate_added_parent{[&](EntityId parent_id, EntityId child_id) {
    COMET_ASSERT(IsEntityUnlocked(child_id),
                 "EntityManager::ValidatePendingOperations",
                 "[pending parent add] entity does not exist", "child_id",
                 child_id, "parent_id", parent_id);
    COMET_ASSERT(IsEntityUnlocked(parent_id),
                 "EntityManager::ValidatePendingOperations",
                 "[pending parent add] parent does not exist", "child_id",
                 child_id, "parent_id", parent_id);
    COMET_ASSERT(!HasAnyParentUnlocked(child_id),
                 "EntityManager::ValidatePendingOperations",
                 "[pending parent add] entity already has a parent", "child_id",
                 child_id, "parent_id", parent_id);
    COMET_ASSERT(!IsDescendantUnlocked(parent_id, child_id),
                 "EntityManager::ValidatePendingOperations",
                 "[pending parent add] would create parent cycle", "child_id",
                 child_id, "parent_id", parent_id);
  }};

  auto validate_added_cmp{[&](EntityId entity_id, EntityId component_id) {
    COMET_ASSERT(IsEntityUnlocked(entity_id),
                 "EntityManager::ValidatePendingOperations",
                 "[pending component add] entity does not exist", "entity_id",
                 entity_id, "component_id", component_id);
  }};

  auto validate_removed_cmp{[&](EntityId entity_id, EntityId component_id) {
    COMET_ASSERT(IsEntityUnlocked(entity_id),
                 "EntityManager::ValidatePendingOperations",
                 "[pending component removal] entity does not exist",
                 "entity_id", entity_id, "component_id", component_id);
  }};

  auto validate_destroyed_entity{[&](EntityId entity_id) {
    COMET_ASSERT(
        IsEntityUnlocked(entity_id), "EntityManager::ValidatePendingOperations",
        "[pending destruction] entity does not exist", "entity_id", entity_id);
  }};

  for (const auto& pair : pending_entities) {
    const auto& pending{pair.value};

    COMET_ASSERT(pair.key == pending.id,
                 "EntityManager::ValidatePendingOperations",
                 "pending entity key/id mismatch", "key", pair.key,
                 "pending_id", pending.id);

    if (pending.is_destroyed) {
      validate_destroyed_entity(pending.id);
      continue;
    }

    for (const auto& added_cmp : pending.added_cmps) {
      const auto component_id{added_cmp.descr.type_descr.id};

      switch (added_cmp.kind) {
        case internal::ComponentKind::Component:
          validate_added_cmp(pending.id, component_id);
          break;

        case internal::ComponentKind::Parent:
          COMET_ASSERT(IsParentTag(component_id),
                       "EntityManager::ValidatePendingOperations",
                       "[pending parent add] component id is not a parent tag",
                       "entity_id", pending.id, "component_id", component_id);

          validate_added_parent(Untag(component_id), pending.id);
          break;

        default:
          COMET_ASSERT(false, "EntityManager::ValidatePendingOperations",
                       "unknown pending component kind", "entity_id",
                       pending.id, "component_id", component_id, "kind",
                       static_cast<u8>(added_cmp.kind));
          break;
      }
    }

    for (const auto removed_cmp_id : pending.removed_cmps) {
      validate_removed_cmp(pending.id, removed_cmp_id);
    }
  }
}
#endif  // COMET_DEBUG_ENTITY

void EntityManager::EnsurePendingComponentTypesKnown(
    const Map<EntityId, internal::PendingEntity>& pending_entities) {
  COMET_PROFILE("EntityManager::EnsurePendingComponentTypesKnown");
  frame::FrameHashSet<ComponentTypeDescr, ComponentTypeDescrHashLogic>
      unique_cmp_type_descrs{};

  for (const auto& pair : pending_entities) {
    const auto& entity{pair.value};

    for (const auto& cmp : entity.added_cmps) {
      unique_cmp_type_descrs.Add(cmp.descr.type_descr);
    }
  }

  known_component_types_.Reserve(known_component_types_.GetEntryCount() +
                                 unique_cmp_type_descrs.GetEntryCount());

  for (const auto& descr : unique_cmp_type_descrs) {
    EnsureComponentTypeKnown(descr);
  }
}

internal::FlushPlan EntityManager::BuildFlushPlan(
    const Map<EntityId, internal::PendingEntity>& pending_entities) {
  COMET_PROFILE("EntityManager::BuildFlushPlan");

  internal::FlushPlan plan{};

  const auto pending_count{pending_entities.GetEntryCount()};

  plan.entity_moves.Reserve(pending_count);
  plan.entity_in_place_updates.Reserve(pending_count);
  plan.destroyed_ids.Reserve(pending_count);
  plan.archetype_plans.Reserve(pending_count);

  for (const auto& pair : pending_entities) {
    const auto& pending{pair.value};
    auto& record{records_.GetOrAdd(pending.id)};

    auto* old_archetype{record.archetype};
    const auto old_row{record.row};

    if (pending.is_destroyed) {
      plan.destroyed_ids.PushLast(pending.id);

#ifdef COMET_DEBUG_ENTITY
      if (old_archetype != nullptr) {
        COMET_ASSERT(
            old_row < old_archetype->size, "EntityManager::BuildFlushPlan",
            "entity record row is out of bounds", "entity_id", pending.id,
            "row", old_row, "archetype_size", old_archetype->size);

        COMET_ASSERT(old_archetype->entity_ids[old_row] == pending.id,
                     "EntityManager::BuildFlushPlan",
                     "entity record row points to another entity", "entity_id",
                     pending.id, "row", old_row, "row_entity_id",
                     old_archetype->entity_ids[old_row]);
      }
#endif  // COMET_DEBUG_ENTITY

      if (old_archetype != nullptr) {
        auto& old_plan{GetOrCreateArchetypePlan(plan, old_archetype)};
        old_plan.destroyed_rows.PushLast(old_row);
        ++old_plan.remove_count;
        --old_plan.final_size;
      }

      continue;
    }

    const auto new_entity_type{GenerateFinalEntityType(old_archetype, pending)};
    auto* new_archetype{GetOrGenerateArchetype(new_entity_type)};

    if (old_archetype == new_archetype) {
      // Check if we actually need to update data.
      if (!pending.added_cmps.IsEmpty()) {
        auto& update = plan.entity_in_place_updates.EmplaceLast();
        update.entity_id = pending.id;
        update.archetype = old_archetype;
        update.row = old_row;
        update.pending = &pending;
      }

      continue;
    }

    auto& new_plan{GetOrCreateArchetypePlan(plan, new_archetype)};
    const auto new_row{new_plan.old_size + new_plan.add_count};

    const auto move_index{plan.entity_moves.GetSize()};
    auto& move{plan.entity_moves.EmplaceLast()};
    move.entity_id = pending.id;
    move.old_archetype = old_archetype;
    move.old_row = old_row;
    move.new_archetype = new_archetype;
    move.new_row = new_row;
    move.pending = &pending;

    new_plan.incoming.PushLast(move_index);
    ++new_plan.add_count;
    ++new_plan.final_size;

    if (old_archetype != nullptr) {
      auto& old_plan{GetOrCreateArchetypePlan(plan, old_archetype)};
      old_plan.outgoing.PushLast(move_index);
      ++old_plan.remove_count;
      --old_plan.final_size;
    }
  }

  for (const auto& archetype_plan : plan.archetype_plans) {
    const auto staging_size{archetype_plan.old_size + archetype_plan.add_count};
    plan.max_staging_archetype_size =
        math::Max(plan.max_staging_archetype_size, staging_size);
  }

  return plan;
}

void EntityManager::ReserveTargetArchetypes(const internal::FlushPlan& plan) {
  COMET_PROFILE("EntityManager::ReserveTargetArchetypes");

  for (const auto& archetype_plan : plan.archetype_plans) {
    COMET_ASSERT(archetype_plan.archetype != nullptr,
                 "EntityManager::ReserveTargetArchetypes", "archetype is null");

    if (archetype_plan.add_count == 0) {
      continue;
    }

    ReserveArchetypeCapacity(
        archetype_plan.archetype,
        archetype_plan.old_size + archetype_plan.add_count);
  }
}

void EntityManager::CopyIntoTargetArchetypes(const internal::FlushPlan& plan) {
  COMET_PROFILE("EntityManager::CopyIntoTargetArchetypes");

  struct JobParams {
    const internal::FlushPlan* plan{nullptr};
    const internal::ArchetypePlan* archetype_plan{nullptr};
  };

  job::CounterGuard guard{};
  auto& scheduler{job::Scheduler::Get()};

  for (const auto& archetype_plan : plan.archetype_plans) {
    if (archetype_plan.incoming.IsEmpty()) {
      continue;
    }

    auto* params{
        COMET_FRAME_ALLOC_ONE_AND_POPULATE(JobParams, &plan, &archetype_plan)};

    scheduler.Kick(job::GenerateJobDescr(
        job::JobPriority::High,
        [](job::JobParamsHandle params_handle) {
          auto* params{reinterpret_cast<const JobParams*>(params_handle)};
          auto& entity_manager{EntityManager::Get()};

          const auto& plan{*params->plan};
          const auto& archetype_plan{*params->archetype_plan};
          auto* new_archetype{archetype_plan.archetype};

          COMET_ASSERT(new_archetype != nullptr,
                       "EntityManager::CopyIntoTargetArchetypes",
                       "new archetype is null");

          for (const auto move_index : archetype_plan.incoming) {
            const auto& move{plan.entity_moves[move_index]};

            COMET_ASSERT(move.new_archetype == new_archetype,
                         "EntityManager::CopyIntoTargetArchetypes",
                         "move target archetype mismatch", "entity_id",
                         move.entity_id);

            COMET_ASSERT(move.pending != nullptr,
                         "EntityManager::CopyIntoTargetArchetypes",
                         "pending entity is null", "entity_id", move.entity_id);

            COMET_ASSERT(move.new_row < new_archetype->capacity,
                         "EntityManager::CopyIntoTargetArchetypes",
                         "new row exceeds archetype capacity", "entity_id",
                         move.entity_id, "new_row", move.new_row, "capacity",
                         new_archetype->capacity);

            new_archetype->entity_ids[move.new_row] = move.entity_id;

            for (usize i{0}; i < new_archetype->entity_type.GetSize(); ++i) {
              auto& new_cmp_array{new_archetype->components[i]};
              const auto component_type_id{new_archetype->entity_type[i]};

              const auto cmp_size{
                  entity_manager.known_component_types_.Get(component_type_id)
                      .type_descr.size};

              if (cmp_size == 0) {
                continue;
              }

              COMET_ASSERT(new_cmp_array.elements != nullptr,
                           "EntityManager::CopyIntoTargetArchetypes",
                           "new component storage is null", "entity_id",
                           move.entity_id, "component_type_id",
                           component_type_id);

              auto* new_cmp_elements{new_cmp_array.elements};
              const auto new_cmp_offset{cmp_size * move.new_row};

              const auto old_cmp_index{
                  move.old_archetype != nullptr
                      ? move.old_archetype->entity_type.GetIndex(
                            component_type_id)
                      : kInvalidIndex};

              const auto* added_cmp{entity_manager.FindAddedComponentDescr(
                  move.pending->added_cmps, component_type_id)};

              if (added_cmp != nullptr) {
                entity_manager.CopyNewComponent(
                    move.pending->added_cmps, component_type_id,
                    new_cmp_elements, new_cmp_offset, cmp_size);
              } else if (old_cmp_index != kInvalidIndex) {
                entity_manager.CopyExistingComponent(
                    move.old_archetype, old_cmp_index, move.old_row,
                    new_cmp_elements, new_cmp_offset, cmp_size);
              } else {
                COMET_ASSERT(
                    false, "EntityManager::CopyIntoTargetArchetypes",
                    "component missing from old archetype and pending add",
                    "entity_id", move.entity_id, "component_type_id",
                    component_type_id);
              }
            }
          }
        },
        params, job::JobStackSize::Normal, guard.GetCounter(),
        "copy_into_target_archetype"));
  }

  guard.Wait();
}

void EntityManager::CommitIncomingRecordsToStagingRows(
    const internal::FlushPlan& plan) {
  COMET_PROFILE("EntityManager::CommitIncomingRecordsToStagingRows");

  for (const auto& move : plan.entity_moves) {
    COMET_ASSERT(move.new_archetype != nullptr,
                 "EntityManager::CommitIncomingRecordsToStagingRows",
                 "new archetype is null", "entity_id", move.entity_id);

    auto& record{records_.GetOrAdd(move.entity_id)};
    record.archetype = move.new_archetype;
    record.row = move.new_row;
  }
}

void EntityManager::ApplyInPlaceUpdates(const internal::FlushPlan& plan) {
  COMET_PROFILE("EntityManager::ApplyInPlaceUpdates");

  for (const auto& update : plan.entity_in_place_updates) {
    auto* archetype = update.archetype;

    for (const auto& cmp : update.pending->added_cmps) {
      const auto cmp_id = cmp.descr.type_descr.id;

      const auto cmp_index = archetype->entity_type.GetIndex(cmp_id);
      COMET_ASSERT(cmp_index != kInvalidIndex,
                   "EntityManager::ApplyInPlaceUpdates",
                   "component index is invalid", "cmp_index", cmp_index,
                   "cmp_id", cmp_id, "entity_id", update.pending->id);

      if (cmp_index == kInvalidIndex) {
        continue;  // Shouldn't happen, but safe.
      }

      const auto cmp_size = known_component_types_.Get(cmp_id).type_descr.size;

      if (cmp_size == 0) {
        continue;
      }

      auto& cmp_array = archetype->components[cmp_index];

      auto* dst = cmp_array.elements + cmp_size * update.row;

      memory::CopyMemory(dst, cmp.descr.data, cmp_size);
    }
  }
}

void EntityManager::CompactSourceArchetypes(internal::FlushPlan& plan) {
  COMET_PROFILE("EntityManager::CompactSourceArchetypes");

  auto& removed_rows{*COMET_FRAME_BITSET_WITH_SIZE(
      math::Max<usize>(plan.max_staging_archetype_size, 16))};

  for (auto& archetype_plan : plan.archetype_plans) {
    auto* archetype{archetype_plan.archetype};

    if (archetype == nullptr || archetype_plan.remove_count == 0) {
      continue;
    }

    const auto old_size{archetype_plan.old_size};
    const auto staging_size{archetype_plan.old_size + archetype_plan.add_count};

    // Step 1: mark rows to remove.
    removed_rows.Clear();
    removed_rows.Resize(staging_size);

    for (const auto move_index : archetype_plan.outgoing) {
      const auto& move{plan.entity_moves[move_index]};

      COMET_ASSERT(move.old_row < old_size,
                   "EntityManager::CompactSourceArchetypes",
                   "outgoing row is out of bounds", "entity_id", move.entity_id,
                   "row", move.old_row, "old_size", old_size);

      removed_rows.Set(move.old_row);
    }

    // Mark destroyed entities.
    for (const auto row : archetype_plan.destroyed_rows) {
      COMET_ASSERT(row < old_size, "EntityManager::CompactSourceArchetypes",
                   "destroyed row is out of bounds", "row", row, "old_size",
                   old_size);

      removed_rows.Set(row);
    }

    // Step 2: compact old holes using the whole staging tail.
    auto tail{staging_size};

    for (usize dst_row{0}; dst_row < old_size; ++dst_row) {
      if (!removed_rows.Test(dst_row)) {
        continue;
      }

      while (tail > dst_row) {
        --tail;

        if (!removed_rows.Test(tail)) {
          break;
        }
      }

      if (tail <= dst_row) {
        break;
      }

      const auto moved_entity_id{archetype->entity_ids[tail]};

      auto& row_move{archetype_plan.compaction_moves.EmplaceLast()};
      row_move.dst_row = dst_row;
      row_move.src_row = tail;
      row_move.moved_entity_id = moved_entity_id;

      archetype->entity_ids[dst_row] = moved_entity_id;

      auto& moved_record{records_.Get(moved_entity_id)};
      moved_record.archetype = archetype;
      moved_record.row = dst_row;

      removed_rows.Set(tail);
    }
  }

  // Step 3: apply component data moves in parallel.
  usize job_count{0};

  for (const auto& archetype_plan : plan.archetype_plans) {
    if (!archetype_plan.compaction_moves.IsEmpty()) {
      ++job_count;
    }
  }

  if (job_count == 0) {
    return;
  }

  auto& scheduler{job::Scheduler::Get()};
  job::CounterGuard guard{};

  auto& job_params{
      *COMET_FRAME_ARRAY_WITH_CAPACITY(internal::ArchetypePlan*, job_count)};

  for (auto& archetype_plan : plan.archetype_plans) {
    if (!archetype_plan.compaction_moves.IsEmpty()) {
      job_params.PushLast(&archetype_plan);
    }
  }

  for (auto* plan_ptr : job_params) {
    scheduler.Kick(job::GenerateJobDescr(
        job::JobPriority::High,
        [](job::JobParamsHandle params_handle) {
          auto* plan{reinterpret_cast<internal::ArchetypePlan*>(params_handle)};
          auto* archetype{plan->archetype};
          auto& em{EntityManager::Get()};

          for (const auto& move : plan->compaction_moves) {
            for (usize i{0}; i < archetype->entity_type.GetSize(); ++i) {
              auto& cmp_array{archetype->components[i]};
              const auto component_type_id{archetype->entity_type[i]};

              const auto cmp_size{
                  em.known_component_types_.Get(component_type_id)
                      .type_descr.size};

              if (cmp_size == 0) {
                continue;
              }

              auto* dst{cmp_array.elements + cmp_size * move.dst_row};
              auto* src{cmp_array.elements + cmp_size * move.src_row};

              memory::CopyMemory(dst, src, cmp_size);
            }
          }
        },
        plan_ptr, job::JobStackSize::Normal, guard.GetCounter(),
        "compact_archetype"));
  }

  guard.Wait();
}

void EntityManager::CommitArchetypeSizes(const internal::FlushPlan& plan) {
  COMET_PROFILE("EntityManager::CommitArchetypeSizes");

  for (const auto& archetype_plan : plan.archetype_plans) {
    auto* archetype{archetype_plan.archetype};

    if (archetype == nullptr) {
      continue;
    }

    COMET_ASSERT(archetype_plan.final_size <= archetype->capacity,
                 "EntityManager::CommitArchetypeSizes",
                 "final size exceeds capacity", "final_size",
                 archetype_plan.final_size, "capacity", archetype->capacity);

    archetype->size = archetype_plan.final_size;
  }
}

void EntityManager::DestroyDeadEntities(const internal::FlushPlan& plan) {
  COMET_PROFILE("EntityManager::DestroyDeadEntities");

  fiber::FiberSharedLockGuard lock{entity_id_mutex_,
                                   fiber::FiberSharedLockType::Exclusive};

  for (const auto entity_id : plan.destroyed_ids) {
    // Remove record first.
    records_.Remove(entity_id);

    // Then invalidate entity ID.
    entity_id_handler_.Destroy(GetGid(entity_id));
  }

#ifdef COMET_DEBUG_ENTITY
  for (const auto entity_id : plan.destroyed_ids) {
    COMET_ASSERT(
        !records_.IsContained(entity_id), "EntityManager::DestroyDeadEntities",
        "entity still has a record after destruction", "entity_id", entity_id);
  }
#endif  // COMET_DEBUG_ENTITY
}

void EntityManager::ShrinkEmptyOrSparseArchetypes(
    const internal::FlushPlan& plan) {
  COMET_PROFILE("EntityManager::ShrinkEmptyOrSparseArchetypes");

  for (const auto& archetype_plan : plan.archetype_plans) {
    auto* archetype{archetype_plan.archetype};

    if (archetype == nullptr) {
      continue;
    }

    // Never destroy root archetype
    if (archetype == root_archetype_) {
      continue;
    }

    const auto size{archetype->size};

    // Step 1: destroy empty archetypes
    if (size == 0) {
      DestroyEmptyArchetype(archetype);
      continue;
    }

    // Step 2: shrink if needed
    ReserveArchetypeCapacity(archetype, size);
  }
}

void EntityManager::EnsureComponentTypeKnown(
    const ComponentTypeDescr& type_descr) {
  auto* component_type{known_component_types_.TryGet(type_descr.id)};

  if (component_type != nullptr) {
    return;
  }

  auto& memory_context{EntityMemoryContext::Get()};
  RegisteredComponentType registered{};

  registered.archetype_map =
      ArchetypeMap{&memory_context.GetArchetypeMapAllocator()};
  registered.type_descr = type_descr;
  registered.archetype_ref_count = 0;

  known_component_types_.Emplace(type_descr.id, registered);
}

internal::ArchetypePlan& EntityManager::GetOrCreateArchetypePlan(
    internal::FlushPlan& plan, Archetype* archetype) {
  COMET_ASSERT(archetype != nullptr, "EntityManager::GetOrCreateArchetypePlan",
               "archetype is null");

  for (auto& archetype_plan : plan.archetype_plans) {
    if (archetype_plan.archetype == archetype) {
      return archetype_plan;
    }
  }

  auto& archetype_plan{plan.archetype_plans.EmplaceLast()};
  archetype_plan.archetype = archetype;
  archetype_plan.old_size = archetype->size;
  archetype_plan.final_size = archetype->size;
  return archetype_plan;
}

EntityType EntityManager::GenerateFinalEntityType(
    const Archetype* old_archetype, const internal::PendingEntity& pending) {
  EntityType out{&EntityMemoryContext::Get().GetEntityTypeAllocator()};

  const auto old_size{
      old_archetype != nullptr ? old_archetype->entity_type.GetSize() : 0};

  // Step 1: estimate final size.
  usize final_size{0};

  // Count surviving old components.
  for (usize i{0}; i < old_size; ++i) {
    const auto id{old_archetype->entity_type[i]};

    if (!pending.removed_cmps.IsContained(id)) {
      ++final_size;
    }
  }

  // Count truly new components.
  for (const auto& cmp : pending.added_cmps) {
    const auto id{cmp.descr.type_descr.id};

    bool exists{false};

    // Check if already in old archetype.
    for (usize i{0}; i < old_size; ++i) {
      if (old_archetype != nullptr && old_archetype->entity_type[i] == id &&
          !pending.removed_cmps.IsContained(id)) {
        exists = true;
        break;
      }
    }

    if (!exists) {
      ++final_size;
    }
  }

  out.Reserve(final_size);

  // Step 2: copy surviving old components.
  for (usize i{0}; i < old_size; ++i) {
    const auto id{old_archetype->entity_type[i]};

    if (!pending.removed_cmps.IsContained(id)) {
      out.PushLast(id);
    }
  }

  // Step 3: add new components (no IsContained needed now).
  for (const auto& cmp : pending.added_cmps) {
    const auto id{cmp.descr.type_descr.id};

    bool exists{false};

    for (usize i{0}; i < out.GetSize(); ++i) {
      if (out[i] == id) {
        exists = true;
        break;
      }
    }

    if (!exists) {
      out.PushLast(id);
    }
  }

  CleanEntityType(out);
  return out;
}

void EntityManager::CopyExistingComponent(
    Archetype* old_archetype, usize old_cmp_index, usize old_entity_index,
    u8* new_cmp_elements, usize new_cmp_offset, usize cmp_size) {
  COMET_ASSERT(old_archetype != nullptr, "EntityManager::CopyExistingComponent",
               "old archetype is null");

  auto& old_cmp_array{old_archetype->components[old_cmp_index]};
  auto* old_cmp_elements{old_cmp_array.elements};

  COMET_ASSERT(old_cmp_elements != nullptr,
               "EntityManager::CopyExistingComponent",
               "old component storage is null");

  const auto old_cmp_offset{cmp_size * old_entity_index};

  memory::CopyMemory(new_cmp_elements + new_cmp_offset,
                     old_cmp_elements + old_cmp_offset, cmp_size);
}

void EntityManager::CopyNewComponent(
    const Array<internal::AddedComponent>& added_cmps,
    EntityId component_type_id, u8* new_cmp_elements, usize new_cmp_offset,
    usize cmp_size) {
  const ComponentDescr* found{nullptr};

  // Linear scan is OK for now.
  for (const auto& cmp : added_cmps) {
    if (cmp.descr.type_descr.id == component_type_id) {
      found = &cmp.descr;
      break;
    }
  }

  if (found != nullptr) {
    COMET_ASSERT(found->data != nullptr || cmp_size == 0,
                 "EntityManager::CopyNewComponent",
                 "component data is null for non-zero size",
                 "component_type_id", component_type_id);

    memory::CopyMemory(new_cmp_elements + new_cmp_offset, found->data,
                       cmp_size);
  } else {
    COMET_ASSERT(cmp_size == 0, "EntityManager::CopyNewComponent",
                 "missing added component data for non-tag component",
                 "component_type_id", component_type_id);
  }
}

void EntityManager::ResizeArchetype(Archetype* archetype, s16 delta) {
  COMET_ASSERT(archetype != nullptr, "EntityManager::ResizeArchetype",
               "archetype is null");

  const auto target{static_cast<ssize>(archetype->size) +
                    static_cast<ssize>(delta)};

  ReserveArchetypeCapacity(archetype,
                           target > 0 ? static_cast<usize>(target) : 0);
}

void EntityManager::ReserveArchetypeCapacity(Archetype* archetype,
                                             usize requested_capacity) {
  COMET_ASSERT(archetype != nullptr, "EntityManager::ReserveArchetypeCapacity",
               "archetype is null");

  auto capacity{requested_capacity};

  if (requested_capacity == 0 && archetype->size == 0) {
    if (archetype == root_archetype_) {
      return;
    }

    DestroyEmptyArchetype(archetype);
    return;
  }

  if (capacity < kMinCapacity_) {
    capacity = kMinCapacity_;
  }

  const auto current_capacity{archetype->capacity};

  if (capacity > current_capacity) {
    if (current_capacity == 0 ||
        capacity >= current_capacity * (1.0f - kGrowthThreshold_)) {
      capacity = math::Max(current_capacity * 2, capacity);
    }
  } else if (capacity < current_capacity) {
    const auto usage{current_capacity == 0
                         ? 1.0f
                         : static_cast<f32>(capacity) / current_capacity};

    if (usage < kShrinkThreshold_ && current_capacity > kMinCapacity_) {
      capacity = math::Max(current_capacity / 2, kMinCapacity_);
    } else {
      capacity = current_capacity;
    }
  }

  auto needs_component_storage{false};

  if (archetype->entity_type.GetSize() > 0) {
    COMET_ASSERT(
        archetype->components.GetSize() >= archetype->entity_type.GetSize(),
        "EntityManager::ReserveArchetypeCapacity",
        "archetype component array is too small");

    for (usize i{0}; i < archetype->entity_type.GetSize(); ++i) {
      const auto cmp_type_id{archetype->entity_type[i]};
      const auto cmp_size{
          known_component_types_.Get(cmp_type_id).type_descr.size};
      const auto expected_size{cmp_size * capacity};
      const auto& cmp_array{archetype->components[i]};

      if (cmp_array.size != expected_size ||
          (expected_size > 0 && cmp_array.elements == nullptr)) {
        needs_component_storage = true;
        break;
      }
    }
  }

  if (capacity == current_capacity && !needs_component_storage) {
    return;
  }

  archetype->entity_ids.Resize(capacity);
  archetype->entity_ids.TrimCapacity();
  archetype->capacity = capacity;

  if (archetype->size > archetype->capacity) {
    archetype->size = archetype->capacity;
  }

  if (archetype->entity_type.GetSize() == 0) {
    return;
  }

  auto& memory_context{EntityMemoryContext::Get()};

  for (usize i{0}; i < archetype->entity_type.GetSize(); ++i) {
    const auto cmp_type_id{archetype->entity_type[i]};
    const auto& old_cmp_array{archetype->components[i]};
    auto* old_cmp_elements{old_cmp_array.elements};
    const auto old_cmp_size{old_cmp_array.size};

    const auto& cmp_type_descr{
        known_component_types_.Get(cmp_type_id).type_descr};

    const auto cmp_size{cmp_type_descr.size};
    const auto cmp_align{cmp_type_descr.align};
    const auto new_size{cmp_size * capacity};
    const auto copy_size{math::Min(old_cmp_size, new_size)};

    archetype->components[i] = {nullptr, new_size};

    if (new_size > 0) {
      archetype->components[i].elements = reinterpret_cast<u8*>(
          memory_context.GetComponentArrayElementsAllocator(new_size)
              .AllocateAligned(new_size, cmp_align));
    }

    if (copy_size != 0) {
      COMET_ASSERT(old_cmp_elements != nullptr,
                   "EntityManager::ReserveArchetypeCapacity",
                   "old component storage is null while copy size is non-zero");

      COMET_ASSERT(archetype->components[i].elements != nullptr,
                   "EntityManager::ReserveArchetypeCapacity",
                   "new component storage is null while copy size is non-zero");

      memory::CopyMemory(archetype->components[i].elements, old_cmp_elements,
                         copy_size);
    }

    if (old_cmp_elements != nullptr) {
      COMET_ASSERT(old_cmp_size > 0, "EntityManager::ReserveArchetypeCapacity",
                   "old component storage exists with zero size");

      memory_context.GetComponentArrayElementsAllocator(old_cmp_size)
          .Deallocate(old_cmp_elements);
    }
  }
}

void EntityManager::DestroyEmptyArchetype(Archetype* archetype) {
  COMET_ASSERT(archetype != nullptr, "EntityManager::DestroyEmptyArchetype",
               "archetype is null");
  COMET_ASSERT(archetype->size == 0, "EntityManager::DestroyEmptyArchetype",
               "archetype is not empty", "size", archetype->size);

  ReleaseArchetypeElements(archetype);

  for (const auto component_type_id : archetype->entity_type) {
    auto* known_component_type{
        known_component_types_.TryGet(component_type_id)};

    if (known_component_type != nullptr) {
      known_component_type->archetype_map.Remove(archetype->id);

      COMET_ASSERT(
          !known_component_type->archetype_map.IsContained(archetype->id),
          "EntityManager::DestroyEmptyArchetype",
          "destroyed archetype still referenced by component type",
          "component_type_id", component_type_id, "archetype_id",
          archetype->id);
    }

    ReleaseComponentTypeArchetypeRef(component_type_id);
  }

  RemoveArchetypeFromList(archetype);
}

void EntityManager::ReleaseArchetypeElements(Archetype* archetype) {
  COMET_ASSERT(archetype != nullptr, "EntityManager::ReleaseArchetypeStorage",
               "archetype is null");

  archetype->entity_ids.Release();

  auto& memory_context{EntityMemoryContext::Get()};

  for (usize i{0}; i < archetype->components.GetSize(); ++i) {
    auto& cmp_array{archetype->components[i]};

    if (cmp_array.elements != nullptr) {
      memory_context.GetComponentArrayElementsAllocator(cmp_array.size)
          .Deallocate(cmp_array.elements);
    }

    cmp_array = {nullptr, 0};
  }

  archetype->capacity = 0;
}

void EntityManager::RemoveArchetypeFromList(Archetype* archetype) {
  const auto index{archetype->index};

  COMET_ASSERT(index < archetypes_.GetSize(), "EntityManager::RemoveArchetype",
               "archetype index out of bounds");
  COMET_ASSERT(archetypes_[index].get() == archetype,
               "EntityManager::RemoveArchetype", "archetype index mismatch");

  const auto last_index{archetypes_.GetSize() - 1};

  if (index != last_index) {
    archetypes_[index].swap(archetypes_[last_index]);
    archetypes_[index]->index = index;
  }

  archetypes_.RemoveFromIndex(last_index);
}

void EntityManager::ReleaseComponentTypeArchetypeRef(
    EntityId component_type_id) {
  auto* component_type{known_component_types_.TryGet(component_type_id)};

  if (component_type == nullptr) {
    return;
  }

  COMET_ASSERT(component_type->archetype_ref_count > 0,
               "EntityManager::ReleaseComponentTypeArchetypeRef",
               "component type archetype ref count underflow",
               "component_type_id", component_type_id);

  --component_type->archetype_ref_count;

  if (component_type->archetype_ref_count == 0) {
    known_component_types_.Remove(component_type_id);
  }
}
}  // namespace entity
}  // namespace comet