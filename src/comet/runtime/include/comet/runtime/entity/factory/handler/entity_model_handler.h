// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RUNTIME_ENTITY_FACTORY_HANDLER_ENTITY_MODEL_HANDLER_H_
#define COMET_RUNTIME_ENTITY_FACTORY_HANDLER_ENTITY_MODEL_HANDLER_H_

#include "comet/core/container/map.h"
#include "comet/core/essentials.h"
#include "comet/core/job/job.h"
#include "comet/core/job/job_utils.h"
#include "comet/core/job/scheduler.h"
#include "comet/core/string/c_string.h"
#include "comet/core/string/tstring.h"
#include "comet/data/resource/common.h"
#include "comet/data/resource/model/model_resource.h"
#include "comet/runtime/entity/entity_id.h"
#include "comet/runtime/entity/entity_manager.h"
#include "comet/runtime/entity/factory/handler/entity_handler.h"
#include "comet/runtime/frame/frame_manager.h"
#include "comet/runtime/frame/frame_packet.h"
#include "comet/runtime/geometry/component/mesh_component.h"
#include "comet/runtime/geometry/geometry_manager.h"
#include "comet/runtime/geometry/mesh.h"
#include "comet/runtime/memory/allocator/free_list_allocator.h"
#include "comet/runtime/memory/allocator/platform_allocator.h"
#include "comet/runtime/physics/physics_manager.h"
#include "comet/runtime/transform/component/transform_component.h"

namespace comet {
namespace entity {
namespace internal {
using ParentEntityIds = Map<resource::RawResourceId, EntityId>;

struct StaticGenerationJobParams {
  resource::ResourceLifeSpan life_span{resource::ResourceLifeSpan::Manual};
  EntityId id{kInvalidEntityId};
  EntityId root_entity_id{kInvalidEntityId};
  EntityId parent_id{kInvalidEntityId};
  const resource::StaticMeshResource* mesh{nullptr};
};

struct SkeletalGenerationJobParams {
  resource::ResourceLifeSpan life_span{resource::ResourceLifeSpan::Manual};
  EntityId id{kInvalidEntityId};
  EntityId root_entity_id{kInvalidEntityId};
  EntityId parent_id{kInvalidEntityId};
  const resource::SkinnedMeshResource* mesh{nullptr};
};
}  // namespace internal

class ModelHandler : public Handler {
 public:
  ModelHandler() = default;
  ModelHandler(const ModelHandler&) = delete;
  ModelHandler(ModelHandler&&) = delete;
  ModelHandler& operator=(const ModelHandler&) = delete;
  ModelHandler& operator=(ModelHandler&&) = delete;
  ~ModelHandler() override = default;

  EntityId GenerateStatic(CTStringView model_path,
                          resource::ResourceLifeSpan life_span =
                              resource::ResourceLifeSpan::Manual) const;

  EntityId GenerateSkeletal(CTStringView model_path,
                            resource::ResourceLifeSpan life_span =
                                resource::ResourceLifeSpan::Manual) const;

  void DestroyStatic(EntityId entity_id) const;
  void DestroySkeletal(EntityId entity_id) const;

  void DestroyStaticImmediate(EntityId entity_id) const;
  void DestroySkeletalImmediate(EntityId entity_id) const;

 protected:
  void OnInitialize() override;
  void OnShutdown() override;

 private:
  static constexpr inline usize kMaxConcurrentJobs_{10};
  static constexpr inline usize kGenerationScratchCapacity_{4096};

  mutable memory::PlatformAllocator scratch_allocator_{kEngineMemoryTagEntity};

  mutable memory::FiberFreeListAllocator static_job_params_allocator_{
      sizeof(internal::StaticGenerationJobParams), kGenerationScratchCapacity_,
      kEngineMemoryTagEntity};

  mutable memory::FiberFreeListAllocator skeletal_job_params_allocator_{
      sizeof(internal::SkeletalGenerationJobParams),
      kGenerationScratchCapacity_, kEngineMemoryTagEntity};

  void DestroyStaticNow(EntityId entity_id) const;
  void DestroySkeletalNow(EntityId entity_id) const;

  void DestroyStaticChildren(EntityId current_entity_id) const;
  void DestroySkeletalChildren(EntityId current_entity_id) const;

  static void OnStaticGeneration(job::JobParamsHandle params_handle);
  static void OnSkeletalGeneration(job::JobParamsHandle params_handle);

  template <typename Mesh, typename Params, typename Allocator, typename JobFn>
  void GenerateMeshChildren(
      EntityId root_entity_id, const Array<Mesh>& meshes,
      resource::ResourceLifeSpan life_span, Allocator& params_allocator,
      JobFn job_fn, [[maybe_unused]] const schar* debug_label_prefix) const {
    auto& entity_manager{EntityManager::Get()};
    auto& scheduler{job::Scheduler::Get()};

    auto entity_ids{internal::ParentEntityIds::WithCapacity(&scratch_allocator_,
                                                            meshes.GetSize())};

    for (const auto& mesh : meshes) {
      entity_ids.Set(mesh.internal_id, entity_manager.Generate());
    }

    auto params_to_free{
        Array<Params*>::WithCapacity(&scratch_allocator_, kMaxConcurrentJobs_)};

    job::CounterGuard guard{};
    usize job_count{0};

    auto flush_jobs{[&]() {
      guard.Wait();

      for (auto* param : params_to_free) {
        param->~Params();
        params_allocator.Deallocate(param);
      }

      params_to_free.Clear();
    }};

    for (const auto& mesh : meshes) {
      auto* params{params_allocator.template AllocateOneAndPopulate<Params>()};

      params->life_span = life_span;
      params->id = entity_ids.Get(mesh.internal_id);
      params->root_entity_id = root_entity_id;
      params->parent_id = mesh.parent_id == resource::kInvalidRawResourceId
                              ? root_entity_id
                              : entity_ids.Get(mesh.parent_id);
      params->mesh = &mesh;

#ifdef COMET_FIBER_DEBUG_LABEL
      schar debug_label[fiber::Fiber::kDebugLabelMaxLen_ + 1]{'\0'};
      constexpr auto max_len{fiber::Fiber::kDebugLabelMaxLen_};
      const auto prefix_len{GetLength(debug_label_prefix)};

      Copy(debug_label, debug_label_prefix, max_len);
      ConvertToStr(mesh.internal_id, debug_label + prefix_len,
                   max_len - prefix_len);
#else
      schar* debug_label{nullptr};
#endif  // COMET_FIBER_DEBUG_LABEL

      scheduler.Kick(job::GenerateJobDescr(job::JobPriority::Normal, job_fn,
                                           params, job::JobStackSize::Normal,
                                           guard.GetCounter(), debug_label));

      params_to_free.PushLast(params);

      if (++job_count % kMaxConcurrentJobs_ == 0) {
        flush_jobs();
      }
    }

    flush_jobs();
  }

  template <typename DestroyMeshComponentFn>
  void DestroyChildren(EntityId root_entity_id,
                       DestroyMeshComponentFn destroy_mesh) const {
    auto& entity_manager{EntityManager::Get()};
    auto& geometry_manager{geometry::GeometryManager::Get()};
    auto& physics_manager{physics::PhysicsManager::Get()};
    auto* packet{frame::FrameManager::Get().GetLogicFramePacket()};

    auto children{Array<EntityId>::WithCapacity(&scratch_allocator_, 64)};

    entity_manager.ForEachChild<geometry::MeshComponent>(
        [&](auto child_entity_id, geometry::MeshComponent& mesh_cmp) {
          if (mesh_cmp.mesh_handle) {
            const auto* mesh{
                static_cast<const geometry::GeometryManager&>(geometry_manager)
                    .TryGet(mesh_cmp.mesh_handle)};

            if (mesh != nullptr) {
              packet->RegisterRemovedGeometry(
                  child_entity_id, mesh_cmp.model_entity_id, mesh->handle);
            }
          }

          destroy_mesh(geometry_manager, &mesh_cmp);
          children.PushLast(child_entity_id);
        },
        root_entity_id);

    entity_manager.ForEachChild<physics::TransformComponent>(
        [&](auto, physics::TransformComponent& transform_cmp) {
          physics_manager.DestroyTransformComponent(&transform_cmp);
        },
        root_entity_id);

    for (auto child : children) {
      entity_manager.Destroy(child);
    }
  }
};
}  // namespace entity
}  // namespace comet

#endif  // COMET_RUNTIME_ENTITY_FACTORY_HANDLER_ENTITY_MODEL_HANDLER_H_