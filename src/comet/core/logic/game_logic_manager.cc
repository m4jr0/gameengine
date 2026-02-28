// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "game_logic_manager.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/concurrency/job/job.h"
#include "comet/core/concurrency/job/job_utils.h"
#include "comet/core/concurrency/job/scheduler.h"
#include "comet/rendering/camera/camera.h"

namespace comet {
GameLogicManager& GameLogicManager::Get() {
  static GameLogicManager singleton{};
  return singleton;
}

void GameLogicManager::Initialize() {
  Manager::Initialize();
  physics_manager_ = &physics::PhysicsManager::Get();
  entity_manager_ = &entity::EntityManager::Get();
  animation_manager_ = &animation::AnimationManager::Get();
  camera_manager_ = &rendering::CameraManager::Get();
  scene_manager_ = &scene::SceneManager::Get();
  event_manager_ = &event::EventManager::Get();
}

void GameLogicManager::Shutdown() {
  physics_manager_ = nullptr;
  entity_manager_ = nullptr;
  animation_manager_ = nullptr;
  camera_manager_ = nullptr;
  scene_manager_ = nullptr;
  event_manager_ = nullptr;
  Manager::Shutdown();
}

void GameLogicManager::Update(frame::FramePacket* packet) {
  PopulatePacket(packet);
  event_manager_->FireAllEvents();

  struct Job {
    frame::FramePacket* packet{nullptr};
    physics::PhysicsManager* physics_manager{nullptr};
    entity::EntityManager* entity_manager{nullptr};
    animation::AnimationManager* animation_manager{nullptr};
  };

  auto* job{COMET_FRAME_ALLOC_ONE_AND_POPULATE(Job)};
  job->packet = packet;
  job->physics_manager = physics_manager_;
  job->entity_manager = entity_manager_;
  job->animation_manager = animation_manager_;

  job::Scheduler::Get().Kick(job::GenerateJobDescr(
      job::JobPriority::High,
      [](job::JobParamsHandle params_handle) {
        auto* job{reinterpret_cast<Job*>(params_handle)};
        auto* packet{job->packet};

        job->physics_manager->Update(packet);

        packet->interpolation =
            packet->lag / time::TimeManager::Get().GetFixedDeltaTime();

        job->animation_manager->Update(packet);
      },
      job, job::JobStackSize::Large, packet->counter, "game_logic_update"));
}

void GameLogicManager::PopulatePacket(frame::FramePacket* packet) {
  auto* camera{camera_manager_->GetMainCamera()};
  packet->projection_matrix = camera->GetProjectionMatrix();
  packet->view_matrix = camera->GetViewMatrix();
  auto expected_entity_count{scene_manager_->GetExpectedEntityCount()};

  packet->added_geometries->Reserve(expected_entity_count);
  packet->dirty_meshes->Reserve(expected_entity_count);
  packet->dirty_transforms->Reserve(expected_entity_count);
  packet->removed_geometries->Reserve(expected_entity_count);
}
}  // namespace comet
