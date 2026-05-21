// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/render/driver/vulkan/utils/vulkan_view_shader_utils.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/rendering/driver/vulkan/handler/vulkan_camera_handler.h"
#include "comet/rendering/driver/vulkan/type/vulkan_shader.h"
#include "comet/rendering/driver/vulkan/utils/vulkan_shader_utils.h"
#include "comet/rendering/driver/vulkan/utils/vulkan_texture_utils.h"

namespace comet {
namespace rendering {
namespace vk {
void AddWorldGlobalFieldUpdates(
    ShaderHandler* shader_handler, ShaderHandle shader_handle,
    const frame::FramePacket* packet,
    frame::FrameArray<ShaderBufferFieldUpdate>& field_updates) {
  COMET_ASSERT(shader_handler != nullptr,
               "vulkan_view_shader_utils::AddWorldGlobalFieldUpdates",
               "shader handler is null");
  COMET_ASSERT(shader_handle,
               "vulkan_view_shader_utils::AddWorldGlobalFieldUpdates",
               "shader handle is invalid");
  COMET_ASSERT(packet != nullptr,
               "vulkan_view_shader_utils::AddWorldGlobalFieldUpdates",
               "frame packet is null");

  const auto global_binding_index{
      shader_handler->GetBindingIndex(shader_handle, shaderconsts::kGlobalSet,
                                      sharedshaderconsts::kGlobalUboBinding)};

  const auto* ambient_color = COMET_FRAME_ALLOC_ONE_AND_POPULATE(
      math::Vec4, packet->ambient_color.r, packet->ambient_color.g,
      packet->ambient_color.b, 1.0f);

  AddFieldUpdate(field_updates, global_binding_index,
                 worldshaderconsts::kAmbientColorFieldIndex, ambient_color,
                 sizeof(math::Vec4));
}

void AddWorldGlobalImageBindings(
    ShaderHandler* shader_handler, const TextureHandler* texture_handler,
    ShaderHandle shader_handle, const TextureMap* shadow_map,
    frame::FrameArray<ShaderImageBindingUpdate>& image_bindings,
    frame::FrameArray<ShaderImageDescriptor>& image_descriptors) {
  if (shadow_map == nullptr) {
    return;
  }

  COMET_ASSERT(shadow_map->texture_handle,
               "vulkan_view_shader_utils::AddWorldGlobalImageBindings",
               "shadow map texture handle is invalid");
  COMET_ASSERT(shadow_map->sampler_handle,
               "vulkan_view_shader_utils::AddWorldGlobalImageBindings",
               "shadow map sampler handle is invalid");
  COMET_ASSERT(shader_handler != nullptr,
               "vulkan_view_shader_utils::AddWorldGlobalImageBindings",
               "shader handler is null");
  COMET_ASSERT(texture_handler != nullptr,
               "vulkan_view_shader_utils::AddWorldGlobalImageBindings",
               "texture handler is null");
  COMET_ASSERT(shader_handle,
               "vulkan_view_shader_utils::AddWorldGlobalImageBindings",
               "shader handle is invalid");

  const auto binding_index{shader_handler->GetBindingIndex(
      shader_handle, shaderconsts::kGlobalSet,
      sharedshaderconsts::kMainShadowMapBinding)};

  const auto* texture{texture_handler->Get(shadow_map->texture_handle)};
  COMET_ASSERT(texture != nullptr,
               "vulkan_view_shader_utils::AddWorldGlobalImageBindings",
               "shadow map texture is null");

  const auto image_layout{GetDescriptorImageLayout(texture)};

  image_descriptors.Clear();
  image_descriptors.Reserve(1);
  image_descriptors.PushLast(GenerateImageDescriptor(
      ShaderBindingType::CombinedImageSampler, shadow_map->texture_handle,
      shadow_map->sampler_handle, image_layout));

  AddImageBinding(image_bindings, binding_index, image_descriptors.GetData(),
                  static_cast<u32>(image_descriptors.GetSize()));
}

void AddWorldShadowSettingsFieldUpdates(
    ShaderHandler* shader_handler, ShaderHandle shader_handle,
    const ShadowSettings* shadow_settings,
    frame::FrameArray<ShaderBufferFieldUpdate>& field_updates) {
  COMET_ASSERT(shader_handler != nullptr,
               "vulkan_view_shader_utils::AddWorldShadowSettingsFieldUpdates",
               "shader handler is null");
  COMET_ASSERT(shader_handle,
               "vulkan_view_shader_utils::AddWorldShadowSettingsFieldUpdates",
               "shader handle is invalid");
  COMET_ASSERT(shadow_settings != nullptr,
               "vulkan_view_shader_utils::AddWorldShadowSettingsFieldUpdates",
               "shadow settings are null");

  const auto binding_index{shader_handler->GetBindingIndex(
      shader_handle, shaderconsts::kGlobalSet,
      sharedshaderconsts::kShadowSettingsBinding)};

  auto* params0 = COMET_FRAME_ALLOC_ONE_AND_POPULATE(
      math::Vec4, shadow_settings->cascade_blend_ratio,
      shadow_settings->pcf_radius,
      static_cast<f32>(shadow_settings->pcf_samples),
      static_cast<f32>(shadow_settings->cascade_count));

  auto* params1 = COMET_FRAME_ALLOC_ONE_AND_POPULATE(
      math::S32Vec4, shadow_settings->debug_single_cascade,
      shadow_settings->is_debug_cascades ? 1 : 0,
      shadow_settings->is_blending_disabled ? 1 : 0, 0);

  AddFieldUpdate(field_updates, binding_index,
                 worldshaderconsts::kShadowSettingsParams0FieldIndex, params0,
                 sizeof(math::Vec4));

  AddFieldUpdate(field_updates, binding_index,
                 worldshaderconsts::kShadowSettingsParams1FieldIndex, params1,
                 sizeof(math::S32Vec4));
}

void AddCameraBufferBinding(
    ShaderHandler* shader_handler, const CameraHandler* camera_handler,
    ShaderHandle shader_handle, FrameInFlightIndex frame_index,
    frame::FrameArray<ShaderBufferBindingUpdate>& buffer_bindings) {
  COMET_ASSERT(shader_handler != nullptr,
               "vulkan_view_shader_utils::AddCameraBufferBinding",
               "shader handler is null");
  COMET_ASSERT(camera_handler != nullptr,
               "vulkan_view_shader_utils::AddCameraBufferBinding",
               "camera handler is null");
  COMET_ASSERT(shader_handle,
               "vulkan_view_shader_utils::AddCameraBufferBinding",
               "shader handle is invalid");

  const auto camera_gpu_data{camera_handler->GetGpuData(frame_index)};

  COMET_ASSERT(camera_gpu_data.ssbo_camera_datas_handle != VK_NULL_HANDLE,
               "vulkan_view_shader_utils::AddCameraBufferBinding",
               "camera buffer handle is invalid", "frame_index", frame_index);
  COMET_ASSERT(camera_gpu_data.ssbo_camera_datas_size > 0,
               "vulkan_view_shader_utils::AddCameraBufferBinding",
               "camera buffer size is zero", "frame_index", frame_index);

  AddBufferBinding(
      buffer_bindings,
      shader_handler->GetBindingIndex(shader_handle, shaderconsts::kPassSet,
                                      sharedshaderconsts::kCameraDatasBinding),
      camera_gpu_data.ssbo_camera_datas_handle,
      camera_gpu_data.ssbo_camera_datas_size);
}
}  // namespace vk
}  // namespace rendering
}  // namespace comet