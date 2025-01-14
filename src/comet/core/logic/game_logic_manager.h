// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_LOGIC_GAME_LOGIC_MANAGER_H_
#define COMET_COMET_CORE_LOGIC_GAME_LOGIC_MANAGER_H_

#include "comet/animation/animation_manager.h"
#include "comet/core/essentials.h"
#include "comet/core/frame/frame_packet.h"
#include "comet/core/manager.h"
#include "comet/entity/entity_manager.h"
#include "comet/event/event_manager.h"
#include "comet/physics/physics_manager.h"
#include "comet/rendering/camera/camera_manager.h"
#include "comet/scene/scene_manager.h"

namespace comet {
class GameLogicManager : public Manager {
 public:
  static GameLogicManager& Get();

  GameLogicManager() = default;
  GameLogicManager(const GameLogicManager&) = delete;
  GameLogicManager(GameLogicManager&&) = delete;
  GameLogicManager& operator=(const GameLogicManager&) = delete;
  GameLogicManager& operator=(GameLogicManager&&) = delete;
  virtual ~GameLogicManager() = default;

  void Initialize() override;
  void Shutdown() override;

  void Update(frame::FramePacket* packet);

 private:
  void PopulatePacket(frame::FramePacket* packet);

  physics::PhysicsManager* physics_manager_{nullptr};
  entity::EntityManager* entity_manager_{nullptr};
  animation::AnimationManager* animation_manager_{nullptr};
  rendering::CameraManager* camera_manager_{nullptr};
  scene::SceneManager* scene_manager_{nullptr};
  event::EventManager* event_manager_{nullptr};
};
}  // namespace comet

#endif  // COMET_COMET_CORE_LOGIC_GAME_LOGIC_MANAGER_H_
