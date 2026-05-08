// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "entity_model_handler.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/animation/animation_manager.h"
#include "comet/animation/component/animation_component.h"
#include "comet/entity/entity_changes_fence.h"
#include "comet/entity/entity_event.h"
#include "comet/event/event_manager.h"
#include "comet/profiler/profiler.h"
#include "comet/resource/resource_manager.h"

namespace comet {
namespace entity {
namespace {
template <typename MeshComponentType>
void RegisterGeneratedGeometry(EntityId root_entity_id) {
  auto& entity_manager{EntityManager::Get()};
  auto* packet{frame::FrameManager::Get().GetLogicFramePacket()};

  COMET_ASSERT(packet != nullptr, "entity::RegisterGeneratedGeometry",
               "logic frame packet is null", "root_entity_id", root_entity_id);

  if (!entity_manager.IsEntity(root_entity_id)) {
    return;
  }

  entity_manager.ForEachChild<MeshComponentType, physics::TransformComponent>(
      [&](EntityId entity_id, MeshComponentType& mesh_cmp,
          physics::TransformComponent& transform_cmp) {
        packet->RegisterNewGeometry(entity_id, &mesh_cmp, &transform_cmp);
      },
      root_entity_id);
}

void FireModelLoaded(EntityId root_entity_id) {
  if (!EntityManager::Get().IsEntity(root_entity_id)) {
    return;
  }

  event::EventManager::Get().FireEvent<ModelLoadedEvent>(root_entity_id);
}
}  // namespace

EntityId ModelHandler::GenerateStatic(
    CTStringView model_path, resource::ResourceLifeSpan life_span) const {
  COMET_PROFILE("ModelHandler::GenerateStatic");

  auto& entity_manager{EntityManager::Get()};
  auto& geometry_manager{geometry::GeometryManager::Get()};
  auto& physics_manager{physics::PhysicsManager::Get()};

  const auto root_entity_id{entity_manager.Generate()};

  auto model_cmp{geometry_manager.GenerateStaticModelComponent(
      root_entity_id, model_path, life_span)};

  if (!model_cmp.resource_handle) {
    entity_manager.Destroy(root_entity_id);
    return kInvalidEntityId;
  }

  const auto* model_resource{
      resource::ResourceManager::Get().GetStaticModels()->Get(
          model_cmp.resource_handle)};

  if (model_resource == nullptr) {
    geometry_manager.DestroyStaticModelComponent(&model_cmp);
    entity_manager.Destroy(root_entity_id);
    return kInvalidEntityId;
  }

  entity_manager.AddComponents(
      root_entity_id,
      physics_manager.GenerateTransformComponent(root_entity_id), model_cmp,
      physics_manager.GenerateTransformRootComponent());

  GenerateMeshChildren<resource::StaticMeshResource,
                       internal::StaticGenerationJobParams>(
      root_entity_id, model_resource->meshes, life_span,
      static_job_params_allocator_, OnStaticGeneration, "mesh_sta_");

  AfterEntityChanges([root_entity_id] {
    RegisterGeneratedGeometry<geometry::MeshComponent>(root_entity_id);
    FireModelLoaded(root_entity_id);
  });

  return root_entity_id;
}

EntityId ModelHandler::GenerateSkeletal(
    CTStringView model_path, resource::ResourceLifeSpan life_span) const {
  COMET_PROFILE("ModelHandler::GenerateSkeletal");

  auto& entity_manager{EntityManager::Get()};
  auto& geometry_manager{geometry::GeometryManager::Get()};
  auto& physics_manager{physics::PhysicsManager::Get()};

  const auto root_entity_id{entity_manager.Generate()};

  auto model_cmp{geometry_manager.GenerateSkeletalModelComponent(
      root_entity_id, model_path, life_span)};

  if (!model_cmp.resource_handle) {
    entity_manager.Destroy(root_entity_id);
    return kInvalidEntityId;
  }

  const auto* model_resource{
      resource::ResourceManager::Get().GetSkeletalModels()->Get(
          model_cmp.resource_handle)};

  if (model_resource == nullptr) {
    geometry_manager.DestroySkeletalModelComponent(&model_cmp);
    entity_manager.Destroy(root_entity_id);
    return kInvalidEntityId;
  }

  auto skeleton_cmp{
      geometry_manager.GenerateSkeletonComponent(model_path, life_span)};

  if (!skeleton_cmp.resource_handle) {
    geometry_manager.DestroySkeletalModelComponent(&model_cmp);
    entity_manager.Destroy(root_entity_id);
    return kInvalidEntityId;
  }

  const auto* skeleton_resource{
      resource::ResourceManager::Get().GetSkeletons()->Get(
          skeleton_cmp.resource_handle)};

  if (skeleton_resource == nullptr) {
    geometry_manager.DestroySkeletonComponent(&skeleton_cmp);
    geometry_manager.DestroySkeletalModelComponent(&model_cmp);
    entity_manager.Destroy(root_entity_id);
    return kInvalidEntityId;
  }

  entity_manager.AddComponents(
      root_entity_id,
      physics_manager.GenerateTransformComponent(root_entity_id), skeleton_cmp,
      model_cmp, physics_manager.GenerateTransformRootComponent(),
      animation::AnimationManager::Get().GenerateAnimationComponent(
          animation::AnimationClipId::Invalid(), 1.0f, std::nullopt,
          life_span));

  GenerateMeshChildren<resource::SkinnedMeshResource,
                       internal::SkeletalGenerationJobParams>(
      root_entity_id, model_resource->meshes, life_span,
      skeletal_job_params_allocator_, OnSkeletalGeneration, "mesh_ske_");

  AfterEntityChanges([root_entity_id] {
    RegisterGeneratedGeometry<geometry::MeshComponent>(root_entity_id);
    FireModelLoaded(root_entity_id);
  });

  return root_entity_id;
}

void ModelHandler::DestroyStatic(EntityId entity_id) const {
  AfterEntityChanges([this, entity_id] { DestroyStaticNow(entity_id); });
}

void ModelHandler::DestroySkeletal(EntityId entity_id) const {
  AfterEntityChanges([this, entity_id] { DestroySkeletalNow(entity_id); });
}

void ModelHandler::DestroyStaticImmediate(EntityId entity_id) const {
  DestroyStaticNow(entity_id);
}

void ModelHandler::DestroySkeletalImmediate(EntityId entity_id) const {
  DestroySkeletalNow(entity_id);
}

void ModelHandler::OnInitialize() {
  static_job_params_allocator_.Initialize();
  skeletal_job_params_allocator_.Initialize();
}

void ModelHandler::OnShutdown() {
  skeletal_job_params_allocator_.Destroy();
  static_job_params_allocator_.Destroy();
}

void ModelHandler::DestroyStaticNow(EntityId entity_id) const {
  if (!EntityManager::Get().IsEntity(entity_id)) {
    return;
  }

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

void ModelHandler::DestroySkeletalNow(EntityId entity_id) const {
  if (!EntityManager::Get().IsEntity(entity_id)) {
    return;
  }

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

void ModelHandler::DestroyStaticChildren(EntityId entity_id) const {
  DestroyChildren(entity_id, [](auto& geometry_manager, auto* mesh_cmp) {
    geometry_manager.DestroyStaticMeshComponent(mesh_cmp);
  });
}

void ModelHandler::DestroySkeletalChildren(EntityId entity_id) const {
  DestroyChildren(entity_id, [](auto& geometry_manager, auto* mesh_cmp) {
    geometry_manager.DestroySkinnedMeshComponent(mesh_cmp);
  });
}

void ModelHandler::OnStaticGeneration(job::JobParamsHandle params_handle) {
  auto* params{
      static_cast<internal::StaticGenerationJobParams*>(params_handle)};

  COMET_ASSERT(params->id != kInvalidEntityId,
               "entity::ModelHandler::OnStaticGeneration", "invalid entity id",
               "entity_id", params->id);

  COMET_ASSERT(params->parent_id != kInvalidEntityId,
               "entity::ModelHandler::OnStaticGeneration",
               "invalid parent entity id", "parent_id", params->parent_id);

  COMET_ASSERT(params->mesh != nullptr,
               "entity::ModelHandler::OnStaticGeneration",
               "static mesh resource is null");

  const auto& mesh{*params->mesh};

  const auto mesh_cmp{
      geometry::GeometryManager::Get().GenerateStaticMeshComponent(
          mesh, params->id, params->root_entity_id)};

  physics::TransformComponent transform_cmp{};
  transform_cmp.root_entity_id = params->root_entity_id;
  transform_cmp.parent_entity_id = params->parent_id;
  transform_cmp.local = mesh.transform;

  EntityManager::Get().AddChildComponents(
      params->id, transform_cmp.parent_entity_id, mesh_cmp, transform_cmp);
}

void ModelHandler::OnSkeletalGeneration(job::JobParamsHandle params_handle) {
  auto* params{
      static_cast<internal::SkeletalGenerationJobParams*>(params_handle)};

  COMET_ASSERT(params->id != kInvalidEntityId,
               "entity::ModelHandler::OnSkeletalGeneration",
               "invalid entity id", "entity_id", params->id);

  COMET_ASSERT(params->parent_id != kInvalidEntityId,
               "entity::ModelHandler::OnSkeletalGeneration",
               "invalid parent entity id", "parent_id", params->parent_id);

  COMET_ASSERT(params->mesh != nullptr,
               "entity::ModelHandler::OnSkeletalGeneration",
               "skinned mesh resource is null");

  const auto& mesh{*params->mesh};

  const auto mesh_cmp{
      geometry::GeometryManager::Get().GenerateSkinnedMeshComponent(
          mesh, params->id, params->root_entity_id)};

  physics::TransformComponent transform_cmp{};
  transform_cmp.root_entity_id = params->root_entity_id;
  transform_cmp.parent_entity_id = params->parent_id;
  transform_cmp.local = mesh.transform;

  EntityManager::Get().AddChildComponents(
      params->id, transform_cmp.parent_entity_id, mesh_cmp, transform_cmp);
}
}  // namespace entity
}  // namespace comet