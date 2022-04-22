// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "glfw_window.h"

#include "comet/core/engine.h"
#include "comet/event/window_event.h"

#ifdef _WIN32
#include "debug_windows.h"
#endif  // _WIN32

namespace comet {
namespace rendering {
GlfwWindow::GlfwWindow(const WindowDescr& descr) : Window(descr) {}

GlfwWindow::GlfwWindow(const GlfwWindow& other)
    : Window(other), handle_(nullptr) {}

GlfwWindow::GlfwWindow(GlfwWindow&& other) noexcept
    : Window(std::move(other)), handle_(std::move(other.handle_)) {}

GlfwWindow& GlfwWindow::operator=(const GlfwWindow& other) {
  if (this == &other) {
    return *this;
  }

  Window::operator=(other);
  handle_ = nullptr;
  return *this;
}

GlfwWindow& GlfwWindow::operator=(GlfwWindow&& other) noexcept {
  if (this == &other) {
    return *this;
  }

  Window::operator=(std::move(other));
  handle_ = std::move(other.handle_);
  return *this;
}

void GlfwWindow::Initialize() {
  if (window_count_ == 0) {
    if (glfwInit() != GLFW_TRUE) {
      COMET_LOG_RENDERING_ERROR("Failed to initialize GLFW");

      throw std::runtime_error(
          "An error occurred during rendering initialization");
    }

    SetGlfwHints();

    glfwSetErrorCallback([](int error_code, const char* description) {
      COMET_LOG_RENDERING_ERROR("GLFW Error (", error_code, ")", description);
    });
  }

  handle_ =
      glfwCreateWindow(static_cast<int>(width_), static_cast<int>(height_),
                       name_.c_str(), nullptr, nullptr);

  if (handle_ == nullptr) {
    COMET_LOG_RENDERING_ERROR("Failed to initialize a GLFW window.");

    throw std::runtime_error(
        "An error occurred during rendering initialization");
  }

  window_count_++;

  glfwSetWindowCloseCallback(handle_, [](GLFWwindow* window) {
    core::Engine::GetEngine()
        .GetEventManager()
        .FireEvent<event::WindowCloseEvent>();
  });

  glfwSetWindowSizeCallback(
      handle_, [](GLFWwindow* window, int width, int height) {
        core::Engine::GetEngine()
            .GetEventManager()
            .FireEvent<event::WindowResizeEvent>(width, height);
      });

  is_initialized_ = true;
}

void GlfwWindow::Destroy() {
  if (handle_ != nullptr) {
    glfwDestroyWindow(handle_);
    handle_ = nullptr;
    window_count_--;

    if (window_count_ <= 0) {
      glfwTerminate();
    }
  }
}

void GlfwWindow::SetGlfwHints() {
  // By default, a GLFW Window is API-less.
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
}

void GlfwWindow::SetSize(unsigned int width, unsigned int height) {
  width_ = width;
  height_ = height;

  if (handle_ != nullptr) {
    glfwSetWindowSize(handle_, width_, height_);
  }
}

bool GlfwWindow::IsInitialized() const { return is_initialized_; }

const GLFWwindow* GlfwWindow::GetHandle() const noexcept { return handle_; }
}  // namespace rendering
}  // namespace comet
