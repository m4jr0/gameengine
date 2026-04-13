// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "opengl_view_shader_utils.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/rendering/driver/opengl/utils/opengl_shader_utils.h"

namespace comet {
namespace rendering {
namespace gl {
void AddWorldGlobalFieldUpdates(
    ShaderHandler* shader_handler, Shader* shader,
    const frame::FramePacket* packet,
    frame::FrameArray<ShaderBufferFieldUpdate>& field_updates) {
  COMET_ASSERT(shader_handler != nullptr, "Shader handler is null!");
  COMET_ASSERT(shader != nullptr, "Shader is null!");
  COMET_ASSERT(packet != nullptr, "Frame packet is null!");

  auto global_binding_index{shader_handler->GetBindingIndex(
      shader, shaderconsts::kGlobalSet, sharedshaderconsts::kGlobalUboBinding)};

  const auto& camera_data{packet->camera_data};

  AddFieldUpdate(field_updates, global_binding_index,
                 worldshaderconsts::kProjectionFieldIndex,
                 &camera_data.projection_matrix,
                 sizeof(camera_data.projection_matrix));

  AddFieldUpdate(field_updates, global_binding_index,
                 worldshaderconsts::kViewFieldIndex, &camera_data.view_matrix,
                 sizeof(camera_data.view_matrix));

  auto* ambient_color{COMET_FRAME_ALLOC_ONE_AND_POPULATE(
      math::Vec4, packet->ambient_color.r, packet->ambient_color.g,
      packet->ambient_color.b, 1.0f)};

  AddFieldUpdate(field_updates, global_binding_index,
                 worldshaderconsts::kAmbientColorFieldIndex, ambient_color,
                 sizeof(math::Vec4));

  auto* view_position{COMET_FRAME_ALLOC_ONE_AND_POPULATE(
      math::Vec4, camera_data.view_position.x, camera_data.view_position.y,
      camera_data.view_position.z, .0f)};

  AddFieldUpdate(field_updates, global_binding_index,
                 worldshaderconsts::kViewPositionFieldIndex, view_position,
                 sizeof(math::Vec4));
}

void AddWorldGlobalImageBindings(
    ShaderHandler* shader_handler, Shader* shader, const TextureMap* shadow_map,
    frame::FrameArray<ShaderImageBindingUpdate>& image_bindings,
    frame::FrameArray<ShaderImageDescriptor>& image_descriptors) {
  COMET_ASSERT(shader_handler != nullptr, "Shader handler is null!");
  COMET_ASSERT(shader != nullptr, "Shader is null!");

  if (shadow_map == nullptr || shadow_map->texture == nullptr ||
      shadow_map->sampler == nullptr) {
    return;
  }

  auto binding_index{shader_handler->GetBindingIndex(
      shader, shaderconsts::kGlobalSet,
      sharedshaderconsts::kMainShadowMapBinding)};

  image_descriptors.Clear();
  image_descriptors.Reserve(1);

  auto& descriptor{image_descriptors.EmplaceBack()};
  descriptor.texture = shadow_map->texture;
  descriptor.sampler = shadow_map->sampler;

  AddImageBinding(image_bindings, binding_index, image_descriptors.GetData(),
                  static_cast<u32>(image_descriptors.GetSize()));
}

void AddDebugGlobalFieldUpdates(
    ShaderHandler* shader_handler, Shader* shader,
    const frame::FramePacket* packet,
    frame::FrameArray<ShaderBufferFieldUpdate>& field_updates) {
  COMET_ASSERT(shader_handler != nullptr, "Shader handler is null!");
  COMET_ASSERT(shader != nullptr, "Shader is null!");
  COMET_ASSERT(packet != nullptr, "Frame packet is null!");

  auto global_binding_index{shader_handler->GetBindingIndex(
      shader, shaderconsts::kGlobalSet, sharedshaderconsts::kGlobalUboBinding)};

  const auto& camera_data{packet->camera_data};

  AddFieldUpdate(field_updates, global_binding_index,
                 debugshaderconsts::kProjectionFieldIndex,
                 &camera_data.projection_matrix,
                 sizeof(camera_data.projection_matrix));

  AddFieldUpdate(field_updates, global_binding_index,
                 debugshaderconsts::kViewFieldIndex, &camera_data.view_matrix,
                 sizeof(camera_data.view_matrix));
}

void AddWorldShadowSettingsFieldUpdates(
    ShaderHandler* shader_handler, Shader* shader,
    const ShadowSettings* shadow_settings,
    frame::FrameArray<ShaderBufferFieldUpdate>& field_updates) {
  COMET_ASSERT(shader_handler != nullptr, "Shader handler is null!");
  COMET_ASSERT(shader != nullptr, "Shader is null!");
  COMET_ASSERT(shadow_settings != nullptr, "Shadow settings are null!");

  auto binding_index{shader_handler->GetBindingIndex(
      shader, shaderconsts::kGlobalSet,
      sharedshaderconsts::kShadowSettingsBinding)};

  auto* params0{COMET_FRAME_ALLOC_ONE_AND_POPULATE(
      math::Vec4, shadow_settings->cascade_blend_ratio,
      shadow_settings->pcf_radius,
      static_cast<f32>(shadow_settings->pcf_samples),
      static_cast<f32>(shadow_settings->cascade_count))};

  auto* params1{COMET_FRAME_ALLOC_ONE_AND_POPULATE(
      math::S32Vec4, shadow_settings->debug_single_cascade,
      shadow_settings->is_debug_cascades ? 1 : 0,
      shadow_settings->is_blending_disabled ? 1 : 0, 0)};

  AddFieldUpdate(field_updates, binding_index,
                 worldshaderconsts::kShadowSettingsParams0FieldIndex, params0,
                 sizeof(math::Vec4));

  AddFieldUpdate(field_updates, binding_index,
                 worldshaderconsts::kShadowSettingsParams1FieldIndex, params1,
                 sizeof(math::S32Vec4));
}
}  // namespace gl
}  // namespace rendering
}  // namespace comet