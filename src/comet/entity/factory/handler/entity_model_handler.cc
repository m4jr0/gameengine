// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "comet_pch.h"

#include "entity_model_handler.h"

#include "comet/animation/animation_manager.h"
#include "comet/animation/component/animation_component.h"
#include "comet/core/c_string.h"
#include "comet/core/concurrency/job/job_utils.h"
#include "comet/core/concurrency/job/scheduler.h"
#include "comet/core/frame/frame_allocator.h"
#include "comet/core/frame/frame_manager.h"
#include "comet/core/frame/frame_packet.h"
#include "comet/core/type/tstring.h"
#include "comet/entity/entity_event.h"
#include "comet/entity/entity_manager.h"
#include "comet/event/event_manager.h"
#include "comet/geometry/component/mesh_component.h"
#include "comet/geometry/geometry_manager.h"
#include "comet/physics/component/transform_component.h"
#include "comet/physics/physics_manager.h"
#include "comet/profiler/profiler.h"

namespace comet {
namespace entity {
EntityId ModelHandler::GenerateStatic(
    CTStringView model_path, resource::ResourceLifeSpan life_span) const {
  COMET_PROFILE("ModelHandler::GenerateStatic");
  auto& entity_manager{EntityManager::Get()};
  auto root_entity_id{entity_manager.Generate()};

  auto model_cmp{geometry::GeometryManager::Get().GenerateStaticModelComponent(
      root_entity_id, model_path, life_span)};
  const auto* model_resource{model_cmp.resource};

  if (model_resource == nullptr) {
    return kInvalidEntityId;
  }

  auto& physics_manager{physics::PhysicsManager::Get()};

  entity_manager.AddComponents(
      root_entity_id,
      physics_manager.GenerateTransformComponent(root_entity_id), model_cmp,
      physics_manager.GenerateTransformRootComponent());

  job::CounterGuard guard{};

#ifdef COMET_FIBER_DEBUG_LABEL
  constexpr auto* kDebugLabelPrefix{"mesh_sta_"};
  constexpr auto kDebugLabelPrefixLen{GetLength(kDebugLabelPrefix)};
#endif  // COMET_FIBER_DEBUG_LABEL

  auto entity_ids{internal::ParentEntityIds{&frame::GetFrameAllocator(),
                                            model_resource->meshes.GetSize()}};

  for (const auto& mesh : model_resource->meshes) {
    entity_ids[mesh.internal_id] = entity_manager.Generate();
  }

  usize job_count{0};
  auto& scheduler{job::Scheduler::Get()};

  for (const auto& mesh : model_resource->meshes) {
    auto* job_params{COMET_FRAME_ALLOC_ONE_AND_POPULATE(
        internal::StaticGenerationJobParams)};
    job_params->life_span = life_span;
    job_params->id = entity_ids[mesh.internal_id];
    job_params->root_entity_id = root_entity_id;

    job_params->parent_id = mesh.parent_id == resource::kInvalidResourceId
                                ? root_entity_id
                                : entity_ids[mesh.parent_id];

    job_params->mesh = &mesh;

#ifdef COMET_FIBER_DEBUG_LABEL
    schar debug_label[fiber::Fiber::kDebugLabelMaxLen_ + 1]{'\0'};
    Copy(debug_label, kDebugLabelPrefix, fiber::Fiber::kDebugLabelMaxLen_);
    ConvertToStr(mesh.internal_id, debug_label + kDebugLabelPrefixLen,
                 fiber::Fiber::kDebugLabelMaxLen_ - kDebugLabelPrefixLen);
#else
    schar* debug_label{nullptr};
#endif  // COMET_FIBER_DEBUG_LABEL

    scheduler.Kick(job::GenerateJobDescr(
        job::JobPriority::Normal, OnStaticGeneration, job_params,
        job::JobStackSize::Normal, guard.GetCounter(), debug_label));

    if (++job_count % kMaxConcurrentJobs_ == 0) {
      guard.Wait();
    }
  }

  guard.Wait();
  event::EventManager::Get().FireEvent<ModelLoadedEvent>(root_entity_id);
  return root_entity_id;
}

EntityId ModelHandler::GenerateSkeletal(
    CTStringView model_path, resource::ResourceLifeSpan life_span) const {
  COMET_PROFILE("ModelHandler::GenerateSkeletal");
  auto& entity_manager{EntityManager::Get()};
  auto root_entity_id{entity_manager.Generate()};

  auto& geometry_manager{geometry::GeometryManager::Get()};
  auto model_cmp{geometry_manager.GenerateSkeletalModelComponent(
      root_entity_id, model_path, life_span)};

  const auto* model_resource{model_cmp.resource};

  if (model_resource == nullptr) {
    return kInvalidEntityId;
  }

  auto skeleton_cmp{
      geometry_manager.GenerateSkeletonComponent(model_path, life_span)};

  const auto* skeleton_resource{skeleton_cmp.resource};

  if (skeleton_resource == nullptr) {
    if (life_span == resource::ResourceLifeSpan::Manual) {
      geometry_manager.DestroySkeletalModelComponent(&model_cmp);
    }

    return kInvalidEntityId;
  }

  auto& physics_manager{physics::PhysicsManager::Get()};

  entity_manager.AddComponents(
      root_entity_id,
      physics_manager.GenerateTransformComponent(root_entity_id), skeleton_cmp,
      model_cmp, physics_manager.GenerateTransformRootComponent(),
      animation::AnimationManager::Get().GenerateAnimationComponent(
          animation::kInvalidAnimationClipId, 1.0f, std::nullopt, life_span));

  job::CounterGuard guard{};

#ifdef COMET_FIBER_DEBUG_LABEL
  constexpr auto* kDebugLabelPrefix{"mesh_ske_"};
  constexpr auto kDebugLabelPrefixLen{GetLength(kDebugLabelPrefix)};
#endif  // COMET_FIBER_DEBUG_LABEL

  auto entity_ids{internal::ParentEntityIds{&frame::GetFrameAllocator(),
                                            model_resource->meshes.GetSize()}};

  for (const auto& mesh : model_resource->meshes) {
    entity_ids[mesh.internal_id] = entity_manager.Generate();
  }

  usize job_count{0};
  auto& scheduler{job::Scheduler::Get()};

  for (const auto& mesh : model_resource->meshes) {
    auto* job_params{COMET_FRAME_ALLOC_ONE_AND_POPULATE(
        internal::SkeletalGenerationJobParams)};
    job_params->life_span = life_span;
    job_params->id = entity_ids[mesh.internal_id];
    job_params->root_entity_id = root_entity_id;

    job_params->parent_id = mesh.parent_id == resource::kInvalidResourceId
                                ? root_entity_id
                                : entity_ids[mesh.parent_id];

    job_params->mesh = &mesh;

#ifdef COMET_FIBER_DEBUG_LABEL
    schar debug_label[fiber::Fiber::kDebugLabelMaxLen_ + 1]{'\0'};
    Copy(debug_label, kDebugLabelPrefix, fiber::Fiber::kDebugLabelMaxLen_);
    ConvertToStr(mesh.internal_id, debug_label + kDebugLabelPrefixLen,
                 fiber::Fiber::kDebugLabelMaxLen_ - kDebugLabelPrefixLen);
#else
    schar* debug_label{nullptr};
#endif  // COMET_FIBER_DEBUG_LABEL

    scheduler.Kick(job::GenerateJobDescr(
        job::JobPriority::Normal, OnSkeletalGeneration, job_params,
        job::JobStackSize::Normal, guard.GetCounter(), debug_label));

    if (++job_count % kMaxConcurrentJobs_ == 0) {
      guard.Wait();
    }
  }

  guard.Wait();
  event::EventManager::Get().FireEvent<ModelLoadedEvent>(root_entity_id);
  return root_entity_id;
}

void ModelHandler::DestroyStatic(EntityId entity_id) const {
  auto& entity_manager{EntityManager::Get()};
  auto& geometry_manager{geometry::GeometryManager::Get()};
  auto& physics_manager{physics::PhysicsManager::Get()};

  auto* model_cmp{
      entity_manager.GetComponent<geometry::StaticModelComponent>(entity_id)};
  auto* transform_root_cmp{
      entity_manager.GetComponent<physics::TransformRootComponent>(entity_id)};
  auto* transform_cmp{
      entity_manager.GetComponent<physics::TransformComponent>(entity_id)};

  if (model_cmp != nullptr) {
    geometry_manager.DestroyStaticModelComponent(model_cmp);
  }

  if (transform_root_cmp != nullptr) {
    physics_manager.DestroyTransformRootComponent(transform_root_cmp);
  }

  if (transform_cmp != nullptr) {
    physics_manager.DestroyTransformComponent(transform_cmp);
  }

  DestroyStaticChildren(entity_id);
  entity_manager.Destroy(entity_id);
}

void ModelHandler::DestroySkeletal(EntityId entity_id) const {
  auto& entity_manager{EntityManager::Get()};
  auto& geometry_manager{geometry::GeometryManager::Get()};
  auto& animation_manager{animation::AnimationManager::Get()};
  auto& physics_manager{physics::PhysicsManager::Get()};

  auto* model_cmp{
      entity_manager.GetComponent<geometry::SkeletalModelComponent>(entity_id)};
  auto* transform_root_cmp{
      entity_manager.GetComponent<physics::TransformRootComponent>(entity_id)};
  auto* transform_cmp{
      entity_manager.GetComponent<physics::TransformComponent>(entity_id)};
  auto* skeleton_cmp{
      entity_manager.GetComponent<geometry::SkeletonComponent>(entity_id)};
  auto* animation_cmp{
      entity_manager.GetComponent<animation::AnimationComponent>(entity_id)};

  if (model_cmp != nullptr) {
    geometry_manager.DestroySkeletalModelComponent(model_cmp);
  }

  if (transform_root_cmp != nullptr) {
    physics_manager.DestroyTransformRootComponent(transform_root_cmp);
  }

  if (transform_cmp != nullptr) {
    physics_manager.DestroyTransformComponent(transform_cmp);
  }

  if (skeleton_cmp != nullptr) {
    geometry_manager.DestroySkeletonComponent(skeleton_cmp);
  }

  if (animation_cmp != nullptr) {
    animation_manager.DestroyAnimationComponent(animation_cmp);
  }

  DestroySkeletalChildren(entity_id);
  entity_manager.Destroy(entity_id);
}

void ModelHandler::DestroyStaticChildren(EntityId current_entity_id) const {
  auto& entity_manager{EntityManager::Get()};
  auto& geometry_manager{geometry::GeometryManager::Get()};
  auto& physics_manager{physics::PhysicsManager::Get()};
  auto* packet{frame::FrameManager::Get().GetLogicFramePacket()};

  EntityManager::Get().EachChild<>(
      [&](auto child_entity_id) {
        auto* mesh_cmp{entity_manager.GetComponent<geometry::MeshComponent>(
            child_entity_id)};
        auto* transform_cmp{
            entity_manager.GetComponent<physics::TransformComponent>(
                child_entity_id)};

        if (mesh_cmp != nullptr) {
          if (mesh_cmp->mesh != nullptr) {
            packet->RegisterRemovedGeometry(
                child_entity_id, mesh_cmp->model_entity_id, mesh_cmp->mesh->id);
          }

          geometry_manager.DestroyStaticMeshComponent(mesh_cmp);
        }

        if (transform_cmp != nullptr) {
          physics_manager.DestroyTransformComponent(transform_cmp);
        }
      },
      current_entity_id);
}

void ModelHandler::DestroySkeletalChildren(EntityId current_entity_id) const {
  auto& entity_manager{EntityManager::Get()};
  auto& geometry_manager{geometry::GeometryManager::Get()};
  auto& physics_manager{physics::PhysicsManager::Get()};
  auto* packet{frame::FrameManager::Get().GetLogicFramePacket()};

  EntityManager::Get().EachChild<>(
      [&](auto child_entity_id) {
        auto* mesh_cmp{entity_manager.GetComponent<geometry::MeshComponent>(
            child_entity_id)};
        auto* transform_cmp{
            entity_manager.GetComponent<physics::TransformComponent>(
                child_entity_id)};

        if (mesh_cmp != nullptr) {
          if (mesh_cmp->mesh != nullptr) {
            packet->RegisterRemovedGeometry(
                child_entity_id, mesh_cmp->model_entity_id, mesh_cmp->mesh->id);
          }

          geometry_manager.DestroySkinnedMeshComponent(mesh_cmp);
        }

        if (transform_cmp != nullptr) {
          physics_manager.DestroyTransformComponent(transform_cmp);
        }
      },
      current_entity_id);
}

void ModelHandler::OnStaticGeneration(job::JobParamsHandle params_handle) {
  auto* params{
      static_cast<internal::StaticGenerationJobParams*>(params_handle)};

  COMET_ASSERT(params->id != kInvalidEntityId, "Invalid entity ID provided!");
  COMET_ASSERT(params->parent_id != kInvalidEntityId,
               "Invalid parent entity ID provided!");

  auto* mesh{params->mesh};

  geometry::MeshComponent mesh_cmp{
      geometry::GeometryManager::Get().GenerateStaticMeshComponent(
          mesh, params->id, params->root_entity_id, params->life_span)};

  physics::TransformComponent transform_cmp{};
  transform_cmp.root_entity_id = params->root_entity_id;
  transform_cmp.parent_entity_id = params->parent_id;
  transform_cmp.local = mesh->transform;

  EntityManager::Get().AddComponents(params->id, mesh_cmp, transform_cmp);
  EntityManager::Get().AddParent(params->id, transform_cmp.parent_entity_id);

  frame::FrameManager::Get().GetLogicFramePacket()->RegisterNewGeometry(
      params->id, &mesh_cmp, &transform_cmp);
}

void ModelHandler::OnSkeletalGeneration(job::JobParamsHandle params_handle) {
  auto* params{
      static_cast<internal::SkeletalGenerationJobParams*>(params_handle)};

  COMET_ASSERT(params->id != kInvalidEntityId, "Invalid entity ID provided!");
  COMET_ASSERT(params->parent_id != kInvalidEntityId,
               "Invalid parent entity ID provided!");

  auto* mesh{params->mesh};

  geometry::MeshComponent mesh_cmp{
      geometry::GeometryManager::Get().GenerateSkinnedMeshComponent(
          mesh, params->id, params->root_entity_id, params->life_span)};

  physics::TransformComponent transform_cmp{};
  transform_cmp.root_entity_id = params->root_entity_id;
  transform_cmp.parent_entity_id = params->parent_id;
  transform_cmp.local = mesh->transform;

  EntityManager::Get().AddComponents(params->id, mesh_cmp, transform_cmp);
  EntityManager::Get().AddParent(params->id, transform_cmp.parent_entity_id);

  frame::FrameManager::Get().GetLogicFramePacket()->RegisterNewGeometry(
      params->id, &mesh_cmp, &transform_cmp);
}
}  // namespace entity
}  // namespace comet