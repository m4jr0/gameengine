// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "glfw_window.h"

#include "comet/core/engine.h"
#include "comet/event/window_event.h"

namespace comet {
namespace rendering {
GlfwWindow::GlfwWindow(const WindowDescr& descr) : Window(descr) {}

GlfwWindow::GlfwWindow(const GlfwWindow& other)
    : Window{other}, handle_{nullptr} {}

GlfwWindow::GlfwWindow(GlfwWindow&& other) noexcept
    : Window{std::move(other)},
      handle_{other.handle_},
      new_width_{other.new_width_},
      new_height_{other.new_height_},
      is_resize_event_{other.is_resize_event_} {
  other.handle_ = nullptr;
  other.new_width_ = 0;
  other.new_height_ = 0;
  other.is_resize_event_ = false;
}

GlfwWindow& GlfwWindow::operator=(const GlfwWindow& other) {
  if (this == &other) {
    return *this;
  }

  Window::operator=(other);
  handle_ = nullptr;
  new_width_ = other.new_width_;
  new_height_ = other.new_height_;
  is_resize_event_ = other.is_resize_event_;
  return *this;
}

GlfwWindow& GlfwWindow::operator=(GlfwWindow&& other) noexcept {
  if (this == &other) {
    return *this;
  }

  Window::operator=(std::move(other));
  handle_ = other.handle_;
  new_width_ = other.new_width_;
  new_height_ = other.new_height_;
  is_resize_event_ = other.is_resize_event_;
  other.handle_ = nullptr;
  other.new_width_ = 0;
  other.new_height_ = 0;
  other.is_resize_event_ = false;
  return *this;
}

void GlfwWindow::Initialize() {
  Window::Initialize();

  if (window_count_ == 0) {
    const auto result{glfwInit()};
    COMET_ASSERT(result == GLFW_TRUE, "Could not initialize GLFW!");
    SetGlfwHints();

    glfwSetErrorCallback([](s32 error_code, const schar* description) {
      COMET_LOG_RENDERING_ERROR("GLFW Error (", error_code, ")", description);
    });
  }

  handle_ = glfwCreateWindow(width_, height_, name_.c_str(), nullptr, nullptr);
  COMET_ASSERT(handle_ != nullptr,
               "Something bad happened while creating the GLFW window!");
  window_count_++;

  glfwSetWindowUserPointer(handle_, static_cast<void*>(this));
  glfwSetWindowAspectRatio(handle_, width_, height_);

  glfwSetWindowCloseCallback(handle_, [](GLFWwindow* window) {
    Engine::Get().GetEventManager().FireEvent<event::WindowCloseEvent>();
  });

  glfwSetWindowSizeCallback(
      handle_, [](GLFWwindow* window, s32 width, s32 height) {
        auto* self = static_cast<GlfwWindow*>(glfwGetWindowUserPointer(window));
        self->SetUpResizeEvent(static_cast<WindowSize>(width),
                               static_cast<WindowSize>(height));
      });
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

  is_resize_event_ = false;
  new_width_ = 0;
  new_height_ = 0;
  Window::Destroy();
}

void GlfwWindow::Update() {
  if (!is_resize_event_ || Engine::Get().GetInputManager().IsMousePressed(
                               input::MouseButton::Left)) {
    return;
  }

  if (new_width_ != width_ || new_height_ != height_) {
    Engine::Get().GetEventManager().FireEvent<event::WindowResizeEvent>(
        new_width_, new_height_);
  }

  new_width_ = new_height_ = 0;
  is_resize_event_ = false;
}

void GlfwWindow::SetGlfwHints() {
  // By default, a GLFW Window is API-less.
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
}

void GlfwWindow::SetSize(WindowSize width, WindowSize height) {
  width_ = width;
  height_ = height;

  if (handle_ != nullptr) {
    glfwSetWindowSize(handle_, width_, height_);

    if (width_ > 0 && height_ > 0) {
      glfwSetWindowAspectRatio(handle_, width_, height_);
    }
  }
}

void GlfwWindow::SetUpResizeEvent(WindowSize width, WindowSize height) {
  new_width_ = width;
  new_height_ = height;
  is_resize_event_ = true;
}

GLFWwindow* GlfwWindow::GetHandle() noexcept { return handle_; }

GlfwWindow::operator GLFWwindow*() noexcept { return GetHandle(); }
}  // namespace rendering
}  // namespace comet
