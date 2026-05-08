// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "opengl_world_view.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/frame/frame_container.h"
#include "comet/core/frame/frame_packet.h"
#include "comet/profiler/profiler.h"
#include "comet/rendering/driver/opengl/type/opengl_shader.h"
#include "comet/rendering/driver/opengl/utils/opengl_shader_utils.h"
#include "comet/rendering/driver/opengl/utils/opengl_view_shader_utils.h"
#include "comet/resource/resource.h"
#include "comet/resource/shader/shader_resource.h"

#ifdef COMET_DEBUG
#include "comet/debugging/rendering/rendering_debug_settings.h"
#endif  // COMET_DEBUG

namespace comet {
namespace rendering {
namespace gl {
WorldView::WorldView(const WorldViewDescr& descr)
    : View{descr},
      shadow_settings_{descr.shadow_settings},
      camera_handler_{descr.camera_handler},
      shader_handler_{descr.shader_handler},
      texture_handler_{descr.texture_handler},
      render_proxy_handler_{descr.render_proxy_handler},
      mesh_handler_{descr.mesh_handler},
      lighting_handler_{descr.lighting_handler} {
  COMET_ASSERT(shadow_settings_ != nullptr, "WorldView::WorldView",
               "shadow settings are null");
  COMET_ASSERT(camera_handler_ != nullptr, "WorldView::WorldView",
               "camera handler is null");
  COMET_ASSERT(shader_handler_ != nullptr, "WorldView::WorldView",
               "shader handler is null");
  COMET_ASSERT(texture_handler_ != nullptr, "WorldView::WorldView",
               "texture handler is null");
  COMET_ASSERT(render_proxy_handler_ != nullptr, "WorldView::WorldView",
               "render proxy handler is null");
  COMET_ASSERT(mesh_handler_ != nullptr, "WorldView::WorldView",
               "mesh handler is null");
  COMET_ASSERT(lighting_handler_ != nullptr, "WorldView::WorldView",
               "lighting handler is null");
}

void WorldView::Prepare(const ViewUpdate& update) {
  COMET_PROFILE("WorldView::Prepare");

  auto* packet{update.packet};
  COMET_ASSERT(packet != nullptr, "WorldView::Prepare", "frame packet is null");

  UpdateWorldPassShaderData(packet);
}

void WorldView::Begin(const ViewUpdate&) {
  COMET_PROFILE("WorldView::Begin");

  if ((pass_descr_.flags & kViewPassFlagBitsHasColor) != 0) {
    glClearColor(clear_color_[0], clear_color_[1], clear_color_[2],
                 clear_color_[3]);
  }

  GLbitfield clear_mask{0};

  if ((pass_descr_.flags & kViewPassFlagBitsHasColor) != 0 &&
      pass_descr_.color_load_op == ViewLoadOp::Clear) {
    clear_mask |= GL_COLOR_BUFFER_BIT;
  }

  if ((pass_descr_.flags & kViewPassFlagBitsHasDepth) != 0 &&
      pass_descr_.depth_load_op == ViewLoadOp::Clear) {
    clear_mask |= GL_DEPTH_BUFFER_BIT;
  }

  if (clear_mask != 0) {
    glClear(clear_mask);
  }
}

void WorldView::Draw(const ViewUpdate& update) {
  COMET_PROFILE("WorldView::Draw");

  SetViewport(update.viewport);
  DrawWorld(update);
}

void WorldView::End(const ViewUpdate&) { COMET_PROFILE("WorldView::End"); }

void WorldView::OnInitialize() {
  {
    ShaderDescr shader_descr{};
    shader_descr.shader_resource_id =
        resource::GenerateResourceIdFromPath<resource::ShaderResource>(
            COMET_TCHAR("shaders/opengl/forward_shader.gl.cshader"));
    shader_descr.bind_type = PipelineBindType::Graphics;
    world_shader_ = shader_handler_->GetOrGenerate(shader_descr);
  }

  CameraFlags camera_flags{kCameraFlagBitsGame};

#ifdef COMET_DEBUG
  camera_flags |= kCameraFlagBitsDebug;
#endif  // COMET_DEBUG

  SetCameraFlags(camera_flags);
}

void WorldView::OnDestroy() {
  if (world_shader_) {
    shader_handler_->Destroy(world_shader_);
    world_shader_.Invalidate();
  }

  shadow_settings_ = nullptr;
  camera_handler_ = nullptr;
  shader_handler_ = nullptr;
  texture_handler_ = nullptr;
  render_proxy_handler_ = nullptr;
  mesh_handler_ = nullptr;
  lighting_handler_ = nullptr;
}

void WorldView::UpdateWorldPassShaderData(const frame::FramePacket* packet) {
  COMET_ASSERT(packet != nullptr, "WorldView::UpdateWorldPassShaderData",
               "frame packet is null");

  {
    static constexpr usize kShaderBufferFieldCapacity{3};
    auto& field_updates{*COMET_FRAME_ARRAY_WITH_CAPACITY(
        ShaderBufferFieldUpdate, kShaderBufferFieldCapacity)};

    AddWorldGlobalFieldUpdates(shader_handler_, world_shader_, packet,
                               field_updates);

    AddWorldShadowSettingsFieldUpdates(shader_handler_, world_shader_,
                                       shadow_settings_, field_updates);

    ShaderGlobalUpdate global_update{};
    global_update.field_updates = &field_updates;

    const auto* shadow_map{lighting_handler_->GetShadowArrayTextureMap()};

    static constexpr usize kShaderImageBindingCapacity{1};
    auto& image_bindings{*COMET_FRAME_ARRAY_WITH_CAPACITY(
        ShaderImageBindingUpdate, kShaderImageBindingCapacity)};
    auto& image_descriptors{*COMET_FRAME_ARRAY_WITH_CAPACITY(
        ShaderImageDescriptor, kShaderImageBindingCapacity)};

    AddWorldGlobalImageBindings(shader_handler_, world_shader_, shadow_map,
                                image_bindings, image_descriptors);

    global_update.image_bindings = &image_bindings;
    shader_handler_->UpdateGlobals(world_shader_, global_update);
  }

  const auto frame_index{frame_state_->GetFrameInFlightIndex()};
  const auto proxy_gpu_data{render_proxy_handler_->GetGpuData(frame_index)};
  const auto light_gpu_data{lighting_handler_->GetLightGpuData(frame_index)};
  const auto shadow_gpu_data{lighting_handler_->GetShadowGpuData(frame_index)};

  static constexpr usize kShaderBufferBindingCapacity{6};
  auto& buffer_bindings{*COMET_FRAME_ARRAY_WITH_CAPACITY(
      ShaderBufferBindingUpdate, kShaderBufferBindingCapacity)};

  AddBufferBinding(buffer_bindings,
                   shader_handler_->GetBindingIndex(
                       world_shader_, shaderconsts::kPassSet,
                       sharedshaderconsts::kProxyLocalDatasBinding),
                   proxy_gpu_data.ssbo_proxy_local_datas_handle,
                   proxy_gpu_data.ssbo_proxy_local_datas_size);

  AddBufferBinding(
      buffer_bindings,
      shader_handler_->GetBindingIndex(world_shader_, shaderconsts::kPassSet,
                                       sharedshaderconsts::kProxyIdsBinding),
      proxy_gpu_data.ssbo_proxy_ids_handle, proxy_gpu_data.ssbo_proxy_ids_size);

  AddBufferBinding(buffer_bindings,
                   shader_handler_->GetBindingIndex(
                       world_shader_, shaderconsts::kPassSet,
                       sharedshaderconsts::kMatrixPalettesBinding),
                   proxy_gpu_data.ssbo_matrix_palettes_handle,
                   proxy_gpu_data.ssbo_matrix_palettes_size);

  AddBufferBinding(
      buffer_bindings,
      shader_handler_->GetBindingIndex(world_shader_, shaderconsts::kPassSet,
                                       sharedshaderconsts::kLightsBinding),
      light_gpu_data.ssbo_lights_native_handle,
      light_gpu_data.ssbo_lights_size);

  AddBufferBinding(
      buffer_bindings,
      shader_handler_->GetBindingIndex(world_shader_, shaderconsts::kPassSet,
                                       sharedshaderconsts::kShadowsBinding),
      shadow_gpu_data.ssbo_shadow_data_native_handle,
      shadow_gpu_data.ssbo_shadow_data_size);

  AddCameraBufferBinding(shader_handler_, camera_handler_, world_shader_,
                         frame_index, buffer_bindings);

  ShaderPassUpdate pass_update{};
  pass_update.buffer_bindings = &buffer_bindings;
  shader_handler_->UpdatePass(world_shader_, pass_update);
}

void WorldView::DrawWorld(const ViewUpdate& update) {
  COMET_PROFILE("WorldView::DrawWorld");

  const auto* batch_groups{render_proxy_handler_->GetBatchGroups()};

  if (batch_groups == nullptr || batch_groups->IsEmpty()) {
    return;
  }

  const auto frame_index{frame_state_->GetFrameInFlightIndex()};

  const auto& indirect_buffer{
      render_proxy_handler_->GetIndirectBuffer(frame_index)};

  COMET_ASSERT(indirect_buffer.native_handle != kInvalidGlNativeStorageHandle,
               "WorldView::DrawWorld", "draw indirect buffer handle is invalid",
               "frame_index", frame_index);
  COMET_ASSERT(indirect_buffer.size > 0, "WorldView::DrawWorld",
               "draw indirect buffer size is zero", "frame_index", frame_index);

  const auto* indirect_batches{render_proxy_handler_->GetIndirectBatches()};
  COMET_ASSERT(indirect_batches != nullptr, "WorldView::DrawWorld",
               "indirect batches are null");

  shader_handler_->Bind(world_shader_);
  shader_handler_->BindVertexSource(world_shader_,
                                    mesh_handler_->GetVertexSource());

  struct WorldPushConstants {
    u32 light_count{0};
    u32 camera_index{0};
    u32 debug_flags{0};
  };

  WorldPushConstants push_data{};
  push_data.light_count = lighting_handler_->GetLightCount();
  push_data.camera_index = static_cast<u32>(update.camera_index);

#ifdef COMET_DEBUG
  const auto is_debug_camera{update.camera_kind == CameraKind::Debug};
  push_data.debug_flags =
      comet::debug::RenderingDebugSettings::Get().GetFlags(is_debug_camera);
#else
  push_data.debug_flags = kWorldDebugFlagBitsNone;
#endif  // COMET_DEBUG

  static constexpr usize kShaderPushConstantBlockCapacity{1};
  auto& blocks{*COMET_FRAME_ARRAY_WITH_CAPACITY(
      ShaderPushConstantBlockUpdate, kShaderPushConstantBlockCapacity)};

  auto& block{blocks.EmplaceLast()};
  block.block_index = worldshaderconsts::kLightingPushConstantIndex;
  block.data = &push_data;
  block.size = sizeof(push_data);

  ShaderPushConstantsUpdate push_constants{};
  push_constants.blocks = &blocks;
  shader_handler_->PushConstants(world_shader_, push_constants);

  glBindBuffer(GL_DRAW_INDIRECT_BUFFER, indirect_buffer.native_handle);

  MaterialHandle last_material_handle{};

  for (const auto& group : *batch_groups) {
    const auto& batch{indirect_batches->Get(group.offset)};
    const auto proxy_id{batch.proxy_id};
    const auto material_handle{
        render_proxy_handler_->GetMaterialHandle(proxy_id)};

    if (material_handle != last_material_handle) {
      if (!shader_handler_->HasMaterial(world_shader_, material_handle)) {
        shader_handler_->BindMaterial(world_shader_, material_handle);
      }

      shader_handler_->BindInstance(world_shader_, material_handle);
      last_material_handle = material_handle;
    }

    glMultiDrawElementsIndirect(
        shader_handler_->GetTopology(world_shader_), GL_UNSIGNED_INT,
        reinterpret_cast<const void*>(group.offset *
                                      sizeof(GpuIndirectRenderProxy)),
        static_cast<GLsizei>(group.count), sizeof(GpuIndirectRenderProxy));
  }

  glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
}

void WorldView::SetViewport(const ViewportRect& viewport) const {
  glViewport(static_cast<GLint>(viewport.x), static_cast<GLint>(viewport.y),
             static_cast<GLsizei>(viewport.width),
             static_cast<GLsizei>(viewport.height));
}
}  // namespace gl
}  // namespace rendering
}  // namespace comet