// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RENDER_DRIVER_OPENGL_OPENGL_DRIVER_H_
#define COMET_RENDER_DRIVER_OPENGL_OPENGL_DRIVER_H_

// External. ///////////////////////////////////////////////////////////////////
#include "glad/glad.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/essentials.h"
#include "comet/runtime/frame/frame_packet.h"
#include "comet/platform/window/window_common.h"
#include "comet/core/memory/memory.h"
#include "comet/runtime/memory/memory_tag.h"
#include "comet/runtime/event/event.h"
#include "comet/runtime/event/event_manager.h"
#include "comet/render/driver/driver.h"
#include "comet/render/driver/opengl/handler/opengl_camera_handler.h"
#include "comet/render/driver/opengl/handler/opengl_lighting_handler.h"
#include "comet/render/driver/opengl/handler/opengl_material_handler.h"
#include "comet/render/driver/opengl/handler/opengl_mesh_handler.h"
#include "comet/render/driver/opengl/handler/opengl_render_proxy_handler.h"
#include "comet/render/driver/opengl/handler/opengl_sampler_handler.h"
#include "comet/render/driver/opengl/handler/opengl_shader_handler.h"
#include "comet/render/driver/opengl/handler/opengl_shader_module_handler.h"
#include "comet/render/driver/opengl/handler/opengl_texture_handler.h"
#include "comet/render/driver/opengl/handler/opengl_view_handler.h"
#include "comet/render/driver/opengl/opengl_frame_state.h"
#include "comet/render/common.h"
#include "comet/platform/window/glfw/opengl/opengl_glfw_window.h"

#ifdef COMET_DEBUG_RENDERING
#include "comet/render/driver/opengl/handler/opengl_debug_handler.h"
#endif  // COMET_DEBUG_RENDERING

namespace comet {
namespace render {
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
  ~OpenGlDriver() override = default;

  void Update(frame::FramePacket* packet) override;

  DriverType GetType() const noexcept override;

  void SetSize(WindowSize width, WindowSize height);
  void OnEvent(const event::Event& event);

  void RegisterEvents();
  void UnregisterEvents();

  platform::Window* GetWindow() override;
  u32 GetDrawCount() const override;

 protected:
  void OnInitialize() override;
  void OnShutdown() override;

 private:
  void InitializeHandlers();
  void DestroyHandlers();

  void ApplyWindowResize();

  void PreDraw(frame::FramePacket* packet);
  void PostDraw(const frame::FramePacket* packet);
  void Draw(frame::FramePacket* packet);

  void HandlePresentationState(frame::FramePacket* packet);
  void UpdateGpuSceneState(frame::FramePacket* packet);
  void RecordFrame(frame::FramePacket* packet);

#ifdef COMET_DEBUG_RENDERING
  static void GLAPIENTRY LogOpenGlMessage(GLenum source, GLenum type, GLuint id,
                                          GLenum severity, GLsizei length,
                                          const GLchar* message,
                                          const void* user_param);
#endif  // COMET_DEBUG_RENDERING

  bool is_resize_{false};

  event::EventListenerId window_resize_listener_id_{
      event::kInvalidEventListenerId};

  memory::UniquePtr<FrameState> frame_state_{nullptr};

  memory::UniquePtr<platform::OpenGlGlfwWindow> window_{nullptr};
  memory::UniquePtr<TextureHandler> texture_handler_{nullptr};
  memory::UniquePtr<ShaderModuleHandler> shader_module_handler_{nullptr};
  memory::UniquePtr<MaterialHandler> material_handler_{nullptr};
  memory::UniquePtr<MeshHandler> mesh_handler_{nullptr};
  memory::UniquePtr<SamplerHandler> sampler_handler_{nullptr};
  memory::UniquePtr<ShaderHandler> shader_handler_{nullptr};
  memory::UniquePtr<LightingHandler> lighting_handler_{nullptr};
  memory::UniquePtr<RenderProxyHandler> render_proxy_handler_{nullptr};
  memory::UniquePtr<CameraHandler> camera_handler_{nullptr};
  memory::UniquePtr<ViewHandler> view_handler_{nullptr};

#ifdef COMET_DEBUG_RENDERING
  memory::UniquePtr<DebugHandler> debug_handler_{nullptr};
#endif  // COMET_DEBUG_RENDERING
};
}  // namespace gl
}  // namespace render
}  // namespace comet

#endif  // COMET_RENDER_DRIVER_OPENGL_OPENGL_DRIVER_H_