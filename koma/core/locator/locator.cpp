// Copyright 2021 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "locator.hpp"

#ifdef _WIN32
// Allow debugging memory leaks on Windows.
#include "debug_windows.hpp"
#endif  // _WIN32

namespace koma {
Game *Locator::game_ = nullptr;
ResourceManager *Locator::resource_manager_ = nullptr;
RenderManager *Locator::render_manager_ = nullptr;
InputManager *Locator::input_manager_ = nullptr;
NullInputManager Locator::null_input_manager_ = NullInputManager();
TimeManager *Locator::time_manager_ = nullptr;
GameObjectManager *Locator::game_object_manager_ = nullptr;
std::shared_ptr<Camera> Locator::main_camera_ = nullptr;

void Locator::Initialize(Game *game) {
  Locator::game_ = game;
  Locator::input_manager_ = &null_input_manager_;
  Locator::time_manager_ = nullptr;
  Locator::game_object_manager_ = nullptr;
}

Game &Locator::game() { return *Locator::game_; };

ResourceManager &Locator::resource_manager() {
  return *Locator::resource_manager_;
}

RenderManager &Locator::render_manager() { return *Locator::render_manager_; }

InputManager &Locator::input_manager() { return *Locator::input_manager_; }

TimeManager &Locator::time_manager() { return *Locator::time_manager_; }

GameObjectManager &Locator::game_object_manager() {
  return *Locator::game_object_manager_;
}

std::shared_ptr<Camera> Locator::main_camera() { return Locator::main_camera_; }

void Locator::resource_manager(ResourceManager *resource_manager) {
  Locator::resource_manager_ = resource_manager;
}

void Locator::render_manager(RenderManager *render_manager) {
  Locator::render_manager_ = render_manager;
}

void Locator::input_manager(InputManager *input_manager) {
  if (input_manager == nullptr) {
    Locator::input_manager_ = &Locator::null_input_manager_;
  } else {
    Locator::input_manager_ = input_manager;
  }
}

void Locator::time_manager(TimeManager *time_manager) {
  Locator::time_manager_ = time_manager;
}

void Locator::game_object_manager(GameObjectManager *game_object_manager) {
  Locator::game_object_manager_ = game_object_manager;
}

void Locator::main_camera(std::shared_ptr<Camera> main_camera) {
  Locator::main_camera_ = main_camera;
}
}  // namespace koma
