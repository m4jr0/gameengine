// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "opengl_view_handler.h"
////////////////////////////////////////////////////////////////////////////////

// External. ///////////////////////////////////////////////////////////////////
#include <utility>
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/memory/memory_utils.h"
#include "comet/core/type_trait.h"
#include "comet/profiler/profiler.h"
#include "comet/rendering/driver/opengl/utils/opengl_shader_utils.h"
#include "comet/rendering/driver/opengl/utils/opengl_view_shader_utils.h"
#include "comet/rendering/driver/opengl/view/opengl_shadow_view.h"
#include "comet/rendering/driver/opengl/view/opengl_world_view.h"
#include "comet/rendering/label/view_label.h"

#ifdef COMET_IMGUI
#include "comet/rendering/driver/opengl/view/opengl_imgui_view.h"
#endif  // COMET_IMGUI

#ifdef COMET_DEBUG
#include "comet/debugging/rendering/rendering_debug_settings.h"
#include "comet/rendering/driver/opengl/view/opengl_debug_view.h"
#endif  // COMET_DEBUG

namespace comet {
namespace rendering {
namespace gl {
ViewHandler::ViewHandler(const ViewHandlerDescr& descr)
    : Handler{descr},
      shadow_settings_{descr.shadow_settings},
      camera_handler_{descr.camera_handler},
      shader_handler_{descr.shader_handler},
      texture_handler_{descr.texture_handler},
      render_proxy_handler_{descr.render_proxy_handler},
      mesh_handler_{descr.mesh_handler},
      lighting_handler_{descr.lighting_handler},
#ifdef COMET_DEBUG_RENDERING
      debug_handler_{descr.debug_handler},
#endif  // COMET_DEBUG_RENDERING
      window_{descr.window},
      rendering_view_descrs_{descr.rendering_view_descrs} {
  COMET_ASSERT(camera_handler_ != nullptr, "ViewHandler::ViewHandler",
               "camera handler is null");
  COMET_ASSERT(shader_handler_ != nullptr, "ViewHandler::ViewHandler",
               "shader handler is null");
  COMET_ASSERT(texture_handler_ != nullptr, "ViewHandler::ViewHandler",
               "texture handler is null");
  COMET_ASSERT(render_proxy_handler_ != nullptr, "ViewHandler::ViewHandler",
               "render proxy handler is null");
  COMET_ASSERT(mesh_handler_ != nullptr, "ViewHandler::ViewHandler",
               "mesh handler is null");
  COMET_ASSERT(lighting_handler_ != nullptr, "ViewHandler::ViewHandler",
               "lighting handler is null");
#ifdef COMET_DEBUG_RENDERING
  COMET_ASSERT(debug_handler_ != nullptr, "ViewHandler::ViewHandler",
               "debug handler is null");
#endif  // COMET_DEBUG_RENDERING
  COMET_ASSERT(window_ != nullptr, "ViewHandler::ViewHandler",
               "window is null");
  COMET_ASSERT(rendering_view_descrs_ != nullptr, "ViewHandler::ViewHandler",
               "rendering view descriptions are null");
}

void ViewHandler::Update(frame::FramePacket* packet) {
  COMET_PROFILE("ViewHandler::Update");
  COMET_ASSERT(packet != nullptr, "ViewHandler::Update",
               "frame packet is null");
  COMET_ASSERT(packet->camera_views != nullptr, "ViewHandler::Update",
               "camera views are null");
  COMET_ASSERT(!packet->camera_views->IsEmpty(), "ViewHandler::Update",
               "camera views are empty");

  const auto& camera_views{*packet->camera_views};
  const auto primary_update{GenerateViewUpdate(
      packet, packet->GetMainCameraView(), packet->main_camera_view_index)};

  PrepareFrameGpuData(packet);

  for (const auto& view : offscreen_views_) {
    RunView(view, primary_update);
  }

  for (const auto& view : scene_views_) {
    view->Prepare(primary_update);

    bool has_drawable_camera{false};

    for (u32 i{0}; i < camera_views.GetSize(); ++i) {
      const auto update{GenerateViewUpdate(packet, &camera_views[i], i)};

      if (ShouldDrawViewForCamera(*view, update)) {
        has_drawable_camera = true;
        break;
      }
    }

    if (!has_drawable_camera) {
      continue;
    }

    view->Begin(primary_update);

    for (u32 i{0}; i < camera_views.GetSize(); ++i) {
      const auto update{GenerateViewUpdate(packet, &camera_views[i], i)};

      if (!ShouldDrawViewForCamera(*view, update)) {
        continue;
      }

      view->Draw(update);
    }

    view->End(primary_update);
  }

  auto overlay_update{primary_update};
  overlay_update.camera_flags = kCameraFlagBitsAll;
  overlay_update.camera_index = 0;
  overlay_update.viewport = ViewportRect{
      .x = .0f,
      .y = .0f,
      .width = static_cast<f32>(window_->GetWidth()),
      .height = static_cast<f32>(window_->GetHeight()),
  };

  for (const auto& view : overlay_views_) {
    RunView(view, overlay_update);
  }
}

const View* ViewHandler::Generate(const RenderingViewDescr& descr) {
  memory::UniquePtr<View> view{nullptr};

  switch (descr.type) {
    case RenderingViewType::World: {
      WorldViewDescr view_descr{};
      view_descr.id = descr.id;
      view_descr.render_stage = ViewRenderStage::SceneBase;

      view_descr.pass_descr.flags = kViewPassFlagBitsSwapchainTarget |
                                    kViewPassFlagBitsHasColor |
                                    kViewPassFlagBitsHasDepth;

      view_descr.pass_descr.color_load_op = ViewLoadOp::Clear;
      view_descr.pass_descr.color_store_op = ViewStoreOp::Store;
      view_descr.pass_descr.depth_load_op = ViewLoadOp::Clear;
      view_descr.pass_descr.depth_store_op = ViewStoreOp::DontCare;
      view_descr.pass_descr.final_color_op = ViewFinalColorOp::Keep;

      view_descr.width = descr.width;
      view_descr.height = descr.height;
      memory::CopyMemory(view_descr.clear_color, descr.clear_color,
                         sizeof(descr.clear_color[0]) * 4);

      view_descr.frame_state = frame_state_;
      view_descr.shadow_settings = shadow_settings_;
      view_descr.camera_handler = camera_handler_;
      view_descr.shader_handler = shader_handler_;
      view_descr.texture_handler = texture_handler_;
      view_descr.render_proxy_handler = render_proxy_handler_;
      view_descr.mesh_handler = mesh_handler_;
      view_descr.lighting_handler = lighting_handler_;

      view = std::make_unique<WorldView>(view_descr);
      break;
    }

    case RenderingViewType::Shadow: {
      ShadowViewDescr view_descr{};
      view_descr.id = descr.id;
      view_descr.render_stage = ViewRenderStage::Offscreen;

      view_descr.pass_descr.flags =
          kViewPassFlagBitsOffscreenTarget | kViewPassFlagBitsHasDepth;

      view_descr.pass_descr.depth_load_op = ViewLoadOp::Clear;
      view_descr.pass_descr.depth_store_op = ViewStoreOp::Store;

      view_descr.width = descr.width;
      view_descr.height = descr.height;
      memory::CopyMemory(view_descr.clear_color, descr.clear_color,
                         sizeof(descr.clear_color[0]) * 4);

      view_descr.frame_state = frame_state_;
      view_descr.shader_handler = shader_handler_;
      view_descr.render_proxy_handler = render_proxy_handler_;
      view_descr.lighting_handler = lighting_handler_;
      view_descr.mesh_handler = mesh_handler_;

      view = std::make_unique<ShadowView>(view_descr);
      break;
    }

#ifdef COMET_DEBUG_VIEW
    case RenderingViewType::Debug: {
      DebugViewDescr view_descr{};
      view_descr.id = descr.id;
      view_descr.render_stage = ViewRenderStage::SceneOverlay;

      view_descr.pass_descr.flags = kViewPassFlagBitsSwapchainTarget |
                                    kViewPassFlagBitsHasColor |
                                    kViewPassFlagBitsHasDepth;

      view_descr.pass_descr.color_load_op = ViewLoadOp::Load;
      view_descr.pass_descr.color_store_op = ViewStoreOp::Store;
      view_descr.pass_descr.depth_load_op = ViewLoadOp::Load;
      view_descr.pass_descr.depth_store_op = ViewStoreOp::DontCare;
      view_descr.pass_descr.final_color_op = ViewFinalColorOp::Keep;

      view_descr.width = descr.width;
      view_descr.height = descr.height;
      memory::CopyMemory(view_descr.clear_color, descr.clear_color,
                         sizeof(descr.clear_color[0]) * 4);

      view_descr.frame_state = frame_state_;
      view_descr.camera_handler = camera_handler_;
      view_descr.shader_handler = shader_handler_;
      view_descr.render_proxy_handler = render_proxy_handler_;

#ifdef COMET_DEBUG_RENDERING
      view_descr.debug_handler = debug_handler_;
#endif  // COMET_DEBUG_RENDERING

      view = std::make_unique<DebugView>(view_descr);
      break;
    }
#endif  // COMET_DEBUG_VIEW

#ifdef COMET_IMGUI
    case RenderingViewType::ImGui: {
      ImGuiViewDescr view_descr{};
      view_descr.id = descr.id;
      view_descr.render_stage = ViewRenderStage::Overlay;

      view_descr.pass_descr.flags = kViewPassFlagBitsSwapchainTarget |
                                    kViewPassFlagBitsOverlayTarget |
                                    kViewPassFlagBitsHasColor;

      view_descr.pass_descr.color_load_op = ViewLoadOp::Load;
      view_descr.pass_descr.color_store_op = ViewStoreOp::Store;
      view_descr.pass_descr.final_color_op = ViewFinalColorOp::Present;

      view_descr.width = descr.width;
      view_descr.height = descr.height;
      memory::CopyMemory(view_descr.clear_color, descr.clear_color,
                         sizeof(descr.clear_color[0]) * 4);

      view_descr.frame_state = frame_state_;
      view_descr.window = window_;

      view = std::make_unique<ImGuiView>(view_descr);
      break;
    }
#endif  // COMET_IMGUI

    default: {
      COMET_ASSERT(false, "ViewHandler::Generate", "view type is unsupported",
                   "view_type", GetRenderingViewTypeLabel(descr.type),
                   "view_type_value", ToUnderlying(descr.type));
      return nullptr;
    }
  }

  COMET_ASSERT(view != nullptr, "ViewHandler::Generate",
               "generated view is null", "view_type",
               GetRenderingViewTypeLabel(descr.type), "view_type_value",
               ToUnderlying(descr.type), "view_id", descr.id);

  auto* generated_view{view.get()};
  view->Initialize();

  switch (generated_view->GetRenderStage()) {
    case ViewRenderStage::Offscreen: {
      offscreen_views_.PushLast(std::move(view));
      break;
    }

    case ViewRenderStage::SceneBase:
    case ViewRenderStage::SceneOverlay: {
      COMET_ASSERT(generated_view->IsSwapchainTarget(), "ViewHandler::Generate",
                   "scene view is not a swapchain target", "view_id",
                   generated_view->GetId());
      scene_views_.PushLast(std::move(view));
      break;
    }

    case ViewRenderStage::Overlay: {
      overlay_views_.PushLast(std::move(view));
      break;
    }

    default: {
      COMET_ASSERT(false, "ViewHandler::Generate",
                   "generated view has unsupported render stage", "view_id",
                   generated_view->GetId());
      break;
    }
  }

  return generated_view;
}

void ViewHandler::Destroy(usize index) { Destroy(Get(index)); }

void ViewHandler::Destroy(View* view) { Destroy(view, false); }

void ViewHandler::SetSize(WindowSize width, WindowSize height) {
  for (auto& view : scene_views_) {
    view->SetSize(width, height);
  }

  for (auto& view : overlay_views_) {
    view->SetSize(width, height);
  }
}

const View* ViewHandler::Get(usize index) const {
  auto* view{TryGet(index)};
  COMET_ASSERT(view != nullptr, "ViewHandler::Get", "view not found", "index",
               index);
  return view;
}

const View* ViewHandler::TryGet(usize index) const {
  for (const auto* views :
       {&offscreen_views_, &scene_views_, &overlay_views_}) {
    if (index < views->GetSize()) {
      return (*views)[index].get();
    }

    index -= views->GetSize();
  }

  return nullptr;
}

void ViewHandler::OnInitialize() {
  const auto capacity{rendering_view_descrs_->GetSize()};

  offscreen_views_ =
      Array<memory::UniquePtr<View>>::WithCapacity(&allocator_, capacity);
  scene_views_ =
      Array<memory::UniquePtr<View>>::WithCapacity(&allocator_, capacity);
  overlay_views_ =
      Array<memory::UniquePtr<View>>::WithCapacity(&allocator_, capacity);

  {
    ShaderDescr shader_descr{};
    shader_descr.shader_resource_id =
        resource::GenerateResourceIdFromPath<resource::ShaderResource>(
            COMET_TCHAR("shaders/opengl/forward_cull_shader.gl.cshader"));
    shader_descr.bind_type = PipelineBindType::Compute;

    forward_cull_shader_ = shader_handler_->GetOrGenerate(shader_descr);
  }

  {
    ShaderDescr shader_descr{};
    shader_descr.shader_resource_id =
        resource::GenerateResourceIdFromPath<resource::ShaderResource>(
            COMET_TCHAR("shaders/opengl/sparse_upload.gl.cshader"));
    shader_descr.bind_type = PipelineBindType::Compute;

    sparse_upload_shader_ = shader_handler_->GetOrGenerate(shader_descr);
  }

  {
    ShaderDescr shader_descr{};
    shader_descr.shader_resource_id =
        resource::GenerateResourceIdFromPath<resource::ShaderResource>(
            COMET_TCHAR("shaders/opengl/shadow_cull_shader.gl.cshader"));
    shader_descr.bind_type = PipelineBindType::Compute;

    shadow_cull_shader_ = shader_handler_->GetOrGenerate(shader_descr);
  }

#ifdef COMET_DEBUG_RENDERING
  {
    ShaderDescr shader_descr{};
    shader_descr.shader_resource_id =
        resource::GenerateResourceIdFromPath<resource::ShaderResource>(
            COMET_TCHAR("shaders/opengl/rendering_debug_generate.gl.cshader"));
    shader_descr.bind_type = PipelineBindType::Compute;

    rendering_debug_generate_shader_ =
        shader_handler_->GetOrGenerate(shader_descr);
  }
#endif  // COMET_DEBUG_RENDERING

  for (const auto& view_descr : *rendering_view_descrs_) {
    Generate(view_descr);
  }
}

void ViewHandler::OnShutdown() {
  const auto destroy_all = [](Array<memory::UniquePtr<View>>& views) {
    for (auto& view : views) {
      view->Destroy();
    }

    views.Release();
  };

  destroy_all(overlay_views_);
  destroy_all(scene_views_);
  destroy_all(offscreen_views_);

  if (forward_cull_shader_) {
    shader_handler_->Destroy(forward_cull_shader_);
    forward_cull_shader_.Invalidate();
  }

  if (sparse_upload_shader_) {
    shader_handler_->Destroy(sparse_upload_shader_);
    sparse_upload_shader_.Invalidate();
  }

  if (shadow_cull_shader_) {
    shader_handler_->Destroy(shadow_cull_shader_);
    shadow_cull_shader_.Invalidate();
  }

  if (rendering_debug_generate_shader_) {
    shader_handler_->Destroy(rendering_debug_generate_shader_);
    rendering_debug_generate_shader_.Invalidate();
  }

  frame_state_ = nullptr;
  shadow_settings_ = nullptr;
  camera_handler_ = nullptr;
  shader_handler_ = nullptr;
  texture_handler_ = nullptr;
  render_proxy_handler_ = nullptr;
  mesh_handler_ = nullptr;
  lighting_handler_ = nullptr;
#ifdef COMET_DEBUG_RENDERING
  debug_handler_ = nullptr;
#endif  // COMET_DEBUG_RENDERING
  window_ = nullptr;
  rendering_view_descrs_ = nullptr;
}

View* ViewHandler::Get(usize index) {
  auto* view{TryGet(index)};
  COMET_ASSERT(view != nullptr, "ViewHandler::Get", "view not found", "index",
               index);
  return view;
}

View* ViewHandler::TryGet(usize index) {
  for (auto* views : {&offscreen_views_, &scene_views_, &overlay_views_}) {
    if (index < views->GetSize()) {
      return (*views)[index].get();
    }

    index -= views->GetSize();
  }

  return nullptr;
}

void ViewHandler::Destroy(View* view, bool is_destroying_handler) {
  COMET_ASSERT(view != nullptr, "ViewHandler::Destroy", "view is null");

  if (is_destroying_handler) {
    view->Destroy();
    return;
  }

  const auto view_id{view->GetId()};

  const auto destroy_from = [&](Array<memory::UniquePtr<View>>& views) {
    for (usize i{0}; i < views.GetSize(); ++i) {
      if (views[i]->GetId() != view_id) {
        continue;
      }

      views[i]->Destroy();
      views.RemoveFromPos(views.begin() + i);
      return true;
    }

    return false;
  };

  if (destroy_from(offscreen_views_) || destroy_from(scene_views_) ||
      destroy_from(overlay_views_)) {
    return;
  }

  COMET_ASSERT(false, "ViewHandler::Destroy", "view was not found in handler",
               "view_id", view_id);
}

void ViewHandler::PrepareFrameGpuData(frame::FramePacket* packet) {
  COMET_PROFILE("ViewHandler::PrepareFrameGpuData");
  COMET_ASSERT(packet != nullptr, "ViewHandler::PrepareFrameGpuData",
               "frame packet is null");

  const auto frame{frame_state_->GetFrameCount()};

  if (prepared_frame_ == frame) {
    return;
  }

  prepared_frame_ = frame;

  packet->draw_count = render_proxy_handler_->GetRenderProxyCount();

  if (packet->draw_count == 0) {
    return;
  }

  RunSparseUpload();
  RunForwardCull(packet);

#ifdef COMET_DEBUG_RENDERING
  debug_handler_->ClearDebugCameraFrustums();

  const auto* camera_views = packet->camera_views;

  for (u32 i = 0; i < camera_views->GetSize(); ++i) {
    const auto& cam = (*camera_views)[i];

    const math::Mat4 view_proj =
        cam.data.projection_matrix * cam.data.view_matrix;

    debug_handler_->SetDebugCameraFrustum(i, view_proj);
  }

  debug_handler_->ClearDebugCascadeFrustumCorners();
  debug_handler_->ClearDebugLightFrustums();
#endif  // COMET_DEBUG_RENDERING

  RunShadowCulls();

#ifdef COMET_DEBUG_RENDERING
  RunCullingDebugGeneration();
#endif  // COMET_DEBUG_RENDERING
}

void ViewHandler::RunView(const memory::UniquePtr<View>& view,
                          const ViewUpdate& update) const {
  view->Prepare(update);
  view->Begin(update);
  view->Draw(update);
  view->End(update);
}

bool ViewHandler::ShouldDrawViewForCamera(
    const View& view, const ViewUpdate& update) const noexcept {
  return (view.GetCameraFlags() & update.camera_flags) != 0;
}

void ViewHandler::RunSparseUpload() {
  COMET_PROFILE("ViewHandler::RunSparseUpload");

  const auto frame_index{frame_state_->GetFrameInFlightIndex()};

  if (!render_proxy_handler_->HasPendingSparseUpload(frame_index)) {
    return;
  }

  const auto gpu_data{
      render_proxy_handler_->GetSparseUploadGpuData(frame_index)};

  static constexpr usize kShaderBufferBindingCapacity{3};
  auto& buffer_bindings{*COMET_FRAME_ARRAY_WITH_CAPACITY(
      ShaderBufferBindingUpdate, kShaderBufferBindingCapacity)};

  AddBufferBinding(
      buffer_bindings,
      shader_handler_->GetBindingIndex(
          sparse_upload_shader_, shaderconsts::kGlobalSet,
          worldsparseuploadshaderconsts::kSparseUploadWordIndicesBinding),
      gpu_data.ssbo_sparse_upload_word_indices_handle,
      gpu_data.ssbo_sparse_upload_word_indices_size);

  AddBufferBinding(
      buffer_bindings,
      shader_handler_->GetBindingIndex(
          sparse_upload_shader_, shaderconsts::kGlobalSet,
          worldsparseuploadshaderconsts::kSparseUploadSourceWordsBinding),
      gpu_data.ssbo_source_words_handle, gpu_data.ssbo_source_words_size);

  AddBufferBinding(
      buffer_bindings,
      shader_handler_->GetBindingIndex(
          sparse_upload_shader_, shaderconsts::kGlobalSet,
          worldsparseuploadshaderconsts::kSparseUploadDestinationWordsBinding),
      gpu_data.ssbo_destination_words_handle,
      gpu_data.ssbo_destination_words_size);

  ShaderGlobalUpdate global_update{};
  global_update.buffer_bindings = &buffer_bindings;
  shader_handler_->UpdateGlobals(sparse_upload_shader_, global_update);

  static constexpr usize kShaderPushConstantBlockCapacity{1};
  auto& blocks{*COMET_FRAME_ARRAY_WITH_CAPACITY(
      ShaderPushConstantBlockUpdate, kShaderPushConstantBlockCapacity)};
  auto& block{blocks.EmplaceLast()};
  block.block_index = worldsparseuploadshaderconsts::kCountPushConstantIndex;
  block.data = &gpu_data.word_count;
  block.size = sizeof(gpu_data.word_count);

  ShaderPushConstantsUpdate push_constants{};
  push_constants.blocks = &blocks;

  shader_handler_->Bind(sparse_upload_shader_);
  shader_handler_->PushConstants(sparse_upload_shader_, push_constants);

  glDispatchCompute(
      render_proxy_handler_->GetSparseUploadGroupCount(frame_index), 1, 1);

  glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT |
                  GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT | GL_COMMAND_BARRIER_BIT);
}

void ViewHandler::RunForwardCull(frame::FramePacket* packet) {
  COMET_PROFILE("ViewHandler::RunForwardCull");

  const auto frame_index{frame_state_->GetFrameInFlightIndex()};
  const auto gpu_data{render_proxy_handler_->GetGpuData(frame_index)};

  COMET_ASSERT(
      gpu_data.ssbo_indirect_proxies_handle != kInvalidGlNativeStorageHandle,
      "WorldView::RunCull", "cull indirect buffer handle is invalid",
      "frame_index", frame_index);
  COMET_ASSERT(
      gpu_data.ssbo_proxy_instances_handle != kInvalidGlNativeStorageHandle,
      "WorldView::RunCull", "cull proxy instances buffer handle is invalid",
      "frame_index", frame_index);
  COMET_ASSERT(gpu_data.ssbo_proxy_ids_handle != kInvalidGlNativeStorageHandle,
               "WorldView::RunCull", "cull proxy ids buffer handle is invalid",
               "frame_index", frame_index);

#ifdef COMET_DEBUG_RENDERING
  debug_handler_->PrepareCullDebugWrite(frame_index);
#endif  // COMET_DEBUG_RENDERING

  static constexpr usize kShaderBufferBindingCapacity{7};
  auto& buffer_bindings{*COMET_FRAME_ARRAY_WITH_CAPACITY(
      ShaderBufferBindingUpdate, kShaderBufferBindingCapacity)};

  AddBufferBinding(buffer_bindings,
                   shader_handler_->GetBindingIndex(
                       forward_cull_shader_, shaderconsts::kPassSet,
                       sharedshaderconsts::kProxyLocalDatasBinding),
                   gpu_data.ssbo_proxy_local_datas_handle,
                   gpu_data.ssbo_proxy_local_datas_size);

  AddBufferBinding(buffer_bindings,
                   shader_handler_->GetBindingIndex(
                       forward_cull_shader_, shaderconsts::kPassSet,
                       sharedshaderconsts::kProxyIdsBinding),
                   gpu_data.ssbo_proxy_ids_handle,
                   gpu_data.ssbo_proxy_ids_size);

  AddBufferBinding(buffer_bindings,
                   shader_handler_->GetBindingIndex(
                       forward_cull_shader_, shaderconsts::kPassSet,
                       sharedshaderconsts::kProxyInstancesBinding),
                   gpu_data.ssbo_proxy_instances_handle,
                   gpu_data.ssbo_proxy_instances_size);

  AddBufferBinding(buffer_bindings,
                   shader_handler_->GetBindingIndex(
                       forward_cull_shader_, shaderconsts::kPassSet,
                       sharedshaderconsts::kIndirectProxiesBinding),
                   gpu_data.ssbo_indirect_proxies_handle,
                   gpu_data.ssbo_indirect_proxies_size);

#ifdef COMET_DEBUG_RENDERING
  const auto debug_gpu_data{debug_handler_->GetGpuData(frame_index)};

  if (debug_gpu_data.ssbo_debug_data_handle != kInvalidGlNativeStorageHandle) {
    AddBufferBinding(buffer_bindings,
                     shader_handler_->GetBindingIndex(
                         forward_cull_shader_, shaderconsts::kPassSet,
                         sharedshaderconsts::kDebugDataBinding),
                     debug_gpu_data.ssbo_debug_data_handle,
                     debug_gpu_data.ssbo_debug_data_size);
  }

  if (debug_gpu_data.ssbo_debug_aabbs_handle != kInvalidGlNativeStorageHandle) {
    AddBufferBinding(buffer_bindings,
                     shader_handler_->GetBindingIndex(
                         forward_cull_shader_, shaderconsts::kPassSet,
                         sharedshaderconsts::kDebugAabbsBinding),
                     debug_gpu_data.ssbo_debug_aabbs_handle,
                     debug_gpu_data.ssbo_debug_aabbs_size);
  }
#endif  // COMET_DEBUG_RENDERING

  AddCameraBufferBinding(shader_handler_, camera_handler_, forward_cull_shader_,
                         frame_index, buffer_bindings);

  ShaderPassUpdate pass_update{};
  pass_update.buffer_bindings = &buffer_bindings;
  shader_handler_->UpdatePass(forward_cull_shader_, pass_update);

  static constexpr usize kShaderPushConstantBlockCapacity{1};
  auto& blocks{*COMET_FRAME_ARRAY_WITH_CAPACITY(
      ShaderPushConstantBlockUpdate, kShaderPushConstantBlockCapacity)};

  struct ForwardCullPushConstants {
    u32 draw_count{0};
    u32 camera_index{0};
  };

  ForwardCullPushConstants push_data{};
  push_data.draw_count = static_cast<u32>(packet->draw_count);
  push_data.camera_index = static_cast<u32>(packet->main_camera_view_index);

  auto& block{blocks.EmplaceLast()};
  block.block_index = worldcullshaderconsts::kDrawCountPushConstantIndex;
  block.data = &push_data;
  block.size = sizeof(push_data);

  ShaderPushConstantsUpdate push_constants{};
  push_constants.blocks = &blocks;

  shader_handler_->Bind(forward_cull_shader_);
  shader_handler_->PushConstants(forward_cull_shader_, push_constants);

  glDispatchCompute(render_proxy_handler_->GetCullGroupCount(), 1, 1);

  glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT |
                  GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT | GL_COMMAND_BARRIER_BIT);

#ifdef COMET_DEBUG_RENDERING
  glMemoryBarrier(GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT);
#endif  // COMET_DEBUG_RENDERING
}

void ViewHandler::UpdateShadowCullShaderPassData() {
  const auto frame_index{frame_state_->GetFrameInFlightIndex()};
  const auto gpu_data{render_proxy_handler_->GetGpuData(frame_index)};
  const auto shadow_proxy_ids{
      render_proxy_handler_->GetShadowProxyIdsBuffer(frame_index)};
  const auto shadow_indirect{
      render_proxy_handler_->GetShadowIndirectBuffer(frame_index)};

  static constexpr usize kShaderBufferBindingCapacity{4};
  auto& buffer_bindings{*COMET_FRAME_ARRAY_WITH_CAPACITY(
      ShaderBufferBindingUpdate, kShaderBufferBindingCapacity)};

  AddBufferBinding(buffer_bindings,
                   shader_handler_->GetBindingIndex(
                       shadow_cull_shader_, shaderconsts::kPassSet,
                       sharedshaderconsts::kProxyLocalDatasBinding),
                   gpu_data.ssbo_proxy_local_datas_handle,
                   gpu_data.ssbo_proxy_local_datas_size);

  AddBufferBinding(buffer_bindings,
                   shader_handler_->GetBindingIndex(
                       shadow_cull_shader_, shaderconsts::kPassSet,
                       sharedshaderconsts::kProxyIdsBinding),
                   shadow_proxy_ids.native_handle, shadow_proxy_ids.size);

  AddBufferBinding(buffer_bindings,
                   shader_handler_->GetBindingIndex(
                       shadow_cull_shader_, shaderconsts::kPassSet,
                       sharedshaderconsts::kProxyInstancesBinding),
                   gpu_data.ssbo_proxy_instances_handle,
                   gpu_data.ssbo_proxy_instances_size);

  AddBufferBinding(buffer_bindings,
                   shader_handler_->GetBindingIndex(
                       shadow_cull_shader_, shaderconsts::kPassSet,
                       sharedshaderconsts::kIndirectProxiesBinding),
                   shadow_indirect.native_handle, shadow_indirect.size);

  ShaderPassUpdate pass_update{};
  pass_update.buffer_bindings = &buffer_bindings;
  shader_handler_->UpdatePass(shadow_cull_shader_, pass_update);
}

void ViewHandler::RunShadowCull(const ShadowRenderJob& job,
                                u32 shadow_job_index) {
  COMET_PROFILE("ViewHandler::RunShadowCull");

  const auto frame_index{frame_state_->GetFrameInFlightIndex()};
  const auto& range{
      render_proxy_handler_->GetShadowCullRange(shadow_job_index)};
  [[maybe_unused]] const auto shadow_proxy_ids{
      render_proxy_handler_->GetShadowProxyIdsBuffer(frame_index)};
  [[maybe_unused]] const auto shadow_indirect{
      render_proxy_handler_->GetShadowIndirectBuffer(frame_index)};

  if (range.batch_count == 0 || range.instance_capacity == 0) {
    return;
  }

  COMET_ASSERT(shadow_proxy_ids.native_handle != kInvalidGlNativeStorageHandle,
               "ViewHandler::RunShadowCull",
               "shadow proxy ids buffer handle is invalid");
  COMET_ASSERT(shadow_indirect.native_handle != kInvalidGlNativeStorageHandle,
               "ViewHandler::RunShadowCull",
               "shadow indirect buffer handle is invalid");

  struct ShadowCullPushConstants {
    u32 draw_count{0};
    u32 indirect_offset{0};
    u32 proxy_id_offset{0};
    u32 padding{0};
    math::Mat4 view_proj{1.0f};
  };

  auto* push_data{COMET_FRAME_ALLOC_ONE_AND_POPULATE(
      ShadowCullPushConstants,
      render_proxy_handler_->GetRenderProxyInstanceCount(),
      range.indirect_offset, range.proxy_id_offset, 0, job.view_proj)};

  static constexpr usize kShaderPushConstantBlockCapacity{1};
  auto& blocks{*COMET_FRAME_ARRAY_WITH_CAPACITY(
      ShaderPushConstantBlockUpdate, kShaderPushConstantBlockCapacity)};

  auto& block{blocks.EmplaceLast()};
  block.block_index = 0;
  block.data = push_data;
  block.size = sizeof(ShadowCullPushConstants);

  ShaderPushConstantsUpdate push_constants{};
  push_constants.blocks = &blocks;

  shader_handler_->Bind(shadow_cull_shader_);
  shader_handler_->PushConstants(shadow_cull_shader_, push_constants);

  glDispatchCompute(
      static_cast<u32>((render_proxy_handler_->GetRenderProxyInstanceCount() +
                        kShaderLocalSize - 1) /
                       kShaderLocalSize),
      1, 1);

  glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT |
                  GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT | GL_COMMAND_BARRIER_BIT);
}

void ViewHandler::RunShadowCulls() {
  UpdateShadowCullShaderPassData();

  u32 shadow_job_index{0};
  const auto* render_jobs{lighting_handler_->GetRenderJobs()};

  if (render_jobs == nullptr || render_jobs->IsEmpty()) {
    return;
  }

  render_proxy_handler_->PrepareShadowCullData(
      frame_state_->GetFrameInFlightIndex(),
      static_cast<u32>(render_jobs->GetSize()));

  for (const auto& job : *render_jobs) {
    COMET_ASSERT(job.resource != nullptr, "ViewHandler::Prepare",
                 "shadow render job resource is null");

#ifdef COMET_DEBUG_RENDERING
    if (job.type == ShadowType::DirectionalOrtho) {
      debug_handler_->SetDebugCascadeFrustumCorners(shadow_job_index,
                                                    job.cascade_corners);
    }

    debug_handler_->SetDebugLightFrustum(shadow_job_index, job.view_proj);
#endif  // COMET_DEBUG_RENDERING

    RunShadowCull(job, shadow_job_index);
    ++shadow_job_index;
  }
}

void ViewHandler::RunCullingDebugGeneration() {
#ifdef COMET_DEBUG_RENDERING
  COMET_PROFILE("ViewHandler::RunCullingDebugGeneration");

  const auto aabb_count{debug_handler_->GetDebugAabbCount()};
  const auto camera_frustum_count{debug_handler_->GetDebugCameraFrustumCount()};
  const auto cascade_frustum_count{
      debug_handler_->GetDebugCascadeFrustumCount()};
  const auto light_frustum_count{debug_handler_->GetDebugLightFrustumCount()};

  const auto work_count{aabb_count + light_frustum_count +
                        cascade_frustum_count + camera_frustum_count};

  if (work_count == 0) {
    return;
  }

  const auto frame_index{frame_state_->GetFrameInFlightIndex()};
  const auto debug_gpu_data{debug_handler_->GetGpuData(frame_index)};
  const auto frustums_buffer{debug_handler_->GetDebugLightFrustumBuffer()};

  auto& buffer_bindings{
      *COMET_FRAME_ARRAY_WITH_CAPACITY(ShaderBufferBindingUpdate, 5)};

  AddBufferBinding(buffer_bindings,
                   shader_handler_->GetBindingIndex(
                       rendering_debug_generate_shader_, shaderconsts::kPassSet,
                       sharedshaderconsts::kDebugAabbsBinding),
                   debug_gpu_data.ssbo_debug_aabbs_handle,
                   debug_gpu_data.ssbo_debug_aabbs_size);

  AddBufferBinding(buffer_bindings,
                   shader_handler_->GetBindingIndex(
                       rendering_debug_generate_shader_, shaderconsts::kPassSet,
                       sharedshaderconsts::kDebugLineVerticesBinding),
                   debug_gpu_data.ssbo_debug_lines_handle,
                   debug_gpu_data.ssbo_debug_lines_size);

  auto frustum_corners_buffer{
      debug_handler_->GetDebugCascadeFrustumCornersBuffer()};

  AddBufferBinding(buffer_bindings,
                   shader_handler_->GetBindingIndex(
                       rendering_debug_generate_shader_, shaderconsts::kPassSet,
                       sharedshaderconsts::kDebugCascadeFrustumsBinding),
                   frustum_corners_buffer.native_handle,
                   frustum_corners_buffer.size);

  AddBufferBinding(buffer_bindings,
                   shader_handler_->GetBindingIndex(
                       rendering_debug_generate_shader_, shaderconsts::kPassSet,
                       sharedshaderconsts::kDebugLightFrustumsBinding),
                   frustums_buffer.native_handle, frustums_buffer.size);

  auto camera_frustum_buffer{debug_handler_->GetDebugCameraFrustumBuffer()};

  AddBufferBinding(buffer_bindings,
                   shader_handler_->GetBindingIndex(
                       rendering_debug_generate_shader_, shaderconsts::kPassSet,
                       sharedshaderconsts::kDebugCameraFrustumsBinding),
                   camera_frustum_buffer.native_handle,
                   camera_frustum_buffer.size);

  ShaderPassUpdate pass_update{};
  pass_update.buffer_bindings = &buffer_bindings;
  shader_handler_->UpdatePass(rendering_debug_generate_shader_, pass_update);

  struct DebugGeneratePushConstants {
    u32 aabb_count{0};
    u32 camera_frustum_count{0};
    u32 cascade_frustum_count{0};
    u32 light_frustum_count{0};
    u32 camera_frustum_vertex_offset{0};
    u32 cascade_frustum_vertex_offset{0};
    u32 light_frustum_vertex_offset{0};
  };

  DebugGeneratePushConstants push_data{};
  push_data.aabb_count = aabb_count;
  push_data.camera_frustum_count = camera_frustum_count;
  push_data.cascade_frustum_count = cascade_frustum_count;
  push_data.light_frustum_count = light_frustum_count;

  push_data.camera_frustum_vertex_offset = aabb_count * 24u;
  push_data.cascade_frustum_vertex_offset =
      push_data.camera_frustum_vertex_offset + camera_frustum_count * 24u;
  push_data.light_frustum_vertex_offset =
      push_data.cascade_frustum_vertex_offset + cascade_frustum_count * 24u;

  auto& blocks{
      *COMET_FRAME_ARRAY_WITH_CAPACITY(ShaderPushConstantBlockUpdate, 1)};
  auto& block{blocks.EmplaceLast()};
  block.block_index = debugshaderconsts::kCountPushConstantIndex;
  block.data = &push_data;
  block.size = sizeof(push_data);

  ShaderPushConstantsUpdate push_constants{};
  push_constants.blocks = &blocks;

  shader_handler_->Bind(rendering_debug_generate_shader_);
  shader_handler_->PushConstants(rendering_debug_generate_shader_,
                                 push_constants);

  const auto group_count{
      static_cast<u32>((work_count + kShaderLocalSize - 1) / kShaderLocalSize)};

  glDispatchCompute(group_count, 1, 1);

  glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT |
                  GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);
#endif  // COMET_DEBUG_RENDERING
}

ViewUpdate ViewHandler::GenerateViewUpdate(frame::FramePacket* packet,
                                           const CameraView* camera_view,
                                           usize camera_index) const {
  COMET_ASSERT(camera_view != nullptr, "ViewHandler::GenerateViewUpdate",
               "main camera data is null");
  ViewUpdate update{};
  update.packet = packet;
  update.camera_data = &camera_view->data;
  update.camera_kind = camera_view->kind;
  update.camera_flags = camera_view->flags;
  update.camera_index = camera_index;
  update.viewport = camera_view->viewport;
  update.is_debug_draw_enabled = camera_view->is_debug_draw_enabled;
  return update;
}
}  // namespace gl
}  // namespace rendering
}  // namespace comet