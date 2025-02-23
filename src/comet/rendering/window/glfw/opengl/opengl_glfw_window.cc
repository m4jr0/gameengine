// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"

#include "opengl_glfw_window.h"

#include <utility>

namespace comet {
namespace rendering {
namespace gl {
OpenGlGlfwWindow::OpenGlGlfwWindow(OpenGlGlfwWindowDescr& descr)
    : GlfwWindow{descr},
      is_vsync_{descr.is_vsync},
      opengl_major_version_{descr.opengl_major_version},
      opengl_minor_version_{descr.opengl_minor_version},
      anti_aliasing_type_{descr.anti_aliasing_type} {}

OpenGlGlfwWindow::OpenGlGlfwWindow(const OpenGlGlfwWindow& other)
    : GlfwWindow{other},
      is_vsync_{other.is_vsync_},
      opengl_major_version_{other.opengl_major_version_},
      opengl_minor_version_{other.opengl_minor_version_},
      anti_aliasing_type_{other.anti_aliasing_type_} {}

OpenGlGlfwWindow::OpenGlGlfwWindow(OpenGlGlfwWindow&& other) noexcept
    : GlfwWindow{std::move(other)},
      is_vsync_{other.is_vsync_},
      opengl_major_version_{other.opengl_major_version_},
      opengl_minor_version_{other.opengl_minor_version_},
      anti_aliasing_type_{other.anti_aliasing_type_} {
  other.is_vsync_ = true;
  other.opengl_major_version_ = 0;
  other.opengl_minor_version_ = 0;
  other.anti_aliasing_type_ = AntiAliasingType::None;
}

OpenGlGlfwWindow& OpenGlGlfwWindow::operator=(const OpenGlGlfwWindow& other) {
  if (this == &other) {
    return *this;
  }

  GlfwWindow::operator=(other);
  is_vsync_ = other.is_vsync_;
  opengl_major_version_ = other.opengl_major_version_;
  opengl_minor_version_ = other.opengl_minor_version_;
  anti_aliasing_type_ = other.anti_aliasing_type_;
  return *this;
}

OpenGlGlfwWindow& OpenGlGlfwWindow::operator=(
    OpenGlGlfwWindow&& other) noexcept {
  if (this == &other) {
    return *this;
  }

  GlfwWindow::operator=(std::move(other));
  is_vsync_ = other.is_vsync_;
  opengl_major_version_ = other.opengl_major_version_;
  opengl_minor_version_ = other.opengl_minor_version_;
  anti_aliasing_type_ = other.anti_aliasing_type_;
  other.is_vsync_ = true;
  other.opengl_major_version_ = 0;
  other.opengl_minor_version_ = 0;
  other.anti_aliasing_type_ = AntiAliasingType::None;
  return *this;
}

void OpenGlGlfwWindow::Initialize() {
  GlfwWindow::Initialize();
  glfwMakeContextCurrent(handle_);
  SetVSync(is_vsync_);
}

void OpenGlGlfwWindow::SetGlfwHints() {
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, opengl_major_version_);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, opengl_minor_version_);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  if (anti_aliasing_type_ != AntiAliasingType::None) {
    s32 bit_value;

    switch (anti_aliasing_type_) {
      case AntiAliasingType::Msaa:
      case AntiAliasingType::MsaaX64:
      case AntiAliasingType::MsaaX32:
      case AntiAliasingType::MsaaX16:
      case AntiAliasingType::MsaaX8:
        bit_value = 8;
        break;
      case AntiAliasingType::MsaaX4:
        bit_value = 4;
        break;
      case AntiAliasingType::MsaaX2:
        bit_value = 2;
        break;
      default:
        bit_value = 1;
    }

    glfwWindowHint(GLFW_SAMPLES, bit_value);
  }
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
