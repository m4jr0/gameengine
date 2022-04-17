// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "opengl_glfw_window.h"

#ifdef _WIN32
#include "debug_windows.h"
#endif  // _WIN32

namespace comet {
namespace rendering {
namespace gl {
OpenGlGlfwWindow::OpenGlGlfwWindow(OpenGlGlfwWindowDescr& descr)
    : GlfwWindow(descr),
      is_vsync_(descr.is_vsync),
      opengl_major_version_(descr.opengl_major_version),
      opengl_minor_version_(descr.opengl_minor_version) {}

OpenGlGlfwWindow::OpenGlGlfwWindow(const OpenGlGlfwWindow& other)
    : GlfwWindow(other),
      is_vsync_(other.is_vsync_),
      opengl_major_version_(other.opengl_major_version_),
      opengl_minor_version_(other.opengl_minor_version_) {}

OpenGlGlfwWindow::OpenGlGlfwWindow(OpenGlGlfwWindow&& other) noexcept
    : GlfwWindow(std::move(other)),
      is_vsync_(std::move(other.is_vsync_)),
      opengl_major_version_(std::move(other.opengl_major_version_)),
      opengl_minor_version_(std::move(other.opengl_minor_version_)) {}

OpenGlGlfwWindow& OpenGlGlfwWindow::operator=(const OpenGlGlfwWindow& other) {
  if (this == &other) {
    return *this;
  }

  GlfwWindow::operator=(other);
  is_vsync_ = other.is_vsync_;
  opengl_major_version_ = other.opengl_major_version_;
  opengl_minor_version_ = other.opengl_minor_version_;
  return *this;
}

OpenGlGlfwWindow& OpenGlGlfwWindow::operator=(
    OpenGlGlfwWindow&& other) noexcept {
  if (this == &other) {
    return *this;
  }

  GlfwWindow::operator=(std::move(other));
  is_vsync_ = std::move(other.is_vsync_);
  opengl_major_version_ = std::move(other.opengl_major_version_);
  opengl_minor_version_ = std::move(other.opengl_minor_version_);
  return *this;
}

void OpenGlGlfwWindow::Initialize() {
  GlfwWindow::Initialize();
  SetVSync(is_vsync_);
}

void OpenGlGlfwWindow::SetGlfwHints() {
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, opengl_major_version_);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, opengl_minor_version_);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
}

void OpenGlGlfwWindow::SwapBuffers() const { glfwSwapBuffers(handle_); }

bool OpenGlGlfwWindow::IsVSync() const noexcept { return is_vsync_; }

void OpenGlGlfwWindow::SetVSync(bool is_vsync) {
  is_vsync_ = is_vsync;

  if (handle_ == nullptr) {
    return;
  }

  if (is_vsync_) {
    glfwSwapInterval(1);
  } else {
    glfwSwapInterval(0);
  }
}
}  // namespace gl
}  // namespace rendering
}  // namespace comet
