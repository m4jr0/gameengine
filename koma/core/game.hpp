// Copyright 2018 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef KOMA_CORE_GAME_HPP_
#define KOMA_CORE_GAME_HPP_

#include <iostream>
#include <string>

#include "game_object_manager.hpp"
#include "input_manager.hpp"
#include "locator.hpp"
#include "physics_manager.hpp"
#include "rendering_manager.hpp"
#include "time_manager.hpp"

namespace koma {
class Game {
 public:
  const double MS_PER_UPDATE = 16.66;  // 60 Hz refresh.

  Game();
  virtual ~Game();

  void Run();
  void Stop();
  void Quit();

  const bool is_running() const { return this->is_running_; };

 protected:
  PhysicsManager physics_manager_;
  RenderingManager rendering_manager_;
  GameObjectManager game_object_manager_;
  TimeManager time_manager_;

  void ResetCounters();

 private:
  bool is_running_ = false;
};
};  // namespace koma

#endif  // KOMA_CORE_GAME_HPP_
