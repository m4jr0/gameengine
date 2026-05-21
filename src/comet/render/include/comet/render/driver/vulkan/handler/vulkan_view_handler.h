// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RENDER_DRIVER_VULKAN_HANDLER_VULKAN_VIEW_HANDLER_H_
#define COMET_RENDER_DRIVER_VULKAN_HANDLER_VULKAN_VIEW_HANDLER_H_

#include "comet/core/essentials.h"
#include "comet/core/frame/frame_packet.h"
#include "comet/core/memory/allocator/platform_allocator.h"
#include "comet/core/memory/memory.h"
#include "comet/core/type/array.h"
#include "comet/rendering/driver/vulkan/handler/vulkan_camera_handler.h"
#include "comet/rendering/driver/vulkan/handler/vulkan_handler.h"
#include "comet/rendering/driver/vulkan/handler/vulkan_lighting_handler.h"
#include "comet/rendering/driver/vulkan/handler/vulkan_mesh_handler.h"
#include "comet/rendering/driver/vulkan/handler/vulkan_render_pass_handler.h"
#include "comet/rendering/driver/vulkan/handler/vulkan_render_proxy_handler.h"
#include "comet/rendering/driver/vulkan/handler/vulkan_shader_handler.h"
#include "comet/rendering/driver/vulkan/handler/vulkan_texture_handler.h"
#include "comet/rendering/driver/vulkan/view/vulkan_view.h"
#include "comet/rendering/type/common.h"
#include "comet/rendering/type/light.h"
#include "comet/rendering/type/view.h"
#include "comet/rendering/window/glfw/vulkan/vulkan_glfw_window.h"

#ifdef COMET_DEBUG_RENDERING
#include "comet/rendering/driver/vulkan/handler/vulkan_debug_handler.h"
#endif  // COMET_DEBUG_RENDERING

namespace comet {
namespace rendering {
namespace vk {
struct ViewHandlerDescr : HandlerDescr {
  const ShadowSettings* shadow_settings{nullptr};
  CameraHandler* camera_handler{nullptr};
  ShaderHandler* shader_handler{nullptr};
  TextureHandler* texture_handler{nullptr};
  RenderPassHandler* render_pass_handler{nullptr};
  RenderProxyHandler* render_proxy_handler{nullptr};
  MeshHandler* mesh_handler{nullptr};
  LightingHandler* lighting_handler{nullptr};

#ifdef COMET_DEBUG_RENDERING
  DebugHandler* debug_handler{nullptr};
#endif  // COMET_DEBUG_RENDERING

  VulkanGlfwWindow* window{nullptr};
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

  memory::PlatformAllocator allocator_{memory::kEngineMemoryTagRendering};

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
  RenderPassHandler* render_pass_handler_{nullptr};
  RenderProxyHandler* render_proxy_handler_{nullptr};
  MeshHandler* mesh_handler_{nullptr};
  LightingHandler* lighting_handler_{nullptr};

#ifdef COMET_DEBUG_RENDERING
  DebugHandler* debug_handler_{nullptr};
#endif  // COMET_DEBUG_RENDERING

  VulkanGlfwWindow* window_{nullptr};
  Array<RenderingViewDescr>* rendering_view_descrs_{nullptr};
};
}  // namespace vk
}  // namespace rendering
}  // namespace comet

#endif  // COMET_RENDER_DRIVER_VULKAN_HANDLER_VULKAN_VIEW_HANDLER_H_