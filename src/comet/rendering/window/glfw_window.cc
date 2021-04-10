// Copyright 2021 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "glfw_window.h"

namespace comet {
namespace rendering {
GlfwWindow::GlfwWindow(const std::string& name, unsigned int width,
                       unsigned int height) {
  name_ = name;
  width_ = width;
  height_ = height;
}

GlfwWindow::GlfwWindow(const GlfwWindow& other)
    : is_vsync_(other.is_vsync_), window_(nullptr) {}

GlfwWindow& GlfwWindow::operator=(const GlfwWindow& other) {
  if (this == &other) {
    return *this;
  }

  is_vsync_ = other.is_vsync_;
  window_ = nullptr;
  return *this;
}

GlfwWindow::~GlfwWindow() { Destroy(); }

void GlfwWindow::Initialize() {
  const auto& logger = core::Logger::Get(core::LoggerType::Rendering);

  if (window_count_ == 0) {
    const bool is_glfw_initialized = glfwInit() == GLFW_TRUE;

    if (!is_glfw_initialized) {
      logger.Error("Failed to initialize GLFW");

      throw std::runtime_error(
          "An error occurred during rendering initialization");
    }
  }

  window_ =
      glfwCreateWindow(static_cast<int>(width_), static_cast<int>(height_),
                       name_.c_str(), nullptr, nullptr);

  if (window_ == nullptr) {
    logger.Error("Failed to initialize a GLFW window.");

    throw std::runtime_error(
        "An error occurred during rendering initialization");
  }

  window_count_++;
  glfwMakeContextCurrent(window_);
  SetVsync(is_vsync_);
}

void GlfwWindow::Destroy() {
  if (window_ != nullptr) {
    glfwDestroyWindow(window_);
    window_ = nullptr;
    window_count_--;

    if (window_count_ <= 0) {
      glfwTerminate();
    }
  }
}

void GlfwWindow::Update() {
  if (window_ != nullptr) {
    glfwSwapBuffers(window_);
  }
}

void GlfwWindow::SetSize(unsigned int width, unsigned int height) {
  width_ = width;
  height_ = height;

  if (window_ != nullptr) {
    glfwSetWindowSize(window_, width_, height_);
  }
}

GLFWwindow* GlfwWindow::GetGlfwWindow() const noexcept { return window_; }

bool GlfwWindow::IsVsync() const noexcept { return is_vsync_; }

void GlfwWindow::SetVsync(bool is_vsync) {
  is_vsync_ = is_vsync;
  if (window_ == nullptr) {
    return;
  }

  if (is_vsync_) {
    glfwSwapInterval(1);
  } else {
    glfwSwapInterval(0);
  }
}
}  // namespace rendering
}  // namespace comet
