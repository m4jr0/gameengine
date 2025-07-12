// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "comet_pch.h"

#include "entity_manager.h"

#include "comet/core/concurrency/job/job_utils.h"
#include "comet/core/concurrency/job/scheduler.h"
#include "comet/core/frame/frame_allocator.h"
#include "comet/core/frame/frame_event.h"
#include "comet/core/memory/memory_utils.h"
#include "comet/entity/entity_memory_manager.h"
#include "comet/entity/factory/entity_factory_manager.h"
#include "comet/event/event_manager.h"
#include "comet/math/math_common.h"
#include "comet/profiler/profiler.h"

namespace comet {
namespace entity {
namespace internal {
HashValue GenerateHash(const ComponentTypeDescr& descr) {
  return comet::GenerateHash(descr.id);
}

const ComponentTypeDescrHashLogic::Hashable&
ComponentTypeDescrHashLogic::GetHashable(const Value& value) {
  return value;
}

HashValue ComponentTypeDescrHashLogic::Hash(const Hashable& hashable) {
  return GenerateHash(hashable);
}

bool ComponentTypeDescrHashLogic::AreEqual(const Hashable& a,
                                           const Hashable& b) {
  return a.id == b.id;
}

void MovedEntities::Add(Archetype* archetype,
                        const internal::DeferredEntity& entity) {
  auto* entities{map.TryGet(archetype)};

  if (entities == nullptr) {
    entities =
        &map.Emplace(archetype,
                     frame::FrameArray<const internal::DeferredEntity*>{})
             .value;
  }

  entities->PushBack(&entity);
}

bool MovedEntities::IsEmpty() const { return map.IsEmpty(); }

bool DeferredChanges::IsEmpty() const {
  return archetype_size_deltas.IsEmpty() && added_to.IsEmpty() &&
         removed_from.IsEmpty() && destroyed_ids.IsEmpty();
}
}  // namespace internal

EntityManager& EntityManager::Get() {
  static EntityManager singleton{};
  return singleton;
}

void EntityManager::Initialize() {
  Manager::Initialize();
  auto& memory_manager{EntityMemoryManager::Get()};
  memory_manager.Initialize();

  records_ = Records{&memory_manager.GetRecordAllocator()};

  registered_component_types_ = RegisteredComponentTypeMap{
      &memory_manager.GetRegisteredComponentTypeMapAllocator()};

  // TODO(m4jr0): Use configuration?
  // Tags: configuration entity memory
  archetypes_ =
      Array<ArchetypePtr>{&memory_manager.GetArchetypePointerAllocator()};

  root_archetype_ = GetArchetype(EntityType{});
  deferred_entities_ = COMET_FRAME_ALLOC_ONE_AND_POPULATE(
      DeferredEntities, kDeferredEntityInitialCount_);
  EntityFactoryManager::Get().Initialize();

  auto on_event{COMET_EVENT_BIND_FUNCTION(OnEvent)};

  event::EventManager::Get().Register(on_event,
                                      frame::NewFrameEvent::kStaticType_);
  event::EventManager::Get().Register(on_event,
                                      frame::EndFrameEvent::kStaticType_);
}

void EntityManager::Shutdown() {
  EntityFactoryManager::Get().Shutdown();
  auto& memory_manager{EntityMemoryManager::Get()};

  for (auto& archetype : archetypes_) {
    for (auto& cmp_array : archetype->components) {
      if (cmp_array.size == 0) {
        continue;
      }

      memory_manager.GetComponentArrayElementsAllocator(cmp_array.size)
          .Deallocate(cmp_array.elements);
      cmp_array.elements = nullptr;
    }
  }

  archetypes_.Destroy();
  component_id_handler_.Shutdown();
  root_archetype_ = nullptr;
  entity_id_handler_.Shutdown();
  records_.Destroy();
  registered_component_types_.Destroy();
  deferred_entities_ = nullptr;
  memory_manager.Shutdown();
  Manager::Shutdown();
}

void EntityManager::DispatchComponentChanges() {
  COMET_PROFILE("EntityManager::DispatchComponentChanges");
  ProcessDeferredOperations();

  {
    fiber::FiberUniqueLock lock{update_mutex_};
    is_update_ = true;
  }

  update_cv_.NotifyAll();

  {
    fiber::FiberUniqueLock lock{update_mutex_};
    is_update_ = false;
  }
}

void EntityManager::WaitForEntityUpdates() {
  fiber::FiberUniqueLock lock{update_mutex_};
  update_cv_.Wait(lock, [this] { return is_update_; });
}

EntityId EntityManager::Generate() {
  auto new_entity_id{entity_id_handler_.Generate()};
  fiber::FiberLockGuard lock{deferred_mutex_};
  COMET_ASSERT(deferred_entities_ != nullptr, "Deferred entities are null!");
  deferred_entities_->Emplace(new_entity_id,
                              internal::DeferredEntity{false, new_entity_id});
  return new_entity_id;
}

bool EntityManager::IsEntity(const EntityId& entity_id) const {
  return entity_id != kInvalidEntityId &&
         entity_id_handler_.IsAlive(GetGid(entity_id));
}

void EntityManager::Destroy(EntityId entity_id) {
  COMET_ASSERT(IsEntity(entity_id),
               "Attempting to destroy a non-existent entity!");
  {
    fiber::FiberLockGuard lock{deferred_mutex_};
    COMET_ASSERT(deferred_entities_ != nullptr, "Deferred entities are null!");
    auto* entity{deferred_entities_->TryGet(entity_id)};

    if (entity != nullptr) {
      entity->is_destroyed = true;
      entity->added_cmps.Destroy();
      entity->removed_cmps.Destroy();
    } else {
      COMET_ASSERT(deferred_entities_ != nullptr,
                   "Deferred entities are null!");
      deferred_entities_->Emplace(entity_id,
                                  internal::DeferredEntity{true, entity_id});
    }
  }

  EachChild<>([&](auto child_entity_id) { Destroy(child_entity_id); },
              entity_id);
}

bool EntityManager::HasComponent(EntityId entity_id,
                                 EntityId component_id) const {
  const auto* record_ptr{records_.TryGet(entity_id)};

  if (record_ptr == nullptr) {
    return false;
  }

  const auto& record{*record_ptr};
  const auto* archetype{record.archetype};
  const auto& archetype_map{
      registered_component_types_.Get(component_id).archetype_map};
  return archetype_map.IsContained(archetype->id);
}

void EntityManager::RemoveComponents(EntityId entity_id,
                                     const Array<EntityId>& component_ids) {
  COMET_ASSERT(IsEntity(entity_id), "Entity #", entity_id, " does not exist!");
  fiber::FiberLockGuard lock{deferred_mutex_};
  COMET_ASSERT(deferred_entities_ != nullptr, "Deferred entities are null!");
  auto* entity{deferred_entities_->TryGet(entity_id)};

  if (entity == nullptr) {
    entity =
        &deferred_entities_
             ->Emplace(entity_id, internal::DeferredEntity{false, entity_id})
             .value;
  }

  COMET_ASSERT(!entity->is_destroyed, "Entity #", entity_id,
               " is scheduled to be destroyed!");
  DeferRemovingComponents(entity, component_ids);
}

void EntityManager::AddParent(EntityId entity_id, EntityId parent_id) {
  COMET_ASSERT(EntityManager::Get().IsEntity(entity_id),
               "Trying to add dead entity #", entity_id,
               " to a parent (entity #", parent_id, ")!");

  COMET_ASSERT(EntityManager::Get().IsEntity(parent_id),
               "Trying to add entity #", entity_id,
               " to a dead parent (entity #", parent_id, ")!");

  fiber::FiberLockGuard lock{deferred_mutex_};
  COMET_ASSERT(deferred_entities_ != nullptr, "Deferred entities are null!");
  auto* entity{deferred_entities_->TryGet(entity_id)};

  if (entity == nullptr) {
    entity =
        &deferred_entities_
             ->Emplace(entity_id, internal::DeferredEntity{false, entity_id})
             .value;
  }

  COMET_ASSERT(!entity->is_destroyed, "Entity #", entity_id,
               " is scheduled to be destroyed!");
  DeferAddingParent(entity, parent_id);
}

bool EntityManager::HasParent(EntityId entity_id, EntityId parent_id) {
  COMET_ASSERT(IsEntity(entity_id), "Trying to check if a dead entity #",
               entity_id, " has a parent entity #", parent_id, "!");
  COMET_ASSERT(IsEntity(entity_id), "Trying to check if an  entity #",
               entity_id, " has a dead parent entity #", parent_id, "!");

  const auto& record{records_[entity_id]};
  const auto& entity_type{record.archetype->entity_type};
  const auto component_type_id{Tag(EntityIdTag::Child, parent_id)};

  for (usize i{0}; i < entity_type.GetSize(); ++i) {
    if (entity_type[i] == component_type_id) {
      return true;
    }
  }

  return false;
}

void EntityManager::RegisterComponentType(
    const ComponentTypeDescr& type_descr) {
  auto* component_type{registered_component_types_.TryGet(type_descr.id)};

  if (component_type != nullptr) {
    ++component_type->use_count;
    return;
  }

  auto& memory_manager{EntityMemoryManager::Get()};
  RegisteredComponentType registered{};

  registered.archetype_map =
      ArchetypeMap{&memory_manager.GetArchetypeMapAllocator()};
  registered.type_descr = type_descr;
  registered.use_count = 1;

  registered_component_types_.Emplace(type_descr.id, registered);
}

void EntityManager::RegisterComponentTypes(
    const Array<ComponentDescr>& component_descrs) {
  for (const auto& component_descr : component_descrs) {
    RegisterComponentType(component_descr.type_descr);
  }
}

void EntityManager::UnregisterComponentType(EntityId component_type_id) {
  auto* component_type{registered_component_types_.TryGet(component_type_id)};

  if (component_type == nullptr) {
    return;
  }

  if (--component_type->use_count == 0) {
    registered_component_types_.Remove(component_type_id);
  }
}

void EntityManager::ResizeArchetype(Archetype* archetype, s16 delta) {
  ReserveArchetypeCapacity(archetype, archetype->entity_ids.GetSize() + delta);
}

void EntityManager::ReserveArchetypeCapacity(Archetype* archetype,
                                             usize capacity) {
  constexpr f32 kGrowthThreshold{0.1f};
  constexpr f32 kShrinkThreshold{0.25f};
  constexpr usize kMinCapacity{16};

  if (capacity < kMinCapacity) {
    capacity = kMinCapacity;
  }

  auto current_capacity{archetype->capacity};

  if (capacity > current_capacity) {
    if (capacity >= current_capacity * (1 - kGrowthThreshold)) {
      capacity = math::Max(current_capacity * 2, capacity);
    }
  } else if (capacity < current_capacity) {
    auto usage{static_cast<f32>(capacity) / current_capacity};

    if (usage < kShrinkThreshold && current_capacity > kMinCapacity) {
      capacity = math::Max(current_capacity / 2, kMinCapacity);
    }
  }

  if (capacity == current_capacity) {
    return;
  }

  auto delta{static_cast<ssize>(capacity) -
             static_cast<ssize>(current_capacity)};

  archetype->entity_ids.Resize(math::Max(usize{0}, capacity));
  archetype->capacity = capacity;

  if (archetype->size > archetype->capacity) {
    archetype->size = archetype->capacity;
  }

  // Case: nothing to resize.
  if (archetype->entity_type.GetSize() == 0) {
    return;
  }

  for (usize i{0}; i < archetype->entity_type.GetSize(); ++i) {
    const auto cmp_type_id{archetype->entity_type[i]};
    const auto& old_cmp_array{archetype->components[i]};
    auto* old_cmp_elements{old_cmp_array.elements};
    auto old_cmp_size{old_cmp_array.size};
    const auto& cmp_type_descr{
        registered_component_types_[cmp_type_id].type_descr};
    const auto cmp_size{cmp_type_descr.size};
    const auto cmp_align{cmp_type_descr.align};

    usize new_size{0};
    auto tmp{static_cast<s32>(old_cmp_size) +
             static_cast<s32>(cmp_size) * delta};

    if (tmp > 0) {
      new_size = tmp;
    }

    // If new_size < copy_size, the last existing components will be
    // truncated.
    const auto copy_size{math::Min(old_cmp_size, new_size)};
    archetype->components[i] = {nullptr, new_size};
    auto& memory_manager{EntityMemoryManager::Get()};

    if (new_size > 0) {
      archetype->components[i].elements = reinterpret_cast<u8*>(
          memory_manager.GetComponentArrayElementsAllocator(new_size)
              .AllocateAligned(new_size, cmp_align));
    }

    if (copy_size != 0) {
      memory::CopyMemory(archetype->components[i].elements, old_cmp_elements,
                         copy_size);
    }

    if (old_cmp_size != 0) {
      memory_manager.GetComponentArrayElementsAllocator(old_cmp_size)
          .Deallocate(old_cmp_elements);
      old_cmp_elements = nullptr;
    }
  }
}

bool EntityManager::DoesEntityTypeContain(const EntityType& entity_type,
                                          EntityId component_type_id) {
  return entity_type.IsContained(component_type_id);
}

void EntityManager::DeferRemovingComponents(internal::DeferredEntity* entity,
                                            const Array<EntityId>& src) {
  for (const auto& id : src) {
    DeferRemovingComponent(entity, id);
  }
}

void EntityManager::DeferRemovingComponent(internal::DeferredEntity* entity,
                                           const EntityId& id) {
  bool is_removal{false};

  for (usize i{0}; i < entity->added_cmps.GetSize(); ++i) {
    if (entity->added_cmps[i].type_descr.id == id) {
      entity->added_cmps.RemoveFromIndex(i);
      is_removal = true;
    }
  }

  if (is_removal) {
    return;
  }

  entity->removed_cmps.PushBack(id);
}

void EntityManager::DeferAddingParent(internal::DeferredEntity* entity,
                                      EntityId parent_id) {
  ComponentTypeDescr type_descr{};
  type_descr.id = Tag(EntityIdTag::Child, parent_id);
  type_descr.size = 0;
  RegisterComponentType(type_descr);
  entity->removed_cmps.RemoveFromValue(type_descr.id);
  entity->added_cmps.EmplaceBack(type_descr, nullptr);
}

void EntityManager::ProcessDeferredOperations() {
  fiber::FiberLockGuard lock{deferred_mutex_};
  RegisterDeferredComponentTypes();

  internal::DeferredChanges changes{PopulateChanges()};

  if (!changes.IsEmpty()) {
    ResizeDeferredArchetypes(changes, true);
    AddDeferredEntitiesToNewArchetypes(changes);
    RemoveDeferredEntitiesFromOldArchetypes(changes);
    ProcessDeferredDestructions(changes);
    ResizeDeferredArchetypes(changes, false);
  }

  deferred_entities_ = COMET_FRAME_ALLOC_ONE_AND_POPULATE(
      DeferredEntities, kDeferredEntityInitialCount_);
}

void EntityManager::RegisterDeferredComponentTypes() {
  frame::FrameHashSet<ComponentTypeDescr, internal::ComponentTypeDescrHashLogic>
      unique_cmp_type_descrs{};

  COMET_ASSERT(deferred_entities_ != nullptr, "Deferred entities are null!");

  for (const auto& pair : *deferred_entities_) {
    const auto& entity{pair.value};

    for (const auto& cmp : entity.added_cmps) {
      unique_cmp_type_descrs.Add(cmp.type_descr);
    }
  }

  registered_component_types_.Reserve(unique_cmp_type_descrs.GetEntryCount());

  for (const auto& descr : unique_cmp_type_descrs) {
    RegisterComponentType(descr);
  }
}

internal::DeferredChanges EntityManager::PopulateChanges() {
  internal::DeferredChanges changes{};
  COMET_ASSERT(deferred_entities_ != nullptr, "Deferred entities are null!");

  for (const auto& pair : *deferred_entities_) {
    auto& entity{pair.value};

    if (entity.is_destroyed) {
      PrepareDeferredDestroyedEntity(changes, entity);
    } else {
      PrepareDeferredEntity(changes, entity);
    }
  }

  return changes;
}

void EntityManager::PrepareDeferredDestroyedEntity(
    internal::DeferredChanges& changes,
    const internal::DeferredEntity& entity) {
  changes.destroyed_ids.PushBack(entity.id);
  auto* record{records_.TryGet(entity.id)};

  if (record != nullptr && record->archetype != nullptr) {
    --changes.archetype_size_deltas[record->archetype];
    changes.removed_from.Add(record->archetype, entity);
  }
}

void EntityManager::PrepareDeferredEntity(
    internal::DeferredChanges& changes,
    const internal::DeferredEntity& entity) {
  auto& record{records_[entity.id]};
  auto* old_archetype{record.archetype};
  EntityType new_entity_type{};

  if (old_archetype != nullptr) {
    new_entity_type = old_archetype->entity_type;

    if (!entity.added_cmps.IsEmpty()) {
      new_entity_type = AddToEntityType(new_entity_type,
                                        GenerateEntityType(entity.added_cmps));
    }
  } else {
    new_entity_type = GenerateEntityType(entity.added_cmps);
  }

  if (!entity.removed_cmps.IsEmpty()) {
    new_entity_type = RemoveFromEntityType(
        new_entity_type, GenerateEntityType(entity.removed_cmps));
  }

  auto* new_archetype{GetArchetype(new_entity_type)};

  if (old_archetype != nullptr) {
    changes.removed_from.Add(old_archetype, entity);
    --changes.archetype_size_deltas[old_archetype];
  }

  ++changes.archetype_size_deltas[new_archetype];

  changes.added_to.Add(new_archetype, entity);
}

void EntityManager::AddDeferredEntitiesToNewArchetypes(
    const internal::DeferredChanges& changes) {
  struct JobParams {
    usize new_entity_index{kInvalidIndex};
    const internal::DeferredEntity* entity{nullptr};
    Archetype* new_archetype{nullptr};
  };

  job::CounterGuard guard{};
  auto& scheduler{job::Scheduler::Get()};

  for (auto& pair : changes.added_to.map) {
    auto* new_archetype{pair.key};
    auto& entities{pair.value};
    auto base_index{new_archetype->size};
    new_archetype->size += entities.GetSize();

    for (const auto* entity : entities) {
      auto new_entity_index{base_index++};

      auto* params{COMET_FRAME_ALLOC_ONE_AND_POPULATE(
          JobParams, new_entity_index, entity, new_archetype)};

      scheduler.Kick(job::GenerateJobDescr(
          job::JobPriority::High,
          [](job::JobParamsHandle params_handle) {
            auto* params{reinterpret_cast<const JobParams*>(params_handle)};
            auto* entity{params->entity};
            auto* new_archetype{params->new_archetype};
            auto new_entity_index{params->new_entity_index};
            auto& entity_manager{EntityManager::Get()};

            auto& record{entity_manager.records_[entity->id]};
            auto* old_archetype{record.archetype};
            new_archetype->entity_ids[new_entity_index] = entity->id;
            record.archetype = new_archetype;
            auto old_entity_index{record.row};
            record.row = new_entity_index;

            entity_manager.TransferComponents(new_archetype, new_entity_index,
                                              old_archetype, old_entity_index,
                                              entity->added_cmps);
          },
          params, job::JobStackSize::Normal, guard.GetCounter(),
          "add_deferred_entity"));
    }
  }

  guard.Wait();
}

void EntityManager::RemoveDeferredEntitiesFromOldArchetypes(
    internal::DeferredChanges& changes) {
  struct JobParams {
    usize old_entity_index{kInvalidIndex};
    usize swap_index{kInvalidIndex};
    Archetype* old_archetype{nullptr};
  };

  job::CounterGuard guard{};
  auto& scheduler{job::Scheduler::Get()};

  for (auto& pair : changes.removed_from.map) {
    auto* old_archetype{pair.key};
    auto& entities{pair.value};
    usize offset{0};

    std::sort(entities.begin(), entities.end(),
              [this](const auto* a, const auto* b) {
                return records_[a->id].row < records_[b->id].row;
              });

    for (const auto* entity : entities) {
      auto& record{records_[entity->id]};
      auto old_entity_index{record.row};
      auto swap_index{old_archetype->size - 1 - offset++};

      if (old_entity_index == swap_index) {
        continue;
      }

      auto* params{COMET_FRAME_ALLOC_ONE_AND_POPULATE(
          JobParams, old_entity_index, swap_index, old_archetype)};

      scheduler.Kick(job::GenerateJobDescr(
          job::JobPriority::High,
          [](job::JobParamsHandle params_handle) {
            auto* params{reinterpret_cast<const JobParams*>(params_handle)};
            auto* old_archetype{params->old_archetype};
            auto old_entity_index{params->old_entity_index};
            auto swap_index{params->swap_index};
            auto& entity_manager{EntityManager::Get()};

            auto& last_entity_record{
                entity_manager.records_[old_archetype->entity_ids[swap_index]]};

            for (usize i{0}; i < old_archetype->entity_type.GetSize(); ++i) {
              auto& cmp_array{old_archetype->components[i]};
              auto component_type_id{old_archetype->entity_type[i]};

              auto cmp_size{
                  entity_manager.registered_component_types_[component_type_id]
                      .type_descr.size};

              if (cmp_size > 0) {
                auto* cmp_elements{cmp_array.elements};
                auto* entity_data{cmp_elements + cmp_size * old_entity_index};
                auto* last_entity_data{cmp_elements + cmp_size * swap_index};
                memory::CopyMemory(entity_data, last_entity_data, cmp_size);
              }
            }

            last_entity_record.row = old_entity_index;
            old_archetype->entity_ids[old_entity_index] =
                old_archetype->entity_ids[swap_index];
          },
          params, job::JobStackSize::Normal, guard.GetCounter(),
          "remove_deferred_entity"));
    }

    old_archetype->size -= entities.GetSize();
  }

  guard.Wait();
}

void EntityManager::ProcessDeferredDestructions(
    const internal::DeferredChanges& changes) {
  for (auto entity_id : changes.destroyed_ids) {
    records_.Remove(entity_id);
    entity_id_handler_.Destroy(GetGid(entity_id));
  }
}

void EntityManager::TransferComponents(
    Archetype* new_archetype, usize new_entity_index, Archetype* old_archetype,
    usize old_entity_index,
    const frame::FrameArray<ComponentDescr>& added_cmps) {
  for (usize i{0}; i < new_archetype->entity_type.GetSize(); ++i) {
    auto& new_cmp_array{new_archetype->components[i]};
    auto component_type_id{new_archetype->entity_type[i]};
    auto cmp_size{
        registered_component_types_[component_type_id].type_descr.size};

    if (cmp_size > 0) {
      auto* new_cmp_elements{new_cmp_array.elements};
      auto new_cmp_offset{cmp_size * new_entity_index};

      auto old_cmp_index{
          old_archetype != nullptr
              ? old_archetype->entity_type.GetIndex(component_type_id)
              : kInvalidIndex};

      if (old_cmp_index != kInvalidIndex) {
        CopyExistingComponent(old_archetype, old_cmp_index, old_entity_index,
                              new_cmp_elements, new_cmp_offset, cmp_size);
      } else {
        CopyNewComponent(added_cmps, component_type_id, new_cmp_elements,
                         new_cmp_offset, cmp_size);
      }
    }
  }
}

void EntityManager::CopyExistingComponent(
    Archetype* old_archetype, usize old_cmp_index, usize old_entity_index,
    u8* new_cmp_elements, usize new_cmp_offset, usize cmp_size) {
  auto& old_cmp_array{old_archetype->components[old_cmp_index]};
  auto* old_cmp_elements{old_cmp_array.elements};
  auto old_cmp_offset{cmp_size * old_entity_index};

  memory::CopyMemory(new_cmp_elements + new_cmp_offset,
                     old_cmp_elements + old_cmp_offset, cmp_size);
}

void EntityManager::CopyNewComponent(
    const frame::FrameArray<ComponentDescr>& added_cmps,
    EntityId component_type_id, u8* new_cmp_elements, usize new_cmp_offset,
    usize cmp_size) {
  const ComponentDescr* found_added_cmp{nullptr};

  for (auto& added_cmp : added_cmps) {
    if (added_cmp.type_descr.id == component_type_id) {
      found_added_cmp = &added_cmp;
      break;
    }
  }

  COMET_ASSERT(found_added_cmp != nullptr,
               "Tried adding a non-existing component!");
  memory::CopyMemory(new_cmp_elements + new_cmp_offset, found_added_cmp->data,
                     cmp_size);
}

void EntityManager::ResizeDeferredArchetypes(
    const internal::DeferredChanges& changes, bool is_growth) {
  struct JobParams {
    s16 delta{0};
    Archetype* archetype{nullptr};
  };

  job::CounterGuard guard{};
  auto& scheduler{job::Scheduler::Get()};

  for (const auto& pair : changes.archetype_size_deltas) {
    if (pair.value == 0 || (pair.value < 0 && is_growth)) {
      continue;
    }

    auto* params{
        COMET_FRAME_ALLOC_ONE_AND_POPULATE(JobParams, pair.value, pair.key)};

    scheduler.Kick(job::GenerateJobDescr(
        job::JobPriority::High,
        [](job::JobParamsHandle params_handle) {
          auto* params{reinterpret_cast<const JobParams*>(params_handle)};
          EntityManager::Get().ResizeArchetype(params->archetype,
                                               params->delta);
        },
        params, job::JobStackSize::Normal, guard.GetCounter(),
        "grow_archetype"));
  }

  guard.Wait();
}

void EntityManager::PrepareNewFrame() {
  COMET_ASSERT(deferred_entities_ == nullptr || deferred_entities_->IsEmpty(),
               "No all entity changes have been processed!");

  deferred_entities_ = COMET_FRAME_ALLOC_ONE_AND_POPULATE(
      DeferredEntities, kDeferredEntityInitialCount_);
}

void EntityManager::OnEvent(const event::Event& event) {
  const auto& event_type{event.GetType()};

  if (event_type == frame::NewFrameEvent::kStaticType_) {
    PrepareNewFrame();
  } else if (event_type == frame::EndFrameEvent::kStaticType_) {
    DispatchComponentChanges();
  }
}
}  // namespace entity
}  // namespace comet
