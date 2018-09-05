// Copyright 2018 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "render_manager.hpp"

#include <boost/format.hpp>
#include <stdexcept>

#include <core/locator/locator.hpp>
#include <utils/logger.hpp>

// TODO(m4jr0): Remove this include (and its uses) when a proper game object
// handling will be added.
#include "../../temporary_code.hpp"

// Allow debugging memory leaks.
#include <debug.hpp>

namespace koma {
void RenderManager::Initialize() {
  auto logger = Logger::Get(LOGGER_KOMA_CORE_RENDER_RENDER_MANAGER);

  if (!glfwInit()) {
    logger->Error("Failed to initialize GLFW");

    throw std::runtime_error(
      "An error occurred during rendering initialization"
    );
  }

  glfwWindowHint(GLFW_SAMPLES, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, this->kOpenGLMajorVersion_);
  glfwWindowHint(GLFW_VERSION_MINOR, this->kOpenGLMinorVersion_);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
  // Fix  compoilation on Mac OS X.
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

  this->height_ = this->kDefaultHeight_;
  this->width_ = this->kDefaultWidth_;

  this->window_ = glfwCreateWindow(
    this->width_,
    this->height_,
    this->GetTitle().c_str(),
    nullptr,
    nullptr
  );

  if (!this->window_) {
    logger->Error(
      "Failed to open a GLFW window. If you have an Intel GPU, they are not ",
      this->kOpenGLMajorVersion_,
      ".",
      this->kOpenGLMinorVersion_,
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

  glClearColor(0.5f, 0.5f, 0.5f, 0.5f);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);
  glEnable(GL_CULL_FACE);
  // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

  // TODO(m4jr0): Remove this line when 3D objects are properly handled.
  InitializeTmp(this->width_, this->height_);
}

void RenderManager::Destroy() {
  // TODO(m4jr0): Remove this line when 3D objects are properly handled.
  DestroyTmp();

  glfwTerminate();
}

void RenderManager::Update(double interpolation,
                           GameObjectManager *game_object_manager) {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  if (glfwWindowShouldClose(this->window_) != 0) {
    Locator::game().Quit();

    return;
  }

  glfwGetWindowSize(this->window_, &this->width_, &this->height_);

  this->current_time_ += Locator::time_manager().time_delta();

  if (this->current_time_ > 1000) {
    glfwSetWindowTitle(this->window_, this->GetTitle().c_str());

    this->current_time_ = 0;
    this->counter_ = 0;
  }

  game_object_manager->Update();

  // TODO(m4jr0): Remove this line when 3D objects are properly handled.
  UpdateTmp();

  GLenum error_code = glGetError();

  if (error_code != GL_NO_ERROR) {
    Logger::Get(LOGGER_KOMA_CORE_RENDER_RENDER_MANAGER)->Error(
      "OpenGL Error ",
      error_code,
      " (",
      boost::format("0x%02x") % error_code,
      "): ",
      glewGetErrorString(error_code)
    );
  }

  glfwSwapBuffers(this->window_);

  Locator::input_manager().Update();

  ++this->counter_;
}

const int RenderManager::width() const noexcept {
  return this->width_;
}

const int RenderManager::height() const noexcept {
  return this->height_;
}

const GLFWwindow *RenderManager::window() const noexcept {
  return this->window_;
}

void RenderManager::width(int width) noexcept {
  this->width_ = width;

  glfwSetWindowSize(this->window_, this->width_, this->height_);
}

void RenderManager::height(int height) noexcept {
  this->height_ = height;

  glfwSetWindowSize(this->window_, this->width_, this->height_);
}

std::string RenderManager::GetTitle() {
  return std::string() +
    this->kDefaultRenderingWindowName_ +
    " | " +
    std::to_string(this->width_) +
    "x" +
    std::to_string(this->height_) +
    "@" +
    std::to_string(this->counter_) +
    "FPS";
}
};  // namespace koma
