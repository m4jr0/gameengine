// Copyright 2021 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_CORE_GAME_HPP_
#define COMET_CORE_GAME_HPP_

constexpr auto kLoggerCometCoreGame = "comet_core_game";

#include <memory>

#include "game_object/camera/camera.hpp"
#include "game_object/game_object_manager.hpp"
#include "input/input_manager.hpp"
#include "physics/physics_manager.hpp"
#include "render/render_manager.hpp"
#include "resource/resource_manager.hpp"
#include "time/time_manager.hpp"

namespace comet {
class Game {
 public:
  static constexpr double kMsPerUpdate_ = 16.66;  // 60 Hz refresh.

  Game(const Game&) = delete;
  Game(Game&&) = delete;
  Game& operator=(const Game&) = delete;
  Game& operator=(Game&&) = delete;

  void Initialize();
  void Run();
  void Stop();
  void Destroy();
  void Quit();

  static Game* const game();
  ResourceManager* const resource_manager();
  RenderManager* const render_manager();
  InputManager* const input_manager();
  TimeManager* const time_manager();
  GameObjectManager* const game_object_manager();
  Camera* const main_camera();

  const bool is_running() const noexcept;

 protected:
  static std::unique_ptr<Game> game_;

  std::unique_ptr<ResourceManager> resource_manager_ = nullptr;
  std::unique_ptr<InputManager> input_manager_ = nullptr;
  std::unique_ptr<PhysicsManager> physics_manager_ = nullptr;
  std::unique_ptr<RenderManager> render_manager_ = nullptr;
  std::unique_ptr<GameObjectManager> game_object_manager_ = nullptr;
  std::unique_ptr<TimeManager> time_manager_ = nullptr;
  std::shared_ptr<Camera> main_camera_ = nullptr;

 private:
  Game();

  bool is_running_ = false;
  bool is_exit_requested_ = false;

  void Exit();
};
}  // namespace comet

#endif  // COMET_CORE_GAME_HPP_
