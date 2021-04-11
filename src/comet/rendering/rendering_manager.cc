// Copyright 2021 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "rendering_manager.h"

#define GLFW_INCLUDE_NONE  // glad will include gl.

#include "GLFW/glfw3.h"
#include "boost/format.hpp"
#include "comet/core/engine.h"
#include "glad/glad.h"

#ifdef _WIN32
#include "debug_windows.h"
#endif  // _WIN32

// TODO(m4jr0): Remove this include (and its uses) when a proper game object
// handling will be added.
#include "comet/rendering/temporary_code.h"

namespace comet {
namespace rendering {
void RenderingManager::Initialize() {
  window_ =
      std::make_unique<GlfwWindow>(kOpenGLMajorVersion_, kOpenGLMinorVersion_);
  window_->Initialize();

  if (!gladLoadGLLoader((GLADloadproc)(glfwGetProcAddress))) {
    core::Logger::Get(core::LoggerType::Rendering)
        .Error("Failed to initialize glad");

    throw std::runtime_error(
        "An error occurred during rendering initialization");
  }

  glClearColor(0.5f, 0.5f, 0.5f, 0.5f);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);
  glEnable(GL_CULL_FACE);
  // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

  // TODO(m4jr0): Remove this line when 3D objects are properly handled.
  InitializeTmp();
}

void RenderingManager::Destroy() {
  // TODO(m4jr0): Remove this line when 3D objects are properly handled.
  DestroyTmp();
  window_->Destroy();
}

void RenderingManager::Update(
    double interpolation, game_object::GameObjectManager& game_object_manager) {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  if (glfwWindowShouldClose(window_->GetGlfwWindow()) != 0) {
    core::Engine::GetEngine().Quit();

    return;
  }

  current_time_ += core::Engine::GetEngine().GetTimeManager().GetTimeDelta();

  if (current_time_ > 1000) {
    current_time_ = 0;
    counter_ = 0;
  }

  game_object_manager.Update();

  // TODO(m4jr0): Remove this line when 3D objects are properly handled.
  UpdateTmp();

  window_->Update();
  core::Engine::GetEngine().GetInputManager().Update();
  ++counter_;
}

const GlfwWindow* RenderingManager::GetWindow() const { return window_.get(); }
}  // namespace rendering
}  // namespace comet
