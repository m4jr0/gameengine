// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "animation_manager.h"

#include "comet/core/concurrency/fiber/fiber.h"
#include "comet/core/concurrency/job/job_utils.h"
#include "comet/core/concurrency/job/scheduler.h"
#include "comet/entity/entity_manager.h"
#include "comet/geometry/component/mesh_component.h"

namespace comet {
namespace animation {
AnimationManager& AnimationManager::Get() {
  static AnimationManager singleton{};
  return singleton;
}

void AnimationManager::Update([[maybe_unused]] frame::FramePacket* packet) {
  auto& scheduler{job::Scheduler::Get()};
  auto* counter{scheduler.GenerateCounter()};

  entity::EntityManager::Get()
      .Each<geometry::MeshComponent, geometry::SkeletonComponent>(
          [&](auto entity_id) {
#ifdef COMET_FIBER_DEBUG_LABEL
            schar debug_label[fiber::Fiber::kDebugLabelMaxLen_ + 1];
            auto anim_entity_id_len{GetLength("anim_entity_id_")};
            Copy(debug_label, "anim_entity_id_", anim_entity_id_len);
            ConvertToStr(entity_id, debug_label + anim_entity_id_len,
                         fiber::Fiber::kDebugLabelMaxLen_ - anim_entity_id_len);
#else
            const schar* debug_label{nullptr};
#endif  // COMET_FIBER_DEBUG_LABEL

            job::GenerateJobDescr(job::JobPriority::High, OnAssetProcessing,
                                  &entity_id, job::JobStackSize::Normal,
                                  counter, debug_label);
          });

  scheduler.Wait(counter);
  scheduler.DestroyCounter(counter);
}

void AnimationManager::OnAssetProcessing(job::JobParamsHandle params_handle) {
  auto& entity_manager{entity::EntityManager::Get()};
  auto entity_id{*static_cast<entity::EntityId*>(params_handle)};

  [[maybe_unused]] auto* mesh_cmp{
      entity_manager.GetComponent<geometry::MeshComponent>(entity_id)};
  [[maybe_unused]] auto* skeleton_cmp{
      entity_manager.GetComponent<geometry::SkeletonComponent>(entity_id)};

  // TODO(m4jr0): Update geometry according to animations.
  mesh_cmp->mesh->is_dirty = true;
}
}  // namespace animation
}  // namespace comet
