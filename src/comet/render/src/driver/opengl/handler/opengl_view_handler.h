// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RENDER_DRIVER_OPENGL_HANDLER_OPENGL_VIEW_HANDLER_H_
#define COMET_RENDER_DRIVER_OPENGL_HANDLER_OPENGL_VIEW_HANDLER_H_

#include "comet/core/essentials.h"
#include "comet/runtime/frame/frame_packet.h"
#include "comet/runtime/memory/allocator/platform_allocator.h"
#include "comet/core/memory/memory.h"
#include "comet/runtime/memory/memory_tag.h"
#include "comet/core/container/array.h"
#include "comet/render/driver/opengl/handler/opengl_camera_handler.h"
#include "comet/render/driver/opengl/handler/opengl_handler.h"
#include "comet/render/driver/opengl/handler/opengl_lighting_handler.h"
#include "comet/render/driver/opengl/handler/opengl_mesh_handler.h"
#include "comet/render/driver/opengl/handler/opengl_render_proxy_handler.h"
#include "comet/render/driver/opengl/handler/opengl_shader_handler.h"
#include "comet/render/driver/opengl/handler/opengl_texture_handler.h"
#include "comet/render/driver/opengl/view/opengl_view.h"
#include "comet/render/common.h"
#include "comet/data/light/light.h"
#include "comet/render/view.h"
#include "comet/platform/window/glfw/opengl/opengl_glfw_window.h"

#ifdef COMET_DEBUG_RENDERING
#include "comet/render/driver/opengl/handler/opengl_debug_handler.h"
#endif  // COMET_DEBUG_RENDERING

namespace comet {
namespace render {
namespace gl {
struct ViewHandlerDescr : HandlerDescr {
  const ShadowSettings* shadow_settings{nullptr};
  CameraHandler* camera_handler{nullptr};
  ShaderHandler* shader_handler{nullptr};
  TextureHandler* texture_handler{nullptr};
  RenderProxyHandler* render_proxy_handler{nullptr};
  MeshHandler* mesh_handler{nullptr};
  LightingHandler* lighting_handler{nullptr};

#ifdef COMET_DEBUG_RENDERING
  DebugHandler* debug_handler{nullptr};
#endif  // COMET_DEBUG_RENDERING

  platform::OpenGlGlfwWindow* window{nullptr};
  Array<RenderingViewDescr>* rendering_view_descrs{nullptr};
};

class ViewHandler : public Handler {
 public:
  ViewHandler() = delete;
  explicit ViewHandler(const ViewHandlerDescr& descr);
  ViewHandler(const ViewHandler&) = delete;
  ViewHandler(ViewHandler&&) = delete;
  ViewHandler& operator=(const ViewHandler&) = delete;
  ViewHandler& operator=(ViewHandler&&) = delete;
  ~ViewHandler() override = default;

  void Update(frame::FramePacket* packet);

  const View* Generate(const RenderingViewDescr& descr);
  void Destroy(usize view);
  void Destroy(View* view);

  void SetSize(WindowSize width, WindowSize height);

  const View* Get(usize index) const;
  const View* TryGet(usize index) const;

 protected:
  void OnInitialize() override;
  void OnShutdown() override;

 private:
  View* Get(usize index);
  View* TryGet(usize index);

  void Destroy(View* view, bool is_destroying_handler);

  void PrepareFrameGpuData(frame::FramePacket* packet);

  void RunView(const memory::UniquePtr<View>& view,
               const ViewUpdate& update) const;

  bool ShouldDrawViewForCamera(const View& view,
                               const ViewUpdate& update) const noexcept;

  void RunSparseUpload();
  void RunForwardCull(frame::FramePacket* packet);
  void UpdateShadowCullShaderPassData();
  void RunShadowCull(const ShadowRenderJob& job, u32 shadow_job_index);
  void RunShadowCulls();
  void RunCullingDebugGeneration();

  ViewUpdate GenerateViewUpdate(frame::FramePacket* packet,
                                const CameraView* camera_view,
                                usize camera_index) const;

  memory::PlatformAllocator allocator_{kEngineMemoryTagRender};

  Array<memory::UniquePtr<View>> offscreen_views_{};
  Array<memory::UniquePtr<View>> scene_views_{};
  Array<memory::UniquePtr<View>> overlay_views_{};

  ShaderHandle forward_cull_shader_{};
  ShaderHandle sparse_upload_shader_{};
  ShaderHandle shadow_cull_shader_{};
  ShaderHandle rendering_debug_generate_shader_{};

  FrameIndex prepared_frame_{kInvalidFrameIndex};

  const ShadowSettings* shadow_settings_{nullptr};
  CameraHandler* camera_handler_{nullptr};
  ShaderHandler* shader_handler_{nullptr};
  TextureHandler* texture_handler_{nullptr};
  RenderProxyHandler* render_proxy_handler_{nullptr};
  MeshHandler* mesh_handler_{nullptr};
  LightingHandler* lighting_handler_{nullptr};

#ifdef COMET_DEBUG_RENDERING
  DebugHandler* debug_handler_{nullptr};
#endif  // COMET_DEBUG_RENDERING

  platform::OpenGlGlfwWindow* window_{nullptr};
  Array<RenderingViewDescr>* rendering_view_descrs_{nullptr};
};
}  // namespace gl
}  // namespace render
}  // namespace comet

#endif  // COMET_RENDER_DRIVER_OPENGL_HANDLER_OPENGL_VIEW_HANDLER_H_