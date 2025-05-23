// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_WINDOW_GLFW_OPENGL_OPENGL_GLFW_WINDOW_H_
#define COMET_COMET_RENDERING_WINDOW_GLFW_OPENGL_OPENGL_GLFW_WINDOW_H_

#define GLFW_INCLUDE_NONE
#include "glad/glad.h"

#include "comet/core/essentials.h"
#include "comet/rendering/window/glfw/glfw_window.h"

namespace comet {
namespace rendering {
namespace gl {
struct OpenGlGlfwWindowDescr : WindowDescr {
  bool is_vsync{true};
  u8 opengl_major_version{0};
  u8 opengl_minor_version{0};
  AntiAliasingType anti_aliasing_type{AntiAliasingType::None};
};

class OpenGlGlfwWindow : public GlfwWindow {
 public:
  OpenGlGlfwWindow() = delete;
  explicit OpenGlGlfwWindow(OpenGlGlfwWindowDescr& descr);
  OpenGlGlfwWindow(const OpenGlGlfwWindow&);
  OpenGlGlfwWindow(OpenGlGlfwWindow&&) noexcept;
  OpenGlGlfwWindow& operator=(const OpenGlGlfwWindow&);
  OpenGlGlfwWindow& operator=(OpenGlGlfwWindow&&) noexcept;
  virtual ~OpenGlGlfwWindow() = default;

  void Initialize() override;
  void SetGlfwHints() override;
  void SwapBuffers() const;

  bool IsVSync() const noexcept;
  void SetVSync(bool is_vsync);

 private:
  bool is_vsync_{true};
  u8 opengl_major_version_{0};
  u8 opengl_minor_version_{0};
  AntiAliasingType anti_aliasing_type_{AntiAliasingType::None};
};
}  // namespace gl
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_WINDOW_GLFW_OPENGL_OPENGL_GLFW_WINDOW_H_