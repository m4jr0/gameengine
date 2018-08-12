// Copyright 2018 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef KOMA_CORE_RESOURCE_LOCATOR_HPP_
#define KOMA_CORE_RESOURCE_LOCATOR_HPP_

#include "../game.hpp"
#include "../game_object/game_object_manager.hpp"
#include "../input/input_manager.hpp"
#include "../rendering/rendering_manager.hpp"
#include "../time/time_manager.hpp"

namespace koma {
class Locator {
 public:
  static void Initialize(Game *);

  static Game &game();
  static RenderingManager &rendering_manager();
  static InputManager &input_manager();
  static TimeManager &time_manager();
  static GameObjectManager &game_object_manager();

  static void rendering_manager(RenderingManager *rendering_manager);
  static void input_manager(InputManager *input_manager);
  static void time_manager(TimeManager *time_manager);
  static void game_object_manager(GameObjectManager *game_object_manager);

 private:
  static Game *game_;
  static RenderingManager *rendering_manager_;
  static InputManager *input_manager_;
  static NullInputManager null_input_manager_;
  static TimeManager *time_manager_;
  static GameObjectManager *game_object_manager_;
};
};  // namespace koma

#endif  // KOMA_CORE_RESOURCE_LOCATOR_HPP_
