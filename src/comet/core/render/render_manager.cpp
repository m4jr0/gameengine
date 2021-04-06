// Copyright 2021 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "render_manager.hpp"

#include "boost/format.hpp"
#include "core/engine.hpp"
#include "utils/logger.hpp"

#ifdef _WIN32
#include "debug_windows.hpp"
#endif  // _WIN32

// TODO(m4jr0): Remove this include (and its uses) when a proper game object
// handling will be added.
#include "../temporary_code.hpp"

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

  height_ = kDefaultHeight_;
  width_ = kDefaultWidth_;
  window_ =
      glfwCreateWindow(width_, height_, GetTitle().c_str(), nullptr, nullptr);

  if (window_ == nullptr) {
    logger->Error(
        "Failed to open a GLFW window. If you have an Intel GPU, they are not ",
        kOpenGLMajorVersion_, ".", kOpenGLMinorVersion_, " compatible");

    throw std::runtime_error(
        "An error occurred during rendering initialization");
  }

  glfwMakeContextCurrent(window_);
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
  InitializeTmp(width_, height_);
}

void RenderManager::Destroy() {
  // TODO(m4jr0): Remove this line when 3D objects are properly handled.
  DestroyTmp();
  glfwTerminate();
}

void RenderManager::Update(double interpolation,
                           GameObjectManager *game_object_manager) {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  if (glfwWindowShouldClose(window_) != 0) {
    Engine::engine()->engine()->Quit();

    return;
  }

  glfwGetWindowSize(window_, &width_, &height_);

  current_time_ += Engine::engine()->time_manager()->time_delta();

  if (current_time_ > 1000) {
    glfwSetWindowTitle(window_, GetTitle().c_str());

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

  glfwSwapBuffers(window_);
  Engine::engine()->input_manager()->Update();
  ++counter_;
}

const int RenderManager::width() const noexcept { return width_; }

const int RenderManager::height() const noexcept { return height_; }

const GLFWwindow *RenderManager::window() const noexcept { return window_; }

void RenderManager::width(int width) noexcept {
  width_ = width;

  glfwSetWindowSize(window_, width_, height_);
}

void RenderManager::height(int height) noexcept {
  height_ = height;
  glfwSetWindowSize(window_, width_, height_);
}

std::string RenderManager::GetTitle() {
  return std::string() + kDefaultRenderingWindowName_ + " | " +
         std::to_string(width_) + "x" + std::to_string(height_) + "@" +
         std::to_string(counter_) + "FPS";
}
}  // namespace comet
