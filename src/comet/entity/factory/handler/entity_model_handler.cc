// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "comet_pch.h"

#include "entity_model_handler.h"

#include <utility>

#include "comet/core/c_string.h"
#include "comet/core/concurrency/fiber/fiber_context.h"
#include "comet/core/concurrency/job/job_utils.h"
#include "comet/core/concurrency/job/scheduler.h"
#include "comet/core/frame/frame_allocator.h"
#include "comet/core/type/tstring.h"
#include "comet/entity/entity_manager.h"
#include "comet/geometry/component/mesh_component.h"
#include "comet/geometry/geometry_manager.h"
#include "comet/physics/component/transform_component.h"
#include "comet/profiler/profiler.h"
#include "comet/resource/resource_manager.h"

namespace comet {
namespace entity {
EntityId ModelHandler::GenerateStatic(CTStringView model_path,
                                      frame::FramePacket* packet) const {
  COMET_PROFILE("ModelHandler::GenerateStatic");
  const auto* model{
      resource::ResourceManager::Get().Load<resource::StaticModelResource>(
          model_path)};

  if (model == nullptr) {
    return kInvalidEntityId;
  }

  auto& entity_manager{EntityManager::Get()};
  auto root_entity_id{entity_manager.Generate()};

  physics::TransformComponent transform_cmp{};
  transform_cmp.root_entity_id = root_entity_id;

  entity_manager.AddComponents(root_entity_id, transform_cmp,
                               physics::TransformRootComponent{});

  auto& scheduler{job::Scheduler::Get()};
  auto* counter{scheduler.GenerateCounter()};

#ifdef COMET_FIBER_DEBUG_LABEL
  constexpr auto* kDebugLabelPrefix{"mesh_sta_"};
  constexpr auto kDebugLabelPrefixLen{GetLength(kDebugLabelPrefix)};
#endif  // COMET_FIBER_DEBUG_LABEL

  auto entity_ids{internal::ParentEntityIds{&frame::GetFrameAllocator(),
                                            model->meshes.GetSize()}};

  for (const auto& mesh : model->meshes) {
    entity_ids[mesh.internal_id] = entity_manager.Generate();
  }

  usize job_count{0};

  for (const auto& mesh : model->meshes) {
    auto* job_params{COMET_FRAME_ALLOC_ONE_AND_POPULATE(
        internal::StaticGenerationJobParams)};
    job_params->id = entity_ids[mesh.internal_id];
    job_params->root_entity_id = root_entity_id;

    job_params->parent_id = mesh.parent_id == resource::kInvalidResourceId
                                ? root_entity_id
                                : entity_ids[mesh.parent_id];

    job_params->mesh = &mesh;
    job_params->packet = packet;

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
        job::JobStackSize::Normal, counter, debug_label));

    if (++job_count % kMaxConcurrentJobs_ == 0) {
      scheduler.Wait(counter);
    }
  }

  scheduler.Wait(counter);
  scheduler.DestroyCounter(counter);
  return root_entity_id;
}

EntityId ModelHandler::GenerateSkeletal(CTStringView model_path,
                                        frame::FramePacket* packet) const {
  COMET_PROFILE("ModelHandler::GenerateSkeletal");
  const auto* model{
      resource::ResourceManager::Get().Load<resource::SkeletalModelResource>(
          model_path)};

  if (model == nullptr) {
    return kInvalidEntityId;
  }

  auto& entity_manager{EntityManager::Get()};
  auto root_entity_id{entity_manager.Generate()};

  physics::TransformComponent transform_cmp{};
  transform_cmp.root_entity_id = root_entity_id;

  entity_manager.AddComponents(root_entity_id, transform_cmp,
                               physics::TransformRootComponent{});

  auto& scheduler{job::Scheduler::Get()};
  auto* counter{scheduler.GenerateCounter()};

#ifdef COMET_FIBER_DEBUG_LABEL
  constexpr auto* kDebugLabelPrefix{"mesh_ske_"};
  constexpr auto kDebugLabelPrefixLen{GetLength(kDebugLabelPrefix)};
#endif  // COMET_FIBER_DEBUG_LABEL

  auto entity_ids{internal::ParentEntityIds{&frame::GetFrameAllocator(),
                                            model->meshes.GetSize()}};

  for (const auto& mesh : model->meshes) {
    entity_ids[mesh.internal_id] = entity_manager.Generate();
  }

  usize job_count{0};

  for (const auto& mesh : model->meshes) {
    auto* job_params{COMET_FRAME_ALLOC_ONE_AND_POPULATE(
        internal::SkeletalGenerationJobParams)};
    job_params->id = entity_ids[mesh.internal_id];
    job_params->root_entity_id = root_entity_id;

    job_params->parent_id = mesh.parent_id == resource::kInvalidResourceId
                                ? root_entity_id
                                : entity_ids[mesh.parent_id];

    job_params->mesh = &mesh;
    job_params->packet = packet;

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
        job::JobStackSize::Normal, counter, debug_label));

    if (++job_count % kMaxConcurrentJobs_ == 0) {
      scheduler.Wait(counter);
    }
  }

  scheduler.Wait(counter);
  scheduler.DestroyCounter(counter);
  return root_entity_id;
}

void ModelHandler::OnStaticGeneration(job::JobParamsHandle params_handle) {
  auto* params{
      static_cast<internal::StaticGenerationJobParams*>(params_handle)};

  COMET_ASSERT(params->id != kInvalidEntityId, "Invalid entity ID provided!");
  COMET_ASSERT(params->parent_id != kInvalidEntityId,
               "Invalid parent entity ID provided!");

  auto* mesh{params->mesh};
  geometry::MeshComponent mesh_cmp{
      geometry::GeometryManager::Get().GenerateComponent(mesh)};

  physics::TransformComponent transform_cmp{};
  transform_cmp.root_entity_id = params->root_entity_id;
  transform_cmp.parent_entity_id = params->parent_id;
  transform_cmp.local = mesh->transform;

  EntityManager::Get().AddComponents(params->id, mesh_cmp, transform_cmp);
  EntityManager::Get().AddParent(params->id, transform_cmp.parent_entity_id);
  auto* packet{params->packet};

  if (packet == nullptr) {
    return;
  }

  packet->RegisterNewGeometry(params->id, &mesh_cmp, &transform_cmp);
}

void ModelHandler::OnSkeletalGeneration(job::JobParamsHandle params_handle) {
  auto* params{
      static_cast<internal::SkeletalGenerationJobParams*>(params_handle)};

  COMET_ASSERT(params->id != kInvalidEntityId, "Invalid entity ID provided!");
  COMET_ASSERT(params->parent_id != kInvalidEntityId,
               "Invalid parent entity ID provided!");

  auto* mesh{params->mesh};
  auto geometry_cmps{geometry::GeometryManager::Get().GenerateComponents(mesh)};

  geometry::MeshComponent mesh_cmp{std::move(geometry_cmps.mesh_cmp)};
  geometry::SkeletonComponent skeleton_cmp{
      std::move(geometry_cmps.skeleton_cmp)};

  physics::TransformComponent transform_cmp{};
  transform_cmp.root_entity_id = params->root_entity_id;
  transform_cmp.parent_entity_id = params->parent_id;
  transform_cmp.local = mesh->transform;

  EntityManager::Get().AddComponents(params->id, mesh_cmp, skeleton_cmp,
                                     transform_cmp);
  EntityManager::Get().AddParent(params->id, transform_cmp.parent_entity_id);
  auto* packet{params->packet};

  if (packet == nullptr) {
    return;
  }

  packet->RegisterNewGeometry(params->id, &mesh_cmp, &transform_cmp);
}
}  // namespace entity
}  // namespace comet