// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/runtime/logic/game_logic_manager.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/animation/animation_manager.h"
#include "comet/core/concurrency/job/job.h"
#include "comet/core/concurrency/job/job_utils.h"
#include "comet/core/concurrency/job/scheduler.h"
#include "comet/core/continuation/continuation_manager.h"
#include "comet/core/continuation/type/continuation_phase.h"
#include "comet/environment/environment_manager.h"
#include "comet/event/event_manager.h"
#include "comet/physics/physics_manager.h"
#include "comet/profiler/profiler.h"
#include "comet/rendering/camera_manager.h"
#include "comet/rendering/light_manager.h"
#include "comet/rendering/rendering_manager.h"
#include "comet/scene/scene_manager.h"

namespace comet {
GameLogicManager& GameLogicManager::Get() {
  static GameLogicManager singleton{};
  return singleton;
}

void GameLogicManager::Update(frame::FramePacket* packet) {
  COMET_PROFILE("GameLogicManager::Update");
  COMET_ASSERT(packet != nullptr, "GameLogicManager::Update",
               "frame packet is null");

  ContinuationManager::Get().Poll(ContinuationPhase::BeforeLogic);
  PopulatePacket(packet);
  event::EventManager::Get().FireAllEvents();

  job::Scheduler::Get().Kick(job::GenerateJobDescr(
      job::JobPriority::High,
      [](job::JobParamsHandle params_handle) {
        COMET_PROFILE("GameLogicManager::Update::Job");
        auto* packet{reinterpret_cast<frame::FramePacket*>(params_handle)};

        scene::SceneManager::Get().Update();
        rendering::LightManager::Get().Update(packet);
        environment::EnvironmentManager::Get().Update(packet);
        physics::PhysicsManager::Get().Update(packet);

        packet->interpolation =
            packet->lag / time::TimeManager::Get().GetFixedDeltaTime();

        animation::AnimationManager::Get().Update(packet);
        ContinuationManager::Get().Poll(ContinuationPhase::AfterLogic);
      },
      packet, job::JobStackSize::Large, packet->counter, "game_logic_update"));
}

void GameLogicManager::PopulatePacket(frame::FramePacket* packet) {
  const auto extent{rendering::RenderingManager::Get().GetWindowExtent()};

  packet->main_camera_view_index =
      rendering::CameraManager::Get().PopulateCameraViews(
          *packet->camera_views, extent.width, extent.height);

#ifdef COMET_DEBUG
  packet->has_debug_camera = packet->camera_views->GetSize() > 1;
#endif  // COMET_DEBUG

  const auto expected_entity_count{
      scene::SceneManager::Get().GetExpectedEntityCount()};

  packet->added_geometries->Reserve(expected_entity_count);
  packet->dirty_meshes->Reserve(expected_entity_count);
  packet->dirty_transforms->Reserve(expected_entity_count);
  packet->removed_geometries->Reserve(expected_entity_count);

  packet->is_populated = true;
}
}  // namespace comet
