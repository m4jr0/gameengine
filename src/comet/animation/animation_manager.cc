// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "animation_manager.h"

#include "comet/animation/component/animation_component.h"
#include "comet/core/concurrency/fiber/fiber.h"
#include "comet/core/concurrency/job/job_utils.h"
#include "comet/core/concurrency/job/scheduler.h"
#include "comet/core/frame/frame_allocator.h"
#include "comet/entity/entity_manager.h"
#include "comet/geometry/component/mesh_component.h"
#include "comet/resource/animation_resource.h"
#include "comet/resource/resource_manager.h"

namespace comet {
namespace animation {
AnimationManager& AnimationManager::Get() {
  static AnimationManager singleton{};
  return singleton;
}

void AnimationManager::Update([[maybe_unused]] frame::FramePacket* packet) {
  auto& scheduler{job::Scheduler::Get()};
  auto* counter{scheduler.GenerateCounter()};
  auto& entity_manager{entity::EntityManager::Get()};

  entity_manager.Each<AnimationComponent>([&](auto entity_id) {
#ifdef COMET_FIBER_DEBUG_LABEL
    schar debug_label[fiber::Fiber::kDebugLabelMaxLen_ + 1];
    auto anim_entity_id_len{GetLength("anim_entity_id_")};
    Copy(debug_label, "anim_entity_id_", anim_entity_id_len);
    ConvertToStr(entity_id, debug_label + anim_entity_id_len,
                 fiber::Fiber::kDebugLabelMaxLen_ - anim_entity_id_len);
#else
    const schar* debug_label{nullptr};
#endif  // COMET_FIBER_DEBUG_LABEL

    auto* animation_cmp{
        entity_manager.GetComponent<AnimationComponent>(entity_id)};

    if (animation_cmp->clip == nullptr) {
      return;
    }

    const auto params_handle{reinterpret_cast<job::JobParamsHandle>(
        COMET_FRAME_ALLOC_ONE_AND_POPULATE(entity::EntityId, entity_id))};

    scheduler.Kick(job::GenerateJobDescr(
        job::JobPriority::High, OnAssetProcessing, params_handle,
        job::JobStackSize::Normal, counter, debug_label));
  });

  scheduler.Wait(counter);
  scheduler.DestroyCounter(counter);
}

void AnimationManager::Play(entity::EntityId entity_id,
                            AnimationId animation_id) {
  const auto* anim_resource{
      resource::ResourceManager::Get().Load<resource::AnimationClipResource>(
          static_cast<resource::ResourceId>(animation_id))};

  if (anim_resource == nullptr) {
    return;
  }

  auto* animation{
      entity::EntityManager::Get().GetComponent<AnimationComponent>(entity_id)};
  COMET_ASSERT(animation != nullptr, "No animation component for entity #",
               entity_id, "!");
  animation->clip = &anim_resource->clip;
}

void AnimationManager::OnAssetProcessing(job::JobParamsHandle params_handle) {
  auto& entity_manager{entity::EntityManager::Get()};
  auto entity_id{*static_cast<entity::EntityId*>(params_handle)};

  auto* animation_cmp{
      entity_manager.GetComponent<AnimationComponent>(entity_id)};

  auto pose{DecompressClipAndExtractPose(*animation_cmp->clip, 1.0f)};
  // BlendPoses -> TBA.
  // GenerateGlobalPose -> TBA.
  // PostProcessPose -> TBA.
  // RecalculateGlobalPose -> TBA.
  // GenerateMatrixPalette -> TBA.
}
}  // namespace animation
}  // namespace comet
