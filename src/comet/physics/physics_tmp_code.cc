// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "physics_tmp_code.h"

#include <atomic>

#include "comet/core/concurrency/fiber/fiber_utils.h"
#include "comet/core/memory/memory_utils.h"
#include "comet/entity/entity_manager.h"
#include "comet/entity/factory/entity_factory_manager.h"
#include "comet/physics/component/transform_component.h"

namespace comet {
namespace physics {
void LoadTmp() {
  static std::atomic<bool> is_loaded{false};

  if (is_loaded.load(std::memory_order_acquire)) {
    return;
  }

  static entity::EntityId character_id;
  static entity::EntityId sponza_id;

  character_id =
      entity::EntityFactoryManager::Get().GetModel()->GenerateSkeletal(
          COMET_CTSTRING_VIEW("models/kate/kate.fbx"));

  sponza_id = entity::EntityFactoryManager::Get().GetModel()->GenerateStatic(
      COMET_CTSTRING_VIEW("models/sponza/sponza.obj"));

  auto job_descr{job::GenerateJobDescr(
      job::JobPriority::Normal,
      [](job::JobParamsHandle) {
        fiber::WaitForEntityUpdates();

        auto* character_transform{
            entity::EntityManager::Get()
                .GetComponent<physics::TransformComponent>(character_id)};

        constexpr auto kCharacterScaleFactor{0.15f};

        character_transform->local[0][0] *= kCharacterScaleFactor;
        character_transform->local[1][1] *= kCharacterScaleFactor;
        character_transform->local[2][2] *= kCharacterScaleFactor;

        auto* sponza_transform{
            entity::EntityManager::Get()
                .GetComponent<physics::TransformComponent>(sponza_id)};

        constexpr auto kSponzaScaleFactor{0.17f};
        sponza_transform->local[0][0] *= kSponzaScaleFactor;
        sponza_transform->local[1][1] *= kSponzaScaleFactor;
        sponza_transform->local[2][2] *= kSponzaScaleFactor;
      },
      nullptr, job::JobStackSize::Normal, nullptr, "loading_entities")};

  auto& scheduler{job::Scheduler::Get()};
  scheduler.Kick(job_descr);
  is_loaded.store(true, std::memory_order_release);
}
}  // namespace physics
}  // namespace comet
