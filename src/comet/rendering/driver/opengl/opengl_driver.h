// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_OPENGL_OPENGL_DRIVER_H_
#define COMET_COMET_RENDERING_DRIVER_OPENGL_OPENGL_DRIVER_H_

#include "comet/core/essentials.h"
#include "comet/event/event.h"
#include "comet/event/event_manager.h"
#include "comet/rendering/driver/driver.h"
#include "comet/rendering/driver/opengl/data/opengl_frame.h"
#include "comet/rendering/driver/opengl/handler/opengl_material_handler.h"
#include "comet/rendering/driver/opengl/handler/opengl_mesh_handler.h"
#include "comet/rendering/driver/opengl/handler/opengl_render_proxy_handler.h"
#include "comet/rendering/driver/opengl/handler/opengl_shader_handler.h"
#include "comet/rendering/driver/opengl/handler/opengl_shader_module_handler.h"
#include "comet/rendering/driver/opengl/handler/opengl_view_handler.h"
#include "comet/rendering/rendering_common.h"
#include "comet/rendering/window/glfw/opengl/opengl_glfw_window.h"

namespace comet {
namespace rendering {
namespace gl {
struct OpenGlDriverDescr : DriverDescr {
  u8 opengl_major_version{0};
  u8 opengl_minor_version{0};
};

class OpenGlDriver : public Driver {
 public:
  explicit OpenGlDriver(const OpenGlDriverDescr& descr);
  OpenGlDriver(const OpenGlDriver&) = delete;
  OpenGlDriver(OpenGlDriver&&) = delete;
  OpenGlDriver& operator=(const OpenGlDriver&) = delete;
  OpenGlDriver& operator=(OpenGlDriver&&) = delete;
  virtual ~OpenGlDriver() = default;

  void Initialize() override;
  void Shutdown() override;
  void Update(time::Interpolation interpolation) override;
  DriverType GetType() const noexcept override;
  u32 GetDrawCount() const override;

  void InitializeHandlers();
  void DestroyHandlers();

  void SetSize(WindowSize width, WindowSize height);
  void OnEvent(const event::Event&);

  void Draw(time::Interpolation interpolation);

  Window* GetWindow() override;

 private:
#ifdef COMET_RENDERING_DRIVER_DEBUG_MODE
  static void GLAPIENTRY LogOpenGlMessage(GLenum source, GLenum type, GLuint id,
                                          GLenum severity, GLsizei length,
                                          const GLchar* message,
                                          const void* user_param);
#endif  // COMET_RENDERING_DRIVER_DEBUG_MODE

  FrameIndex frame_count_{0};

  std::unique_ptr<OpenGlGlfwWindow> window_{nullptr};
  std::unique_ptr<MaterialHandler> material_handler_{nullptr};
  std::unique_ptr<MeshHandler> mesh_handler_{nullptr};
  std::unique_ptr<RenderProxyHandler> render_proxy_handler_{nullptr};
  std::unique_ptr<ShaderHandler> shader_handler_{nullptr};
  std::unique_ptr<ShaderModuleHandler> shader_module_handler_{nullptr};
  std::unique_ptr<TextureHandler> texture_handler_{nullptr};
  std::unique_ptr<ViewHandler> view_handler_{nullptr};
};
}  // namespace gl
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_OPENGL_OPENGL_DRIVER_H_
