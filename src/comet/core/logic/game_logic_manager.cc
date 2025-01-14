// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "comet_pch.h"

#include "game_logic_manager.h"

#include "comet/core/frame/frame_utils.h"
#include "comet/entity/entity_id.h"
#include "comet/geometry/component/mesh_component.h"
#include "comet/math/bounding_volume.h"
#include "comet/physics/component/transform_component.h"
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
  physics_manager_->Update(packet);
  entity_manager_->DispatchComponentChanges();

  packet->interpolation =
      packet->lag / time::TimeManager::Get().GetFixedDeltaTime();

  animation_manager_->Update(packet);
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
