// Copyright 2018 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Allows debugging memory leaks.
#include "../../debug.hpp"

#include "rendering_manager.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stdexcept>

#include "../../utils/logger.hpp"
#include "../locator/locator.hpp"

// TODO(m4jr0): Remove this include (and its uses) when a proper game object
// handling will be added.
#include "../../temporary_code.hpp"

namespace koma {
void RenderingManager::Initialize() {
  auto logger = Logger::Get(LOGGER_KOMA_CORE_RENDERING);

  if (!glfwInit()) {
    logger->Error("Failed to initialize GLFW");

    throw std::runtime_error(
      "An error occurred during rendering initialization"
    );
  }

  glfwWindowHint(GLFW_SAMPLES, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, this->kOpenGLMajorVersion);
  glfwWindowHint(GLFW_VERSION_MINOR, this->kOpenGLMinorVersion);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_OPENGL_CORE_PROFILE);

  this->current_height_ = this->kDefaultHeight;
  this->current_width_ = this->kDefaultWidth;

  this->window_ = glfwCreateWindow(
    this->current_width_,
    this->current_height_,
    this->kRenderingWindowName,
    nullptr,
    nullptr
  );

  if (!this->window_) {
    logger->Error(
      "Failed to open a GLFW window. If you have an Intel GPU, they are not ",
      this->kOpenGLMajorVersion,
      ".",
      this->kOpenGLMinorVersion,
      " compatible"
    );

    throw std::runtime_error(
      "An error occurred during rendering initialization"
    );
  }

  glfwMakeContextCurrent(this->window_);
  glewExperimental = true;

  if (glewInit() != GLEW_OK) {
    logger->Error("Failed to initialize GLEW");

    throw std::runtime_error(
      "An error occurred during rendering initialization"
    );
  }

  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);

  InitializeTmp(this->current_width_, this->current_height_);
}

void RenderingManager::Destroy() {
  DestroyTmp();

  glfwTerminate();
}

void RenderingManager::Update(double interpolation,
                              GameObjectManager *game_object_manager) {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  if (glfwWindowShouldClose(this->window_) != 0) Locator::game().Quit();

  this->current_time_ += Locator::time_manager().time_delta();

  if (this->current_time_ > 1000) {
    glfwSetWindowTitle(this->window_, std::move(this->GetTitle()).c_str());

    this->current_time_ = 0;
    this->counter_ = 0;
  }

  game_object_manager->Update(interpolation);

  UpdateTmp();

  glfwSwapBuffers(this->window_);
  glfwPollEvents();

  ++this->counter_;
}

std::string RenderingManager::GetTitle() {
  return std::string() +
    this->kRenderingWindowName +
    " | " +
    std::to_string(this->current_width_) +
    "x" +
    std::to_string(this->current_height_) +
    "@" +
    std::to_string(this->counter_) +
    "FPS";
}
};  // namespace koma