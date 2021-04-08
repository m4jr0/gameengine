// Copyright 2021 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "rendering_manager.h"

#include "boost/format.hpp"
#include "comet/core/engine.h"
#include "comet/core/logger.h"

#ifdef _WIN32
#include "debug_windows.h"
#endif  // _WIN32

// TODO(m4jr0): Remove this include (and its uses) when a proper game object
// handling will be added.
#include "comet/rendering/temporary_code.h"

namespace comet {
void RenderManager::Initialize() {
  const auto logger = Logger::Get(kLoggerCometCoreRenderRenderManager);

  if (!glfwInit()) {
    logger->Error("Failed to initialize GLFW");

    throw std::runtime_error(
        "An error occurred during rendering initialization");
  }

  glfwWindowHint(GLFW_SAMPLES, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, kOpenGLMajorVersion_);
  glfwWindowHint(GLFW_VERSION_MINOR, kOpenGLMinorVersion_);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
  // Fix compilation on Mac OS X.
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif  //  __APPLE__

  window_ = std::make_unique<GlfwWindow>();
  window_->Initialize();
  glewExperimental = true;

  if (glewInit() != GLEW_OK) {
    logger->Error("Failed to initialize GLEW");

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

void RenderManager::Destroy() {
  // TODO(m4jr0): Remove this line when 3D objects are properly handled.
  DestroyTmp();
  window_->Destroy();
}

void RenderManager::Update(double interpolation,
                           GameObjectManager *game_object_manager) {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  if (glfwWindowShouldClose(window_->glfw_window()) != 0) {
    Engine::engine()->engine()->Quit();

    return;
  }

  current_time_ += Engine::engine()->time_manager()->time_delta();

  if (current_time_ > 1000) {
    current_time_ = 0;
    counter_ = 0;
  }

  game_object_manager->Update();

  // TODO(m4jr0): Remove this line when 3D objects are properly handled.
  UpdateTmp();

  const auto error_code = glGetError();

  if (error_code != GL_NO_ERROR) {
    Logger::Get(kLoggerCometCoreRenderRenderManager)
        ->Error("OpenGL Error ", error_code, " (",
                boost::format("0x%02x") % error_code,
                "): ", glewGetErrorString(error_code));
  }

  window_->Update();
  Engine::engine()->input_manager()->Update();
  ++counter_;
}

const GlfwWindow *RenderManager::window() const { return window_.get(); }
}  // namespace comet
