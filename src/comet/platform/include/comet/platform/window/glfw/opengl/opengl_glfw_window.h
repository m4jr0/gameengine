// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_PLATFORM_WINDOW_GLFW_OPENGL_OPENGL_GLFW_WINDOW_H_
#define COMET_PLATFORM_WINDOW_GLFW_OPENGL_OPENGL_GLFW_WINDOW_H_

#include "comet/core/essentials.h"
#include "comet/platform/window/glfw/glfw_window.h"

namespace comet {
namespace platform {
struct OpenGlGlfwWindowDescr : WindowDescr {
  bool is_vsync{true};
  u8 opengl_major_version{0};
  u8 opengl_minor_version{0};
  u8 msaa_sample_count{1};
};

class OpenGlGlfwWindow : public GlfwWindow {
 public:
  OpenGlGlfwWindow() = delete;
  explicit OpenGlGlfwWindow(OpenGlGlfwWindowDescr& descr);
  OpenGlGlfwWindow(const OpenGlGlfwWindow&);
  OpenGlGlfwWindow(OpenGlGlfwWindow&&) noexcept;
  OpenGlGlfwWindow& operator=(const OpenGlGlfwWindow&);
  OpenGlGlfwWindow& operator=(OpenGlGlfwWindow&&) noexcept;
  ~OpenGlGlfwWindow() override = default;

  void SetGlfwHints() override;

  void SwapBuffers() const;

  bool IsVSync() const noexcept;
  void SetVSync(bool is_vsync);

 protected:
  void OnInitialize() override;

 private:
  bool is_vsync_{true};
  u8 opengl_major_version_{0};
  u8 opengl_minor_version_{0};
  u8 msaa_sample_count_{1};
};
}  // namespace platform
}  // namespace comet

#endif  // COMET_PLATFORM_WINDOW_GLFW_OPENGL_OPENGL_GLFW_WINDOW_H_