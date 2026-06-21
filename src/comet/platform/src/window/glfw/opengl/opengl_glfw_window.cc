// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_platform_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/platform/window/glfw/opengl/opengl_glfw_window.h"
////////////////////////////////////////////////////////////////////////////////

// External. ///////////////////////////////////////////////////////////////////
#include <utility>
////////////////////////////////////////////////////////////////////////////////

namespace comet {
namespace platform {
OpenGlGlfwWindow::OpenGlGlfwWindow(OpenGlGlfwWindowDescr& descr)
    : GlfwWindow{descr},
      is_vsync_{descr.is_vsync},
      opengl_major_version_{descr.opengl_major_version},
      opengl_minor_version_{descr.opengl_minor_version},
      msaa_sample_count_{descr.msaa_sample_count} {}

OpenGlGlfwWindow::OpenGlGlfwWindow(const OpenGlGlfwWindow& other)
    : GlfwWindow{other},
      is_vsync_{other.is_vsync_},
      opengl_major_version_{other.opengl_major_version_},
      opengl_minor_version_{other.opengl_minor_version_},
      msaa_sample_count_{other.msaa_sample_count_} {}

OpenGlGlfwWindow::OpenGlGlfwWindow(OpenGlGlfwWindow&& other) noexcept
    : GlfwWindow{std::move(other)},
      is_vsync_{other.is_vsync_},
      opengl_major_version_{other.opengl_major_version_},
      opengl_minor_version_{other.opengl_minor_version_},
      msaa_sample_count_{other.msaa_sample_count_} {
  other.is_vsync_ = true;
  other.opengl_major_version_ = 0;
  other.opengl_minor_version_ = 0;
  other.msaa_sample_count_ = 1;
}

OpenGlGlfwWindow& OpenGlGlfwWindow::operator=(const OpenGlGlfwWindow& other) {
  if (this == &other) {
    return *this;
  }

  GlfwWindow::operator=(other);
  is_vsync_ = other.is_vsync_;
  opengl_major_version_ = other.opengl_major_version_;
  opengl_minor_version_ = other.opengl_minor_version_;
  msaa_sample_count_ = other.msaa_sample_count_;
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
  msaa_sample_count_ = other.msaa_sample_count_;

  other.is_vsync_ = true;
  other.opengl_major_version_ = 0;
  other.opengl_minor_version_ = 0;
  other.msaa_sample_count_ = 1;
  return *this;
}

void OpenGlGlfwWindow::SetGlfwHints() {
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, opengl_major_version_);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, opengl_minor_version_);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);

  if (msaa_sample_count_ > 1) {
    glfwWindowHint(GLFW_SAMPLES, msaa_sample_count_);
  }
}

void OpenGlGlfwWindow::SwapBuffers() const { glfwSwapBuffers(handle_); }

bool OpenGlGlfwWindow::IsVSync() const noexcept { return is_vsync_; }

void OpenGlGlfwWindow::SetVSync(bool is_vsync) {
  is_vsync_ = is_vsync;

  if (handle_ == nullptr) {
    return;
  }

  glfwSwapInterval(is_vsync_ ? 1 : 0);
}

void OpenGlGlfwWindow::OnInitialize() {
  GlfwWindow::OnInitialize();
  glfwMakeContextCurrent(handle_);
  SetVSync(is_vsync_);
}
}  // namespace platform
}  // namespace comet