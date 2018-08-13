// Copyright 2018 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef KOMA_CORE_RESOURCE_LOCATOR_HPP_
#define KOMA_CORE_RESOURCE_LOCATOR_HPP_

#include <memory>

#include "../game.hpp"
#include "../game_object/camera/camera.hpp"
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
  static std::shared_ptr<Camera> main_camera();

  static void rendering_manager(RenderingManager *);
  static void input_manager(InputManager *);
  static void time_manager(TimeManager *);
  static void game_object_manager(GameObjectManager *);
  static void main_camera(std::shared_ptr<Camera>);

 private:
  static Game *game_;
  static RenderingManager *rendering_manager_;
  static InputManager *input_manager_;
  static NullInputManager null_input_manager_;
  static TimeManager *time_manager_;
  static GameObjectManager *game_object_manager_;
  static std::shared_ptr<Camera> main_camera_;
};
};  // namespace koma

#endif  // KOMA_CORE_RESOURCE_LOCATOR_HPP_
