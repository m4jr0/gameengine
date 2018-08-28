// Copyright 2018 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef KOMA_CORE_GAME_HPP_
#define KOMA_CORE_GAME_HPP_

#define LOGGER_KOMA_CORE_GAME "koma_core_game"

#include "game_object/game_object_manager.hpp"
#include "input/input_manager.hpp"
#include "physics/physics_manager.hpp"
#include "render/render_manager.hpp"
#include "time/time_manager.hpp"

namespace koma {
class Locator;

class Game {
 public:
  static constexpr double kMsPerUpdate_ = 16.66;  // 60 Hz refresh.

  Game();
  virtual ~Game();

  void Initialize();
  void Run();
  void Stop();
  void Quit();

  const bool is_running() const noexcept;

 protected:
  InputManager input_manager_;
  PhysicsManager physics_manager_;
  RenderManager render_manager_;
  GameObjectManager game_object_manager_;
  TimeManager time_manager_;

 private:
  bool is_running_ = false;
};
};  // namespace koma

#endif  // KOMA_CORE_GAME_HPP_
