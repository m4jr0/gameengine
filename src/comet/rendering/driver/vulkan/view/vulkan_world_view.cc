// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "vulkan_world_view.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/frame/frame_container.h"
#include "comet/core/frame/frame_packet.h"
#include "comet/core/memory/memory_utils.h"
#include "comet/profiler/profiler.h"
#include "comet/rendering/driver/vulkan/type/vulkan_shader.h"
#include "comet/rendering/driver/vulkan/utils/vulkan_shader_utils.h"
#include "comet/rendering/driver/vulkan/utils/vulkan_view_shader_utils.h"
#include "comet/rendering/driver/vulkan/utils/vulkan_view_utils.h"

#ifdef COMET_DEBUG
#include "comet/debugging/rendering/rendering_debug_settings.h"
#endif  // COMET_DEBUG

namespace comet {
namespace rendering {
namespace vk {
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

  VkClearValue clear_values[2]{};
  memory::CopyMemory(&clear_values[0].color, clear_color_,
                     sizeof(clear_values[0].color.float32[0]) * 4);
  clear_values[1].depthStencil.depth = 1.0f;
  clear_values[1].depthStencil.stencil = 0;

  const auto current_frame{context_->GetFrameInFlightIndex()};
  auto& frame_data{context_->GetFrameData(current_frame)};
  const auto command_buffer_handle{frame_data.command_buffer_handle};

  render_pass_handler_->BeginPass(render_pass_handle_, command_buffer_handle,
                                  context_->GetImageIndex(), clear_values, 2);
}

void WorldView::Draw(const ViewUpdate& update) {
  COMET_PROFILE("WorldView::Draw");
  SetViewportAndScissor(update.viewport);
  DrawWorld(update);
}

void WorldView::End([[maybe_unused]] const ViewUpdate& update) {
  COMET_PROFILE("WorldView::End");

  const auto current_frame{context_->GetFrameInFlightIndex()};
  auto& frame_data{context_->GetFrameData(current_frame)};

  render_pass_handler_->EndPass(frame_data.command_buffer_handle);
}

void WorldView::OnInitialize() {
  RenderPassDescr render_pass_descr{};

  VkExtent2D extent{};
  extent.width = width_;
  extent.height = height_;

  render_pass_descr.extent = extent;
  render_pass_descr.offset = {0, 0};

  render_pass_descr.dependencies = frame::FrameArray<VkSubpassDependency>{};
  render_pass_descr.dependencies.Reserve(2);

  auto& dependency_1{render_pass_descr.dependencies.EmplaceLast()};
  dependency_1.srcSubpass = VK_SUBPASS_EXTERNAL;
  dependency_1.dstSubpass = 0;
  dependency_1.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                              VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
  dependency_1.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
                               VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
  dependency_1.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                              VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
  dependency_1.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
                               VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
  dependency_1.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

  auto& dependency_2{render_pass_descr.dependencies.EmplaceLast()};
  dependency_2.srcSubpass = 0;
  dependency_2.dstSubpass = VK_SUBPASS_EXTERNAL;
  dependency_2.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency_2.dstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
  dependency_2.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  dependency_2.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
  dependency_2.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

  const auto& device{context_->GetDevice()};
  const auto is_msaa{device.IsMsaa()};

  render_pass_descr.clear_flags = GenerateClearFlags(pass_descr_);
  render_pass_descr.attachment_descrs = frame::FrameArray<AttachmentDescr>{};
  render_pass_descr.attachment_descrs.Reserve(is_msaa ? 3 : 2);

  GenerateAttachmentDescrs(
      pass_descr_, is_msaa ? device.GetMsaaSamples() : VK_SAMPLE_COUNT_1_BIT,
      render_pass_descr.attachment_descrs);

  render_pass_descr.options = kRenderPassOptionFlagBitsSwapchainTarget;

  if (is_msaa) {
    render_pass_descr.options |= kRenderPassOptionFlagBitsMultisampled;
  }

  render_pass_handle_ = render_pass_handler_->GetOrGenerate(render_pass_descr);

  {
    ShaderDescr shader_descr{};
    shader_descr.shader_resource_id =
        resource::GenerateResourceIdFromPath<resource::ShaderResource>(
            COMET_TCHAR("shaders/vulkan/forward_shader.vk.cshader"));
    shader_descr.render_pass_handle = render_pass_handle_;
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

    AddWorldGlobalImageBindings(shader_handler_, texture_handler_,
                                world_shader_, shadow_map, image_bindings,
                                image_descriptors);

    global_update.image_bindings = &image_bindings;
    shader_handler_->UpdateGlobals(world_shader_, global_update);
  }

  const auto frame_index{context_->GetFrameInFlightIndex()};
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
      light_gpu_data.ssbo_lights_handle, light_gpu_data.ssbo_lights_size);

  AddBufferBinding(
      buffer_bindings,
      shader_handler_->GetBindingIndex(world_shader_, shaderconsts::kPassSet,
                                       sharedshaderconsts::kShadowsBinding),
      shadow_gpu_data.ssbo_shadow_data_handle,
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

  const auto frame_index{context_->GetFrameInFlightIndex()};

  const auto& indirect_buffer{
      render_proxy_handler_->GetIndirectBuffer(frame_index)};

  COMET_ASSERT(indirect_buffer.handle != VK_NULL_HANDLE, "WorldView::DrawWorld",
               "draw indirect buffer handle is invalid", "frame_index",
               frame_index);
  COMET_ASSERT(indirect_buffer.size > 0, "WorldView::DrawWorld",
               "draw indirect buffer size is zero", "frame_index", frame_index);

  const auto* indirect_batches{render_proxy_handler_->GetIndirectBatches()};
  COMET_ASSERT(indirect_batches != nullptr, "WorldView::DrawWorld",
               "indirect batches are null");

  const auto current_frame{context_->GetFrameInFlightIndex()};
  auto& frame_data{context_->GetFrameData(current_frame)};
  const auto command_buffer_handle{frame_data.command_buffer_handle};

  mesh_handler_->Bind();
  shader_handler_->Bind(world_shader_);

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
      debug::RenderingDebugSettings::Get().GetFlags(is_debug_camera);
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

    vkCmdDrawIndexedIndirect(command_buffer_handle, indirect_buffer.handle,
                             group.offset * sizeof(GpuIndirectRenderProxy),
                             group.count, sizeof(GpuIndirectRenderProxy));
  }
}

void WorldView::SetViewportAndScissor(const ViewportRect& viewport) const {
  const auto current_frame{context_->GetFrameInFlightIndex()};
  auto& frame_data{context_->GetFrameData(current_frame)};
  const auto command_buffer_handle{frame_data.command_buffer_handle};

  VkViewport vk_viewport{
      .x = viewport.x,
      .y = viewport.y,
      .width = viewport.width,
      .height = viewport.height,
      .minDepth = .0f,
      .maxDepth = 1.0f,
  };

  vkCmdSetViewport(command_buffer_handle, 0, 1, &vk_viewport);

  VkRect2D scissor{
      .offset =
          {
              static_cast<s32>(viewport.x),
              static_cast<s32>(viewport.y),
          },
      .extent =
          {
              static_cast<u32>(viewport.width),
              static_cast<u32>(viewport.height),
          },
  };

  vkCmdSetScissor(command_buffer_handle, 0, 1, &scissor);
}
}  // namespace vk
}  // namespace rendering
}  // namespace comet