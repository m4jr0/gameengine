// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "glfw_window.h"
////////////////////////////////////////////////////////////////////////////////

// External. ///////////////////////////////////////////////////////////////////
#include <utility>
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/logger.h"
#include "comet/event/event_manager.h"
#include "comet/input/input_manager.h"
#include "comet/rendering/window/window_event.h"

namespace comet {
namespace rendering {
GlfwWindow::GlfwWindow(const WindowDescr& descr) : Window{descr} {}

GlfwWindow::GlfwWindow(const GlfwWindow& other)
    : Window{other}, handle_{nullptr} {}

GlfwWindow::GlfwWindow(GlfwWindow&& other) noexcept
    : Window{std::move(other)},
      handle_{other.handle_},
      is_resize_{other.is_resize_} {
  other.handle_ = nullptr;
  other.is_resize_ = false;
}

GlfwWindow& GlfwWindow::operator=(const GlfwWindow& other) {
  if (this == &other) {
    return *this;
  }

  Window::operator=(other);
  handle_ = nullptr;
  is_resize_ = other.is_resize_;
  return *this;
}

GlfwWindow& GlfwWindow::operator=(GlfwWindow&& other) noexcept {
  if (this == &other) {
    return *this;
  }

  Window::operator=(std::move(other));
  handle_ = other.handle_;
  is_resize_ = other.is_resize_;
  other.handle_ = nullptr;
  other.is_resize_ = false;
  return *this;
}

void GlfwWindow::Initialize() {
  Window::Initialize();

  if (window_count_ == 0) {
    COMET_LOG_RENDERING_INFO("Initializing GLFW...");
    [[maybe_unused]] const auto result{glfwInit()};
    COMET_ASSERT(result == GLFW_TRUE, "Could not initialize GLFW!");
    SetGlfwHints();

    glfwSetErrorCallback([](s32 error_code, const schar* description) {
      COMET_LOG_RENDERING_ERROR("GLFW Error (", error_code, ")", description);
    });
  }

  handle_ = glfwCreateWindow(width_, height_, name_, nullptr, nullptr);
  COMET_ASSERT(handle_ != nullptr,
               "Something bad happened while creating the GLFW window!");
  window_count_++;

  glfwSetWindowUserPointer(handle_, static_cast<void*>(this));
  glfwSetWindowAspectRatio(handle_, width_, height_);

  glfwSetWindowCloseCallback(handle_, [](GLFWwindow*) {
    event::EventManager::Get().FireEvent<rendering::WindowCloseEvent>();
  });

  glfwSetWindowSizeCallback(
      handle_, [](GLFWwindow* window, s32 width, s32 height) {
        auto* self{static_cast<GlfwWindow*>(glfwGetWindowUserPointer(window))};
        self->SetSize(static_cast<WindowSize>(width),
                      static_cast<WindowSize>(height));
      });
}

void GlfwWindow::Destroy() {
  if (handle_ != nullptr) {
    glfwDestroyWindow(handle_);
    handle_ = nullptr;
    window_count_--;

    if (window_count_ <= 0) {
      COMET_LOG_RENDERING_INFO("Terminating GLFW...");
      glfwTerminate();
    }
  }

  is_resize_ = false;
  Window::Destroy();
}

void GlfwWindow::Update() {
  if (!is_resize_ ||
      input::InputManager::Get().IsMousePressed(input::MouseButton::Left)) {
    return;
  }

  event::EventManager::Get().FireEvent<rendering::WindowResizeEvent>(width_,
                                                                     height_);
  is_resize_ = false;
}

void GlfwWindow::SetGlfwHints() {
  // By default, a GLFW Window is API-less.
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
}

void GlfwWindow::SetSize(WindowSize width, WindowSize height) {
  width_ = width;
  height_ = height;
  is_resize_ = true;
}

GLFWwindow* GlfwWindow::GetHandle() noexcept { return handle_; }

GlfwWindow::operator GLFWwindow*() noexcept { return GetHandle(); }

void GlfwWindow::UpdateSize() {
  if (handle_ != nullptr) {
    glfwSetWindowSize(handle_, width_, height_);

    if (width_ > 0 && height_ > 0) {
      glfwSetWindowAspectRatio(handle_, width_, height_);
    }
  }

  is_resize_ = true;
}
}  // namespace rendering
}  // namespace comet
