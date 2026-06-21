// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_platform_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/platform/window/glfw/glfw_window.h"
////////////////////////////////////////////////////////////////////////////////

// External. ///////////////////////////////////////////////////////////////////
#include <utility>
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/debug/debug_label.h"
#include "comet/core/logger/logging.h"
#include "comet/runtime/event/event_manager.h"
#include "comet/runtime/input/input_manager.h"
#include "comet/platform/window/window_event.h"

namespace comet {
namespace platform {
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

void GlfwWindow::OnInitialize() {
  if (window_count_ == 0) {
    COMET_LOG_INFO(LoggerType::Rendering, "GlfwWindow::OnInitialize",
                   "initializing glfw");

    glfwSetErrorCallback([](s32 error_code, const schar* description) {
      COMET_LOG_ERROR(LoggerType::Rendering, "GlfwWindow::internal::glfwError",
                      "glfw error", "error_code", error_code, "description",
                      description);
    });

    [[maybe_unused]] const auto result{glfwInit()};
    COMET_ASSERT(result == GLFW_TRUE, "GlfwWindow::OnInitialize",
                 "glfw initialization failed");

    DumpPlatform();
    SetGlfwHints();
  }

  handle_ = glfwCreateWindow(width_, height_, name_, nullptr, nullptr);
  COMET_ASSERT(handle_ != nullptr, "GlfwWindow::OnInitialize",
               "glfw window creation failed");

  window_count_++;

  glfwSetWindowUserPointer(handle_, static_cast<void*>(this));
  glfwSetWindowAspectRatio(handle_, width_, height_);

  glfwSetWindowCloseCallback(handle_, [](GLFWwindow*) {
    event::EventManager::Get().FireEvent<WindowCloseEvent>();
  });

  glfwSetWindowSizeCallback(
      handle_, [](GLFWwindow* window, s32 width, s32 height) {
        auto* self{static_cast<GlfwWindow*>(glfwGetWindowUserPointer(window))};
        self->SetSize(static_cast<WindowSize>(width),
                      static_cast<WindowSize>(height));
      });
}

void GlfwWindow::OnDestroy() {
  if (handle_ != nullptr) {
    glfwDestroyWindow(handle_);
    handle_ = nullptr;
    window_count_--;

    if (window_count_ <= 0) {
      COMET_LOG_INFO(LoggerType::Rendering, "GlfwWindow::OnDestroy",
                     "terminating glfw");
      glfwTerminate();
    }
  }

  is_resize_ = false;
}

void GlfwWindow::OnUpdate() {
  if (!is_resize_ ||
      input::InputManager::Get().IsMousePressed(input::MouseButton::Left)) {
    return;
  }

  event::EventManager::Get().FireEvent<WindowResizeEvent>(width_, height_);
  is_resize_ = false;
}

void GlfwWindow::UpdateSize() {
  COMET_ASSERT(handle_ != nullptr, "GlfwWindow::UpdateSize",
               "glfw window handle is null");

  if (handle_ != nullptr) {
    glfwSetWindowSize(handle_, width_, height_);

    if (width_ > 0 && height_ > 0) {
      glfwSetWindowAspectRatio(handle_, width_, height_);
    }
  }

  is_resize_ = true;
}

void GlfwWindow::DumpPlatform() const {
  const auto platform{glfwGetPlatform()};
  [[maybe_unused]] const schar* label;

  switch (platform) {
    case GLFW_PLATFORM_WIN32:
      label = "win32";
      break;
    case GLFW_PLATFORM_COCOA:
      label = "cocoa";
      break;
    case GLFW_PLATFORM_WAYLAND:
      label = "wayland";
      break;
    case GLFW_PLATFORM_X11:
      label = "x11";
      break;
    case GLFW_PLATFORM_NULL:
      label = "null";
      break;

    default:
      label = kUnknownLabel;
      break;
  }

  COMET_LOG_INFO(LoggerType::Rendering, "GlfwWindow::DumpPlatform",
                 "glfw platform detected", "platform", label);
}
}  // namespace platform
}  // namespace comet
