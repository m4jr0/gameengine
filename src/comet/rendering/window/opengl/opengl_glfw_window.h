// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_WINDOW_OPENGL_OPENGL_GLFW_WINDOW_H_
#define COMET_COMET_RENDERING_WINDOW_OPENGL_OPENGL_GLFW_WINDOW_H_

#define GLFW_INCLUDE_NONE

#include "comet_precompile.h"

#include "glad/glad.h"

#include "comet/rendering/window/glfw_window.h"

namespace comet {
namespace rendering {
namespace gl {
struct OpenGlGlfwWindowDescr : WindowDescr {
  bool is_vsync = true;
  unsigned int opengl_major_version = 0;
  unsigned int opengl_minor_version = 0;
};

class OpenGlGlfwWindow : public GlfwWindow {
 public:
  OpenGlGlfwWindow() = default;
  explicit OpenGlGlfwWindow(OpenGlGlfwWindowDescr& descr);
  OpenGlGlfwWindow(const OpenGlGlfwWindow&);
  OpenGlGlfwWindow(OpenGlGlfwWindow&&) noexcept;
  OpenGlGlfwWindow& operator=(const OpenGlGlfwWindow&);
  OpenGlGlfwWindow& operator=(OpenGlGlfwWindow&&) noexcept;
  virtual ~OpenGlGlfwWindow() = default;

  virtual void Initialize() override;
  virtual void SetGlfwHints() override;
  void SwapBuffers() const;

  virtual bool IsVSync() const noexcept;
  virtual void SetVSync(bool is_vsync);

 private:
  bool is_vsync_ = true;
  unsigned int opengl_major_version_ = 0;
  unsigned int opengl_minor_version_ = 0;
};
}  // namespace gl
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_WINDOW_OPENGL_OPENGL_GLFW_WINDOW_H_