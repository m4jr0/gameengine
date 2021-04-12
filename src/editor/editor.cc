// Copyright 2021 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "editor.h"
#include "editor/ui/imgui_opengl_renderer.h"

#ifdef _WIN32
// Allow debugging memory leaks on Windows.
#include "debug_windows.h"
#endif  // _WIN32

namespace comet {
namespace editor {
CometEditor::CometEditor() {
  resource_manager_ = std::make_unique<resource::ResourceManager>();
  physics_manager_ = std::make_unique<physics::PhysicsManager>();
  rendering_manager_ = std::make_unique<ui::ImguiOpenGlRenderer>();
  game_object_manager_ = std::make_unique<game_object::GameObjectManager>();
  time_manager_ = std::make_unique<time::TimeManager>();
  input_manager_ = std::make_unique<input::InputManager>();
  event_manager_ = std::make_unique<event::EventManager>();

  Engine::engine_ = this;
}
}  // namespace editor

std::unique_ptr<core::Engine> core::CreateEngine() {
  return std::make_unique<editor::CometEditor>();
}
}  // namespace comet