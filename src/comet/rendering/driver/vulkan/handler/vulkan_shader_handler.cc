// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "vulkan_shader_handler.h"
////////////////////////////////////////////////////////////////////////////////

// External. ///////////////////////////////////////////////////////////////////
#include <algorithm>
#include <utility>
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/frame/frame_container.h"
#include "comet/core/memory/allocator/allocator.h"
#include "comet/core/memory/memory_utils.h"
#include "comet/core/type/array.h"
#include "comet/core/type_trait.h"
#include "comet/rendering/driver/vulkan/type/vulkan_buffer.h"
#include "comet/rendering/driver/vulkan/type/vulkan_descriptor.h"
#include "comet/rendering/driver/vulkan/type/vulkan_mesh.h"
#include "comet/rendering/driver/vulkan/type/vulkan_pipeline.h"
#include "comet/rendering/driver/vulkan/type/vulkan_shader.h"
#include "comet/rendering/driver/vulkan/type/vulkan_texture_map.h"
#include "comet/rendering/driver/vulkan/utils/vulkan_buffer_utils.h"
#include "comet/rendering/driver/vulkan/utils/vulkan_descriptor_utils.h"
#include "comet/rendering/driver/vulkan/utils/vulkan_initializer_utils.h"
#include "comet/rendering/driver/vulkan/utils/vulkan_pipeline_utils.h"
#include "comet/rendering/driver/vulkan/utils/vulkan_shader_utils.h"
#include "comet/rendering/driver/vulkan/utils/vulkan_texture_utils.h"
#include "comet/rendering/driver/vulkan/vulkan_context.h"
#include "comet/rendering/driver/vulkan/vulkan_debug.h"
#include "comet/rendering/label/pipeline_label.h"
#include "comet/rendering/label/shader_label.h"
#include "comet/rendering/type/pipeline.h"
#include "comet/rendering/type/shader.h"
#include "comet/rendering/utils/shader_utils.h"
#include "comet/resource/resource_manager.h"

namespace comet {
namespace rendering {
namespace vk {
namespace internal {
static bool HasBindingInSet(const Shader* shader, u32 set) {
  for (const auto& binding : shader->bindings) {
    if (binding.set == set) {
      return true;
    }
  }
  return false;
}
}  // namespace internal

ShaderHandler::ShaderHandler(const ShaderHandlerDescr& descr)
    : Handler{descr},
      shaders_{&cache_allocator_, 1024},
      shader_module_handler_{descr.shader_module_handler},
      pipeline_handler_{descr.pipeline_handler},
      material_handler_{descr.material_handler},
      texture_handler_{descr.texture_handler},
      sampler_handler_{descr.sampler_handler},
      descriptor_handler_{descr.descriptor_handler},
      render_pass_handler_{descr.render_pass_handler} {
  COMET_ASSERT(shader_module_handler_ != nullptr,
               "ShaderHandler::ShaderHandler", "shader module handler is null");
  COMET_ASSERT(pipeline_handler_ != nullptr, "ShaderHandler::ShaderHandler",
               "pipeline handler is null");
  COMET_ASSERT(material_handler_ != nullptr, "ShaderHandler::ShaderHandler",
               "material handler is null");
  COMET_ASSERT(texture_handler_ != nullptr, "ShaderHandler::ShaderHandler",
               "texture handler is null");
  COMET_ASSERT(sampler_handler_ != nullptr, "ShaderHandler::ShaderHandler",
               "sampler handler is null");
  COMET_ASSERT(descriptor_handler_ != nullptr, "ShaderHandler::ShaderHandler",
               "descriptor handler is null");
  COMET_ASSERT(render_pass_handler_ != nullptr, "ShaderHandler::ShaderHandler",
               "render pass handler is null");
}

ShaderHandle ShaderHandler::GetOrGenerate(const ShaderDescr& descr) {
  return GetOrGenerate(descr.shader_resource_id, descr.render_pass_handle,
                       descr.bind_type);
}

ShaderHandle ShaderHandler::GetOrGenerate(
    resource::ShaderResourceId shader_resource_id,
    RenderPassHandle render_pass_handle, PipelineBindType bind_type) {
  COMET_ASSERT(shader_resource_id.IsValid(), "ShaderHandler::GetOrGenerate",
               "shader resource id is invalid");

  if (bind_type == PipelineBindType::Graphics) {
    COMET_ASSERT(render_pass_handle, "ShaderHandler::GetOrGenerate",
                 "graphics shader requires a render pass handle");
  }

  ShaderKey key{};
  key.shader_resource_id = shader_resource_id;
  key.render_pass_handle = render_pass_handle;
  key.bind_type = bind_type;

  if (const auto handle{shaders_.TryAcquire(key)}; handle) {
    return handle;
  }

  ShaderHandle generated_handle{ShaderHandle::Invalid()};
  auto* shader_resource_handler{resource::ResourceManager::Get().GetShaders()};

  const auto is_loaded{shader_resource_handler->WithTemporaryLoad(
      shader_resource_id,
      [this, &generated_handle, shader_resource_id, render_pass_handle,
       bind_type](const resource::ShaderResource* shader_resource) {
        ShaderDescr descr{};
        descr.shader_resource_id = shader_resource_id;
        descr.render_pass_handle = render_pass_handle;
        descr.bind_type = bind_type;

        auto* shader{GenerateShader(descr, shader_resource)};
        COMET_ASSERT(shader != nullptr, "ShaderHandler::GetOrGenerate",
                     "generated shader is null");

        ShaderKey key{};
        key.shader_resource_id = shader_resource_id;
        key.render_pass_handle = render_pass_handle;
        key.bind_type = bind_type;

        generated_handle = shaders_.Create(key, shader);
        COMET_ASSERT(generated_handle, "ShaderHandler::GetOrGenerate",
                     "shader instance creation failed");

        shader->handle = generated_handle;
      })};

  return is_loaded ? generated_handle : ShaderHandle::Invalid();
}

void ShaderHandler::Destroy(ShaderHandle handle) {
  auto* shader{shaders_.Get(handle)};

  if (!shaders_.Release(handle)) {
    return;
  }

  DestroyShader(shader);
  shaders_.Remove(handle);
}

void ShaderHandler::Bind(ShaderHandle handle) const {
  const auto* shader{Get(handle)};

  const auto pipeline{shader->bind_type == PipelineBindType::Graphics
                          ? shader->graphics_pipeline
                          : shader->compute_pipeline};

  COMET_ASSERT(pipeline, "ShaderHandler::Bind", "pipeline handle is invalid");

  pipeline_handler_->Bind(pipeline);

  const auto bind_point{GetVkPipelineBindPoint(shader->bind_type)};

  const auto current_frame{context_->GetFrameInFlightIndex()};
  auto& frame_data{context_->GetFrameData(current_frame)};
  const auto command_buffer_handle{frame_data.command_buffer_handle};
  const auto image_index{context_->GetImageIndex()};

  if (!shader->global_descriptor_data.descriptor_set_handles.IsEmpty() &&
      internal::HasBindingInSet(shader, shaderconsts::kGlobalSet)) {
    const auto set_handle{
        shader->global_descriptor_data.descriptor_set_handles[image_index]};

    vkCmdBindDescriptorSets(command_buffer_handle, bind_point,
                            pipeline_handler_->GetNativeLayoutHandle(pipeline),
                            shaderconsts::kGlobalSet, 1, &set_handle, 0,
                            nullptr);
  }

  if (!shader->storage_descriptor_data.descriptor_set_handles.IsEmpty() &&
      internal::HasBindingInSet(shader, shaderconsts::kPassSet)) {
    const auto set_handle{
        shader->storage_descriptor_data.descriptor_set_handles[image_index]};

    vkCmdBindDescriptorSets(command_buffer_handle, bind_point,
                            pipeline_handler_->GetNativeLayoutHandle(pipeline),
                            shaderconsts::kPassSet, 1, &set_handle, 0, nullptr);
  }
}

void ShaderHandler::BindInstance(ShaderHandle handle,
                                 MaterialHandle material_handle) {
  BindInstance(handle, GetMaterialHandler()->Get(material_handle));
}

void ShaderHandler::BindInstance(ShaderHandle handle,
                                 const Material* material) {
  auto* shader{Get(handle)};
  COMET_ASSERT(material != nullptr, "ShaderHandler::BindInstance",
               "material is null", "shader_handle", handle);
  COMET_ASSERT(shader->bind_type == PipelineBindType::Graphics,
               "ShaderHandler::UpdateInstance",
               "materials are only supported on graphics shaders",
               "shader_handle", shader->handle);

  if (!internal::HasBindingInSet(shader, shaderconsts::kMaterialSet)) {
#ifdef COMET_DEBUG_RENDERING
    COMET_LOG_WARNING(LoggerType::Rendering, "ShaderHandler::BindInstance",
                      "shader has no material set", "shader_handle",
                      shader->handle, "render_pass_handle",
                      shader->render_pass_handle);
#endif  // COMET_DEBUG_RENDERING
    return;
  }

  COMET_ASSERT(shader->bind_type == PipelineBindType::Graphics,
               "ShaderHandler::BindInstance",
               "material instances can only be bound on graphics shaders",
               "shader_handle", handle);

  const auto pipeline{shader->graphics_pipeline};

  COMET_ASSERT(pipeline, "ShaderHandler::BindInstance",
               "pipeline handle is invalid", "shader_handle", handle,
               "pipeline_bind_type",
               GetPipelineBindTypeLabel(shader->bind_type),
               "pipeline_bind_type_value", ToUnderlying(shader->bind_type));
  pipeline_handler_->Bind(pipeline);

  const auto bind_point{pipeline_handler_->GetBindType(pipeline) ==
                                PipelineBindType::Graphics
                            ? VK_PIPELINE_BIND_POINT_GRAPHICS
                            : VK_PIPELINE_BIND_POINT_COMPUTE};

  const auto current_frame{context_->GetFrameInFlightIndex()};
  auto& frame_data{context_->GetFrameData(current_frame)};

  const auto command_buffer_handle{frame_data.command_buffer_handle};
  const auto image_index{context_->GetImageIndex()};
  auto& instance{GetInstance(shader, material)};

  if (instance.descriptor_data.descriptor_set_handles.IsEmpty()) {
    return;
  }

  const auto set_handle{
      instance.descriptor_data.descriptor_set_handles[image_index]};

  vkCmdBindDescriptorSets(command_buffer_handle, bind_point,
                          pipeline_handler_->GetNativeLayoutHandle(pipeline),
                          shaderconsts::kMaterialSet, 1, &set_handle, 0,
                          VK_NULL_HANDLE);
}

void ShaderHandler::UpdateGlobals(ShaderHandle handle,
                                  const ShaderGlobalUpdate& update) {
  auto* shader{Get(handle)};

  const auto image_index{context_->GetImageIndex()};

  if (shader->global_descriptor_data.descriptor_set_handles.IsEmpty()) {
    return;
  }

  const auto set_handle{
      shader->global_descriptor_data.descriptor_set_handles[image_index]};

  if (update.field_updates != nullptr && !update.field_updates->IsEmpty()) {
    auto& buffer{shader->uniform_buffers[image_index]};
    ScopedMappedBuffer mapped{buffer};

    for (const auto& field_update : *update.field_updates) {
      COMET_ASSERT(field_update.binding_index < shader->bindings.GetSize(),
                   "ShaderHandler::UpdateGlobals",
                   "global field binding index is invalid", "shader_handle",
                   handle, "binding_index", field_update.binding_index,
                   "binding_count", shader->bindings.GetSize());

      const auto& binding{shader->bindings[field_update.binding_index]};

      COMET_ASSERT(binding.scope == ShaderBindingScope::Global,
                   "ShaderHandler::UpdateGlobals", "binding scope mismatch",
                   "shader_handle", handle, "binding_index",
                   field_update.binding_index, "expected_scope",
                   GetShaderBindingScopeLabel(ShaderBindingScope::Global),
                   "expected_scope_value",
                   ToUnderlying(ShaderBindingScope::Global), "actual_scope",
                   GetShaderBindingScopeLabel(binding.scope),
                   "actual_scope_value", ToUnderlying(binding.scope));
      COMET_ASSERT(binding.type == ShaderBindingType::UniformBuffer,
                   "ShaderHandler::UpdateGlobals", "binding type mismatch",
                   "shader_handle", handle, "binding_index",
                   field_update.binding_index, "expected_type",
                   GetShaderBindingTypeLabel(ShaderBindingType::UniformBuffer),
                   "expected_type_value",
                   ToUnderlying(ShaderBindingType::UniformBuffer),
                   "actual_type", GetShaderBindingTypeLabel(binding.type),
                   "actual_type_value", ToUnderlying(binding.type));

      WriteBindingFieldToMappedUbo(buffer, shader->bound_global_ubo_offset,
                                   binding, field_update.field_index,
                                   field_update.data, field_update.size);
    }
  }

  if (update.buffer_bindings != nullptr && !update.buffer_bindings->IsEmpty()) {
    for (const auto& buffer_update : *update.buffer_bindings) {
      COMET_ASSERT(buffer_update.binding_index < shader->bindings.GetSize(),
                   "ShaderHandler::UpdateGlobals",
                   "global buffer binding index is invalid", "shader_handle",
                   handle, "binding_index", buffer_update.binding_index,
                   "binding_count", shader->bindings.GetSize());

      const auto& binding{shader->bindings[buffer_update.binding_index]};

      COMET_ASSERT(binding.scope == ShaderBindingScope::Global,
                   "ShaderHandler::UpdateGlobals", "binding scope mismatch",
                   "shader_handle", handle, "binding_index",
                   buffer_update.binding_index, "expected_scope",
                   GetShaderBindingScopeLabel(ShaderBindingScope::Global),
                   "expected_scope_value",
                   ToUnderlying(ShaderBindingScope::Global), "actual_scope",
                   GetShaderBindingScopeLabel(binding.scope),
                   "actual_scope_value", ToUnderlying(binding.scope));
      COMET_ASSERT(binding.type == ShaderBindingType::UniformBuffer ||
                       binding.type == ShaderBindingType::StorageBuffer,
                   "ShaderHandler::UpdateGlobals", "binding type mismatch",
                   "shader_handle", handle, "binding_index",
                   buffer_update.binding_index, "expected_type",
                   GetShaderBindingTypeLabel(ShaderBindingType::UniformBuffer),
                   "expected_type_value",
                   ToUnderlying(ShaderBindingType::UniformBuffer),
                   "actual_type", GetShaderBindingTypeLabel(binding.type),
                   "actual_type_value", ToUnderlying(binding.type));

      UpdateDescriptorSetBuffer(
          set_handle, binding, buffer_update.buffer_handle,
          buffer_update.buffer_size, buffer_update.buffer_offset);
    }
  }

  if (update.image_bindings != nullptr && !update.image_bindings->IsEmpty()) {
    for (const auto& image_update : *update.image_bindings) {
      COMET_ASSERT(image_update.binding_index < shader->bindings.GetSize(),
                   "ShaderHandler::UpdateGlobals",
                   "global image binding index is invalid", "shader_handle",
                   handle, "binding_index", image_update.binding_index,
                   "binding_count", shader->bindings.GetSize());

      const auto& binding{shader->bindings[image_update.binding_index]};

      COMET_ASSERT(binding.scope == ShaderBindingScope::Global,
                   "ShaderHandler::UpdateGlobals", "binding scope mismatch",
                   "shader_handle", handle, "binding_index",
                   image_update.binding_index, "expected_scope",
                   GetShaderBindingScopeLabel(ShaderBindingScope::Global),
                   "expected_scope_value",
                   ToUnderlying(ShaderBindingScope::Global), "actual_scope",
                   GetShaderBindingScopeLabel(binding.scope),
                   "actual_scope_value", ToUnderlying(binding.scope));
      COMET_ASSERT(IsImageBindingType(binding.type),
                   "ShaderHandler::UpdateGlobals", "binding type mismatch",
                   "shader_handle", handle, "binding_index",
                   image_update.binding_index, "expected_image_binding", true,
                   "actual_type", GetShaderBindingTypeLabel(binding.type),
                   "actual_type_value", ToUnderlying(binding.type));
      COMET_ASSERT(image_update.descriptors != nullptr,
                   "ShaderHandler::UpdateGlobals", "image descriptors are null",
                   "shader_handle", handle, "binding_index",
                   image_update.binding_index);
      COMET_ASSERT(image_update.descriptor_count > 0,
                   "ShaderHandler::UpdateGlobals",
                   "image descriptor count is zero", "shader_handle", handle,
                   "binding_index", image_update.binding_index);

      UpdateDescriptorSetImages(set_handle, binding, image_update.descriptors,
                                image_update.descriptor_count);
    }
  }
}

void ShaderHandler::UpdatePass(ShaderHandle handle,
                               const ShaderPassUpdate& update) {
  auto* shader{Get(handle)};

  const auto image_index{context_->GetImageIndex()};

  if (shader->storage_descriptor_data.descriptor_set_handles.IsEmpty()) {
    return;
  }

  const auto set_handle{
      shader->storage_descriptor_data.descriptor_set_handles[image_index]};

  if (update.buffer_bindings != nullptr && !update.buffer_bindings->IsEmpty()) {
    for (const auto& buffer_update : *update.buffer_bindings) {
      COMET_ASSERT(buffer_update.binding_index < shader->bindings.GetSize(),
                   "ShaderHandler::UpdatePass",
                   "pass buffer binding index is invalid", "shader_handle",
                   handle, "binding_index", buffer_update.binding_index,
                   "binding_count", shader->bindings.GetSize());

      const auto& binding{shader->bindings[buffer_update.binding_index]};

      COMET_ASSERT(binding.scope == ShaderBindingScope::Pass,
                   "ShaderHandler::UpdatePass", "binding scope mismatch",
                   "shader_handle", handle, "binding_index",
                   buffer_update.binding_index, "expected_scope",
                   GetShaderBindingScopeLabel(ShaderBindingScope::Pass),
                   "expected_scope_value",
                   ToUnderlying(ShaderBindingScope::Pass), "actual_scope",
                   GetShaderBindingScopeLabel(binding.scope),
                   "actual_scope_value", ToUnderlying(binding.scope));
      COMET_ASSERT(
          binding.type == ShaderBindingType::UniformBuffer ||
              binding.type == ShaderBindingType::StorageBuffer,
          "ShaderHandler::UpdatePass", "binding type mismatch", "shader_handle",
          handle, "binding_index", buffer_update.binding_index, "expected_type",
          GetShaderBindingTypeLabel(ShaderBindingType::UniformBuffer),
          "expected_type_value", ToUnderlying(ShaderBindingType::UniformBuffer),
          "actual_type", GetShaderBindingTypeLabel(binding.type),
          "actual_type_value", ToUnderlying(binding.type));

      UpdateDescriptorSetBuffer(
          set_handle, binding, buffer_update.buffer_handle,
          buffer_update.buffer_size, buffer_update.buffer_offset);
    }
  }

  if (update.image_bindings != nullptr && !update.image_bindings->IsEmpty()) {
    for (const auto& image_update : *update.image_bindings) {
      COMET_ASSERT(image_update.binding_index < shader->bindings.GetSize(),
                   "ShaderHandler::UpdatePass",
                   "pass image binding index is invalid", "shader_handle",
                   handle, "binding_index", image_update.binding_index,
                   "binding_count", shader->bindings.GetSize());

      const auto& binding{shader->bindings[image_update.binding_index]};

      COMET_ASSERT(binding.scope == ShaderBindingScope::Pass,
                   "ShaderHandler::UpdatePass", "binding scope mismatch",
                   "shader_handle", handle, "binding_index",
                   image_update.binding_index, "expected_scope",
                   GetShaderBindingScopeLabel(ShaderBindingScope::Pass),
                   "expected_scope_value",
                   ToUnderlying(ShaderBindingScope::Pass), "actual_scope",
                   GetShaderBindingScopeLabel(binding.scope),
                   "actual_scope_value", ToUnderlying(binding.scope));
      COMET_ASSERT(IsImageBindingType(binding.type),
                   "ShaderHandler::UpdatePass", "binding type mismatch",
                   "shader_handle", handle, "binding_index",
                   image_update.binding_index, "expected_image_binding", true,
                   "actual_type", GetShaderBindingTypeLabel(binding.type),
                   "actual_type_value", ToUnderlying(binding.type));
      COMET_ASSERT(image_update.descriptors != nullptr,
                   "ShaderHandler::UpdatePass", "image descriptors are null",
                   "shader_handle", handle, "binding_index",
                   image_update.binding_index);
      COMET_ASSERT(image_update.descriptor_count > 0,
                   "ShaderHandler::UpdatePass",
                   "image descriptor count is zero", "shader_handle", handle,
                   "binding_index", image_update.binding_index);

      UpdateDescriptorSetImages(set_handle, binding, image_update.descriptors,
                                image_update.descriptor_count);
    }
  }
}

void ShaderHandler::UpdateInstance(ShaderHandle handle,
                                   MaterialHandle material_handle,
                                   const ShaderInstanceUpdate& update) {
  UpdateInstance(handle, GetMaterialHandler()->Get(material_handle), update);
}

void ShaderHandler::UpdateInstance(ShaderHandle handle,
                                   const Material* material,
                                   const ShaderInstanceUpdate& update) {
  auto* shader{Get(handle)};
  COMET_ASSERT(material != nullptr, "ShaderHandler::UpdateInstance",
               "material is null", "shader_handle", handle);
  COMET_ASSERT(internal::HasBindingInSet(shader, shaderconsts::kMaterialSet),
               "ShaderHandler::UpdateInstance", "shader has no material set",
               "shader_handle", shader->handle, "render_pass_handle",
               shader->render_pass_handle);
  COMET_ASSERT(shader->bind_type == PipelineBindType::Graphics,
               "ShaderHandler::UpdateInstance",
               "materials are only supported on graphics shaders",
               "shader_handle", shader->handle);

  auto& instance{GetInstance(shader, material)};
  const auto image_index{context_->GetImageIndex()};

  if (instance.descriptor_data.descriptor_set_handles.IsEmpty()) {
    return;
  }

  const auto set_handle{
      instance.descriptor_data.descriptor_set_handles[image_index]};

  shader->bound_instance_ubo_offset =
      shader->global_ubo_data.stride + instance.offset;

  if (update.field_updates != nullptr && !update.field_updates->IsEmpty()) {
    auto& buffer{shader->uniform_buffers[image_index]};
    ScopedMappedBuffer mapped{buffer};

    for (const auto& field_update : *update.field_updates) {
      COMET_ASSERT(field_update.binding_index < shader->bindings.GetSize(),
                   "ShaderHandler::UpdateInstance",
                   "instance field binding index is invalid", "shader_handle",
                   handle, "binding_index", field_update.binding_index,
                   "binding_count", shader->bindings.GetSize());

      const auto& binding{shader->bindings[field_update.binding_index]};

      COMET_ASSERT(binding.scope == ShaderBindingScope::Material,
                   "ShaderHandler::UpdateInstance", "binding scope mismatch",
                   "shader_handle", handle, "binding_index",
                   field_update.binding_index, "expected_scope",
                   GetShaderBindingScopeLabel(ShaderBindingScope::Material),
                   "expected_scope_value",
                   ToUnderlying(ShaderBindingScope::Material), "actual_scope",
                   GetShaderBindingScopeLabel(binding.scope),
                   "actual_scope_value", ToUnderlying(binding.scope));
      COMET_ASSERT(binding.type == ShaderBindingType::UniformBuffer,
                   "ShaderHandler::UpdateInstance", "binding type mismatch",
                   "shader_handle", handle, "binding_index",
                   field_update.binding_index, "expected_type",
                   GetShaderBindingTypeLabel(ShaderBindingType::UniformBuffer),
                   "expected_type_value",
                   ToUnderlying(ShaderBindingType::UniformBuffer),
                   "actual_type", GetShaderBindingTypeLabel(binding.type),
                   "actual_type_value", ToUnderlying(binding.type));

      WriteBindingFieldToMappedUbo(buffer, shader->bound_instance_ubo_offset,
                                   binding, field_update.field_index,
                                   field_update.data, field_update.size);
    }
  }

  if (update.buffer_bindings != nullptr && !update.buffer_bindings->IsEmpty()) {
    for (const auto& buffer_update : *update.buffer_bindings) {
      COMET_ASSERT(buffer_update.binding_index < shader->bindings.GetSize(),
                   "ShaderHandler::UpdateInstance",
                   "instance buffer binding index is invalid", "shader_handle",
                   handle, "binding_index", buffer_update.binding_index,
                   "binding_count", shader->bindings.GetSize());

      const auto& binding{shader->bindings[buffer_update.binding_index]};

      COMET_ASSERT(binding.scope == ShaderBindingScope::Material,
                   "ShaderHandler::UpdateInstance", "binding scope mismatch",
                   "shader_handle", handle, "binding_index",
                   buffer_update.binding_index, "expected_scope",
                   GetShaderBindingScopeLabel(ShaderBindingScope::Material),
                   "expected_scope_value",
                   ToUnderlying(ShaderBindingScope::Material), "actual_scope",
                   GetShaderBindingScopeLabel(binding.scope),
                   "actual_scope_value", ToUnderlying(binding.scope));
      COMET_ASSERT(binding.type == ShaderBindingType::UniformBuffer ||
                       binding.type == ShaderBindingType::StorageBuffer,
                   "ShaderHandler::UpdateInstance", "binding type mismatch",
                   "shader_handle", handle, "binding_index",
                   buffer_update.binding_index, "expected_type",
                   GetShaderBindingTypeLabel(ShaderBindingType::UniformBuffer),
                   "expected_type_value",
                   ToUnderlying(ShaderBindingType::UniformBuffer),
                   "actual_type", GetShaderBindingTypeLabel(binding.type),
                   "actual_type_value", ToUnderlying(binding.type));

      UpdateDescriptorSetBuffer(
          set_handle, binding, buffer_update.buffer_handle,
          buffer_update.buffer_size, buffer_update.buffer_offset);
    }
  }

  if (update.image_bindings != nullptr && !update.image_bindings->IsEmpty()) {
    for (const auto& image_update : *update.image_bindings) {
      COMET_ASSERT(image_update.binding_index < shader->bindings.GetSize(),
                   "ShaderHandler::UpdateInstance",
                   "instance image binding index is invalid", "shader_handle",
                   handle, "binding_index", image_update.binding_index,
                   "binding_count", shader->bindings.GetSize());

      const auto& binding{shader->bindings[image_update.binding_index]};

      COMET_ASSERT(binding.scope == ShaderBindingScope::Material,
                   "ShaderHandler::UpdateInstance", "binding scope mismatch",
                   "shader_handle", handle, "binding_index",
                   image_update.binding_index, "expected_scope",
                   GetShaderBindingScopeLabel(ShaderBindingScope::Material),
                   "expected_scope_value",
                   ToUnderlying(ShaderBindingScope::Material), "actual_scope",
                   GetShaderBindingScopeLabel(binding.scope),
                   "actual_scope_value", ToUnderlying(binding.scope));
      COMET_ASSERT(IsImageBindingType(binding.type),
                   "ShaderHandler::UpdateInstance", "binding type mismatch",
                   "shader_handle", handle, "binding_index",
                   image_update.binding_index, "expected_image_binding", true,
                   "actual_type", GetShaderBindingTypeLabel(binding.type),
                   "actual_type_value", ToUnderlying(binding.type));
      COMET_ASSERT(image_update.descriptors != nullptr,
                   "ShaderHandler::UpdateInstance",
                   "image descriptors are null", "shader_handle", handle,
                   "binding_index", image_update.binding_index);
      COMET_ASSERT(image_update.descriptor_count > 0,
                   "ShaderHandler::UpdateInstance",
                   "image descriptor count is zero", "shader_handle", handle,
                   "binding_index", image_update.binding_index);

      UpdateDescriptorSetImages(set_handle, binding, image_update.descriptors,
                                image_update.descriptor_count);

      if (image_index == 0) {
        auto* runtime_binding{FindBindingRuntimeData(
            instance.binding_data, image_update.binding_index)};

        if (runtime_binding == nullptr) {
          ShaderBindingImageRuntimeData new_runtime_binding{};
          new_runtime_binding.binding_index = image_update.binding_index;
          new_runtime_binding.descriptors =
              Array<ShaderImageDescriptor>{&general_allocator_};
          new_runtime_binding.descriptors.PushFromRange(
              image_update.descriptors, image_update.descriptor_count);
          instance.binding_data.image_bindings.EmplaceLast(
              std::move(new_runtime_binding));
        } else {
          runtime_binding->descriptors.Clear();
          runtime_binding->descriptors.PushFromRange(
              image_update.descriptors, image_update.descriptor_count);
        }
      }
    }
  }
}

void ShaderHandler::PushConstants(ShaderHandle handle,
                                  const ShaderPushConstantsUpdate& update) {
  const auto* shader{Get(handle)};

  if (update.blocks == nullptr) {
    return;
  }

  const auto current_frame{context_->GetFrameInFlightIndex()};
  auto& frame_data{context_->GetFrameData(current_frame)};

  const auto command_buffer_handle{frame_data.command_buffer_handle};

  const auto pipeline{shader->bind_type == PipelineBindType::Graphics
                          ? shader->graphics_pipeline
                          : shader->compute_pipeline};

  COMET_ASSERT(pipeline, "ShaderHandler::PushConstants",
               "pipeline handle is invalid", "shader_handle", handle);

  for (const auto& block_update : *update.blocks) {
    COMET_ASSERT(
        block_update.block_index < shader->push_constant_blocks.GetSize(),
        "ShaderHandler::PushConstants", "push constant block index is invalid",
        "shader_handle", handle, "block_index", block_update.block_index,
        "block_count", shader->push_constant_blocks.GetSize());

    const auto& block{shader->push_constant_blocks[block_update.block_index]};
    COMET_ASSERT(block_update.data != nullptr, "ShaderHandler::PushConstants",
                 "push constant block data is null", "shader_handle", handle,
                 "block_index", block_update.block_index);
    COMET_ASSERT(block_update.size <= static_cast<u32>(block.size),
                 "ShaderHandler::PushConstants",
                 "push constant update exceeds block size", "shader_handle",
                 handle, "block_index", block_update.block_index, "update_size",
                 block_update.size, "block_size", block.size);

    if (shader->graphics_pipeline && IsGraphicsStage(block.stages)) {
      vkCmdPushConstants(
          command_buffer_handle,
          pipeline_handler_->GetNativeLayoutHandle(shader->graphics_pipeline),
          block.stages, static_cast<u32>(block.offset),
          static_cast<u32>(block_update.size), block_update.data);
    }

    if (shader->compute_pipeline && IsComputeStage(block.stages)) {
      vkCmdPushConstants(
          command_buffer_handle,
          pipeline_handler_->GetNativeLayoutHandle(shader->compute_pipeline),
          block.stages, static_cast<u32>(block.offset),
          static_cast<u32>(block_update.size), block_update.data);
    }
  }
}

void ShaderHandler::BindMaterial(ShaderHandle handle,
                                 MaterialHandle material_handle) {
  BindMaterial(Get(handle), GetMaterialHandler()->Get(material_handle));
}

void ShaderHandler::BindMaterial(ShaderHandle handle,
                                 const Material* material) {
  BindMaterial(Get(handle), material);
}

void ShaderHandler::UnbindMaterial(ShaderHandle handle,
                                   MaterialHandle material_handle) {
  UnbindMaterial(Get(handle), GetMaterialHandler()->Get(material_handle));
}

void ShaderHandler::UnbindMaterial(ShaderHandle handle,
                                   const Material* material) {
  UnbindMaterial(Get(handle), material);
}

void ShaderHandler::UnbindMaterialFromAllShaders(const Material* material) {
  COMET_ASSERT(material != nullptr,
               "ShaderHandler::UnbindMaterialFromAllShaders",
               "material is null");

  shaders_.ForEachLive([this, material](ShaderHandle, Shader* shader) {
    if (!HasMaterial(shader, material)) {
      return;
    }

    UnbindMaterial(shader, material);
  });
}

bool ShaderHandler::HasMaterial(ShaderHandle handle,
                                MaterialHandle material_handle) const {
  return HasMaterial(Get(handle), GetMaterialHandler()->Get(material_handle));
}

bool ShaderHandler::HasMaterial(ShaderHandle handle,
                                const Material* material) const {
  return HasMaterial(Get(handle), material);
}

ShaderBindingIndex ShaderHandler::GetBindingIndex(ShaderHandle handle, u32 set,
                                                  u32 binding) const {
  const auto* shader{Get(handle)};

  for (ShaderBindingIndex i{0}; i < shader->bindings.GetSize(); ++i) {
    const auto& shader_binding{shader->bindings[i]};

    if (shader_binding.set == set && shader_binding.binding == binding) {
      return i;
    }
  }

  COMET_ASSERT(false, "ShaderHandler::GetBindingIndex",
               "shader binding not found", "shader_handle", handle, "set", set,
               "binding", binding);
  return 0;
}

void ShaderHandler::OnInitialize() {
  general_allocator_.Initialize();
  shader_instance_allocator_.Initialize();
  shaders_.Initialize();
  descriptor_set_layout_count_ = context_->GetImageCount();

  material_handler_->SetDestroyCallback(
      [](const Material* material, void* user_data) {
        auto* shader_handler{static_cast<ShaderHandler*>(user_data)};
        shader_handler->UnbindMaterialFromAllShaders(material);
      },
      this);
}

void ShaderHandler::OnShutdown() {
  material_handler_->SetDestroyCallback(nullptr, nullptr);
  descriptor_set_layout_count_ = 0;

  memory::PlatformAllocator tmp_allocator{memory::kEngineMemoryTagRendering};
  auto handles_to_destroy{Array<ShaderHandle>::WithCapacity(
      &tmp_allocator, shaders_.GetLiveCount())};

  shaders_.ForEachLive(
      [&handles_to_destroy](ShaderHandle handle, const Shader*) {
        handles_to_destroy.PushLast(handle);
      });

  for (const auto handle : handles_to_destroy) {
    const auto ref_count{shaders_.GetRefCount(handle)};

    if (ref_count > 0) {
      [[maybe_unused]] const auto key{shaders_.GetSourceId(handle)};

      COMET_LOG_WARNING(LoggerType::Rendering, "ShaderHandler::OnShutdown",
                        "forcing shader destruction", "shader_handle", handle,
                        "ref_count", ref_count, "shader_resource_id",
                        key.shader_resource_id, "render_pass_handle",
                        key.render_pass_handle, "bind_type",
                        GetPipelineBindTypeLabel(key.bind_type),
                        "bind_type_value", ToUnderlying(key.bind_type));
    }

    auto* shader{shaders_.Drain(handle)};

    if (shader == nullptr) {
      continue;
    }

    COMET_ASSERT(shader->handle == handle, "ShaderHandler::OnShutdown",
                 "shader handle mismatch", "expected_handle", handle,
                 "actual_handle", shader->handle, "shader_resource_id",
                 shader->id, "render_pass_handle", shader->render_pass_handle);

    DestroyShader(shader);
  }

  shaders_.Destroy();
  shader_instance_allocator_.Destroy();
  general_allocator_.Destroy();
}

Shader* ShaderHandler::Get(ShaderHandle handle) {
  auto* shader{shaders_.TryGet(handle)};
  COMET_ASSERT(shader != nullptr, "ShaderHandler::Get", "shader not found",
               "shader_handle", handle);
  ;
  return shader;
}

const Shader* ShaderHandler::Get(ShaderHandle handle) const {
  const auto* shader{shaders_.TryGet(handle)};
  COMET_ASSERT(shader != nullptr, "ShaderHandler::Get", "shader not found",
               "shader_handle", handle);
  return shader;
}

MaterialInstance& ShaderHandler::GetInstance(Shader* shader,
                                             const Material* material) {
  COMET_ASSERT(shader != nullptr, "ShaderHandler::GetInstance",
               "shader is null");
  COMET_ASSERT(material != nullptr, "ShaderHandler::GetInstance",
               "material is null");

  auto* index{shader->instances.indices.TryGet(material->id)};
  COMET_ASSERT(index != nullptr, "ShaderHandler::GetInstance",
               "material is not bound to shader", "shader_handle",
               shader->handle, "material_resource_id", material->id,
               "render_pass_handle", shader->render_pass_handle);

  return shader->instances.list[*index];
}

const MaterialInstance* ShaderHandler::TryGetInstance(
    const Shader* shader, const Material* material) const {
  if (shader == nullptr || material == nullptr) {
    return nullptr;
  }

  auto* index{shader->instances.indices.TryGet(material->id)};
  if (index == nullptr) {
    return nullptr;
  }

  return &shader->instances.list[*index];
}

Shader* ShaderHandler::GenerateShader(
    const ShaderDescr& descr, const resource::ShaderResource* shader_resource) {
  COMET_ASSERT(shader_resource != nullptr, "ShaderHandler::GenerateShader",
               "shader resource is null");

  auto* shader{shader_instance_allocator_.AllocateOneAndPopulate<Shader>()};
  shader->handle = ShaderHandle::Invalid();
  shader->id = shader_resource->GetId();
  shader->render_pass_handle = descr.render_pass_handle;
  shader->bind_type = descr.bind_type;

  const auto& rasterizer_resource{shader_resource->descr.rasterizer};

  shader->rasterizer = {
      .is_wireframe = rasterizer_resource.is_wireframe,
      .is_depth_bias = rasterizer_resource.is_depth_bias,
      .cull_mode = rasterizer_resource.cull_mode,
  };

  const auto& depth_stencil_resource{shader_resource->descr.depth_stencil};

  shader->depth_stencil = {
      .is_depth_test = depth_stencil_resource.is_depth_test,
      .is_depth_write = depth_stencil_resource.is_depth_write,
      .compare_op = depth_stencil_resource.compare_op,
  };

  shader->topology = shader_resource->descr.topology;
  shader->vertex_layout = shader_resource->descr.vertex_layout;

  shader->vertex_attributes =
      Array<VkVertexInputAttributeDescription>{&general_allocator_};
  shader->bindings = Array<ShaderBinding>{&general_allocator_};
  shader->push_constant_blocks =
      Array<ShaderPushConstantBlock>{&general_allocator_};
  shader->module_handles = Array<ShaderModuleHandle>{&general_allocator_};
  shader->push_constant_ranges =
      Array<VkPushConstantRange>{&general_allocator_};

  shader->global_descriptor_data.descriptor_set_handles =
      Array<VkDescriptorSet>{&general_allocator_};
  shader->storage_descriptor_data.descriptor_set_handles =
      Array<VkDescriptorSet>{&general_allocator_};

  shader->instances.list = Array<MaterialInstance>{&shader_instance_allocator_};
  shader->instances.indices =
      Map<resource::MaterialResourceId, u32>{&general_allocator_};

  HandleShaderModulesGeneration(shader, shader_resource);
  HandleAttributesGeneration(shader, shader_resource);
  HandleBindingsGeneration(shader, shader_resource);
  HandlePushConstantBlocksGeneration(shader, shader_resource);
  HandleDescriptorSetLayoutsGeneration(shader);
  HandlePipelineGeneration(shader);
  HandleDescriptorPoolGeneration(shader);
  HandleUboBufferGeneration(shader);

  AllocateGlobalDescriptorSets(shader);
  AllocateStorageDescriptorSets(shader);

  return shader;
}

void ShaderHandler::DestroyShader(Shader* shader) {
  COMET_ASSERT(shader != nullptr, "ShaderHandler::DestroyShader",
               "shader is null");

  const auto& device{context_->GetDevice()};

  shader->global_descriptor_data.descriptor_set_handles.Release();
  shader->storage_descriptor_data.descriptor_set_handles.Release();

  shader->global_descriptor_data.descriptor_pool_handle = VK_NULL_HANDLE;
  shader->storage_descriptor_data.descriptor_pool_handle = VK_NULL_HANDLE;
  shader->global_descriptor_data.update_frame = kInvalidFrameIndex;
  shader->storage_descriptor_data.update_frame = kInvalidFrameIndex;

  shader->global_ubo_data = {};
  shader->instance_ubo_data = {};

  if (!shader->uniform_buffers.IsEmpty()) {
    for (auto& buffer : shader->uniform_buffers) {
      if (IsBufferInitialized(buffer)) {
        DestroyBuffer(buffer);
      }
    }
  }

  shader->uniform_buffers.Release();

  if (shader->descriptor_pool_handle != VK_NULL_HANDLE) {
    DestroyDescriptorPool(device, shader->descriptor_pool_handle);
    shader->descriptor_pool_handle = VK_NULL_HANDLE;
  }

  for (auto& instance : shader->instances.list) {
    instance.descriptor_data.descriptor_set_handles.Release();
    instance.descriptor_data.descriptor_pool_handle = VK_NULL_HANDLE;
    instance.descriptor_data.update_frame = kInvalidFrameIndex;

    DestroyBindingRuntimeData(instance.binding_data);
  }

  shader->instances.list.Release();
  shader->instances.indices.Release();

  for (u32 i{0}; i < shader->layout_bindings.count; ++i) {
    const auto layout_handle{shader->layout_handles[i]};

    if (layout_handle != VK_NULL_HANDLE) {
      vkDestroyDescriptorSetLayout(device, layout_handle, nullptr);
      shader->layout_handles[i] = VK_NULL_HANDLE;
    }
  }

  shader->layout_bindings = {};
  shader->vertex_attributes.Release();

  for (auto& binding : shader->bindings) {
    binding.fields.Release();
  }

  for (auto& block : shader->push_constant_blocks) {
    block.fields.Release();
  }

  shader->bindings.Release();
  shader->push_constant_blocks.Release();

  if (shader_module_handler_->IsInitialized()) {
    for (const auto module_handle : shader->module_handles) {
      shader_module_handler_->Destroy(module_handle);
    }
  }

  shader->module_handles.Release();
  shader->push_constant_ranges.Release();
  shader->graphics_pipeline.Invalidate();
  shader->compute_pipeline.Invalidate();
  shader->handle.Invalidate();
  shader_instance_allocator_.Deallocate(shader);
}

bool ShaderHandler::HasVertexStage(
    const Shader& shader, const ShaderModuleHandler& shader_module_handler) {
  for (const auto module_handle : shader.module_handles) {
    if (shader_module_handler.GetStage(module_handle) ==
        VK_SHADER_STAGE_VERTEX_BIT) {
      return true;
    }
  }

  return false;
}

void ShaderHandler::PopulateVertexAttributes(
    ShaderVertexLayout layout,
    Array<VkVertexInputAttributeDescription>& attributes, u32& stride) {
  switch (layout) {
    case ShaderVertexLayout::None:
      attributes.Clear();
      stride = 0;
      return;

    case ShaderVertexLayout::SkinnedVertex:
      PopulateSkinnedVertexAttributes(attributes, stride);
      return;

    case ShaderVertexLayout::DebugLine:
      PopulateDebugLineVertexAttributes(attributes, stride);
      return;

    default:
      COMET_ASSERT(false, "ShaderHandler::PopulateVertexAttributes",
                   "unsupported shader vertex layout", "vertex_layout",
                   GetShaderVertexLayoutLabel(layout), "vertex_layout_value",
                   ToUnderlying(layout));
      attributes.Clear();
      stride = 0;
      return;
  }
}

void ShaderHandler::PopulateSkinnedVertexAttributes(
    Array<VkVertexInputAttributeDescription>& attributes, u32& stride) {
  attributes.Clear();
  attributes.Reserve(7);

  attributes.PushLast(VkVertexInputAttributeDescription{
      .location = 0,
      .binding = 0,
      .format = VK_FORMAT_R32G32B32_SFLOAT,
      .offset = static_cast<u32>(offsetof(geometry::SkinnedVertex, position)),
  });

  attributes.PushLast(VkVertexInputAttributeDescription{
      .location = 1,
      .binding = 0,
      .format = VK_FORMAT_R32G32B32_SFLOAT,
      .offset = static_cast<u32>(offsetof(geometry::SkinnedVertex, normal)),
  });

  attributes.PushLast(VkVertexInputAttributeDescription{
      .location = 2,
      .binding = 0,
      .format = VK_FORMAT_R32G32B32A32_SFLOAT,
      .offset = static_cast<u32>(offsetof(geometry::SkinnedVertex, tangent)),
  });

  attributes.PushLast(VkVertexInputAttributeDescription{
      .location = 3,
      .binding = 0,
      .format = VK_FORMAT_R32G32_SFLOAT,
      .offset = static_cast<u32>(offsetof(geometry::SkinnedVertex, uv)),
  });

  attributes.PushLast(VkVertexInputAttributeDescription{
      .location = 4,
      .binding = 0,
      .format = VK_FORMAT_R32G32B32A32_SFLOAT,
      .offset = static_cast<u32>(offsetof(geometry::SkinnedVertex, color)),
  });

  attributes.PushLast(VkVertexInputAttributeDescription{
      .location = 5,
      .binding = 0,
      .format = VK_FORMAT_R16G16B16A16_UINT,
      .offset =
          static_cast<u32>(offsetof(geometry::SkinnedVertex, joint_indices)),
  });

  attributes.PushLast(VkVertexInputAttributeDescription{
      .location = 6,
      .binding = 0,
      .format = VK_FORMAT_R32G32B32A32_SFLOAT,
      .offset =
          static_cast<u32>(offsetof(geometry::SkinnedVertex, joint_weights)),
  });

  stride = static_cast<u32>(sizeof(geometry::SkinnedVertex));
}

void ShaderHandler::PopulateDebugLineVertexAttributes(
    Array<VkVertexInputAttributeDescription>& attributes, u32& stride) {
  attributes.Clear();
  attributes.Reserve(2);

  attributes.PushLast(VkVertexInputAttributeDescription{
      .location = 0,
      .binding = 0,
      .format = VK_FORMAT_R32G32B32A32_SFLOAT,
      .offset = static_cast<u32>(offsetof(GpuDebugLineVertex, position)),
  });

  attributes.PushLast(VkVertexInputAttributeDescription{
      .location = 1,
      .binding = 0,
      .format = VK_FORMAT_R32G32B32A32_SFLOAT,
      .offset = static_cast<u32>(offsetof(GpuDebugLineVertex, color)),
  });

  stride = static_cast<u32>(sizeof(GpuDebugLineVertex));
}

bool ShaderHandler::HasMaterial(const Shader* shader,
                                const Material* material) const {
  COMET_ASSERT(shader != nullptr, "ShaderHandler::HasMaterial",
               "shader is null");
  COMET_ASSERT(material != nullptr, "ShaderHandler::HasMaterial",
               "material is null");
  return shader->instances.indices.TryGet(material->id) != nullptr;
}

void ShaderHandler::BindMaterial(Shader* shader, const Material* material) {
  COMET_ASSERT(shader != nullptr, "ShaderHandler::BindMaterial",
               "shader is null");
  COMET_ASSERT(material != nullptr, "ShaderHandler::BindMaterial",
               "material is null");
  COMET_ASSERT(!HasMaterial(shader, material), "ShaderHandler::BindMaterial",
               "material is already bound", "shader_handle", shader->handle,
               "material_resource_id", material->id, "render_pass_handle",
               shader->render_pass_handle);
  COMET_ASSERT(shader->bind_type == PipelineBindType::Graphics,
               "ShaderHandler::BindMaterial",
               "materials are only supported on graphics shaders",
               "shader_handle", shader->handle);
  COMET_ASSERT(
      shader->instances.list.GetSize() < kMaxMaterialInstances,
      "ShaderHandler::BindMaterial", "maximum material instance count reached",
      "shader_handle", shader->handle, "material_instance_count",
      shader->instances.list.GetSize(), "max_material_instances",
      kMaxMaterialInstances, "render_pass_handle", shader->render_pass_handle);

  if (!internal::HasBindingInSet(shader, shaderconsts::kMaterialSet)) {
    COMET_ASSERT(false, "ShaderHandler::BindMaterial",
                 "shader has no material set", "shader_handle", shader->handle,
                 "render_pass_handle", shader->render_pass_handle);
    return;
  }

  MaterialInstance instance{};
  instance.descriptor_data.descriptor_set_handles =
      Array<VkDescriptorSet>{&general_allocator_};
  instance.binding_data.image_bindings =
      Array<ShaderBindingImageRuntimeData>{&general_allocator_};

  u32 material_image_binding_count{0};

  for (const auto& binding : shader->bindings) {
    if (binding.set == shaderconsts::kMaterialSet &&
        IsImageBindingType(binding.type)) {
      ++material_image_binding_count;
    }
  }

  instance.binding_data.image_bindings.Reserve(material_image_binding_count);
  const auto instance_index{static_cast<u32>(shader->instances.list.GetSize())};
  AllocateMaterialDescriptorSets(shader, instance);
  instance.material_resource_id = material->id;
  UpdateInstanceUboDescriptors(shader, instance, instance_index);

  auto& resolved_image_descriptors{*COMET_FRAME_ARRAY(ShaderImageDescriptor)};

  for (u32 image_index{0}; image_index < descriptor_set_layout_count_;
       ++image_index) {
    const auto set_handle{
        instance.descriptor_data.descriptor_set_handles[image_index]};

    for (const auto& binding : shader->bindings) {
      if (binding.set != shaderconsts::kMaterialSet) {
        continue;
      }

      if (IsImageBindingType(binding.type)) {
        CollectMaterialImageDescriptors(material, binding,
                                        resolved_image_descriptors);

        if (image_index == 0) {
          ShaderBindingImageRuntimeData runtime_binding{};
          runtime_binding.binding_index = binding.index;
          runtime_binding.descriptors =
              Array<ShaderImageDescriptor>{&general_allocator_};
          runtime_binding.descriptors.PushFromRange(resolved_image_descriptors);
          instance.binding_data.image_bindings.EmplaceLast(
              std::move(runtime_binding));
        }

        UpdateDescriptorSetImages(
            set_handle, binding, resolved_image_descriptors.GetData(),
            static_cast<u32>(resolved_image_descriptors.GetSize()));
      }
    }
  }

  shader->instances.indices.Emplace(material->id, instance_index);
  shader->instances.list.EmplaceLast(std::move(instance));
}

void ShaderHandler::UnbindMaterial(Shader* shader, const Material* material) {
  COMET_ASSERT(shader != nullptr, "ShaderHandler::UnbindMaterial",
               "shader is null");
  COMET_ASSERT(material != nullptr, "ShaderHandler::UnbindMaterial",
               "material is null");
  COMET_ASSERT(HasMaterial(shader, material), "ShaderHandler::UnbindMaterial",
               "material is not bound", "shader_handle", shader->handle,
               "material_resource_id", material->id, "render_pass_handle",
               shader->render_pass_handle);
  COMET_ASSERT(shader->bind_type == PipelineBindType::Graphics,
               "ShaderHandler::UnbindMaterial",
               "materials are only supported on graphics shaders",
               "shader_handle", shader->handle);

  auto* index_ptr{shader->instances.indices.TryGet(material->id)};
  COMET_ASSERT(index_ptr != nullptr, "ShaderHandler::UnbindMaterial",
               "material instance index is null", "shader_handle",
               shader->handle, "material_resource_id", material->id,
               "render_pass_handle", shader->render_pass_handle);

  const auto index{*index_ptr};
  auto& instances{shader->instances.list};
  auto& instance{instances[index]};
  auto& device{context_->GetDevice()};

  auto& descriptor_data{instance.descriptor_data};
  FreeDescriptor(device, descriptor_data.descriptor_set_handles,
                 descriptor_data.descriptor_pool_handle);

  DestroyBindingRuntimeData(instance.binding_data);

  const auto last_index{static_cast<u32>(instances.GetSize() - 1)};
  const auto removed_material_resource_id{material->id};

  if (index != last_index) {
    instances[index] = std::move(instances[last_index]);
    auto& moved_instance{instances[index]};
    shader->instances.indices.Set(moved_instance.material_resource_id, index);
    UpdateInstanceUboDescriptors(shader, moved_instance, index);
  }

  instances.Resize(last_index);
  shader->instances.indices.Remove(removed_material_resource_id);
}

void ShaderHandler::HandleShaderModulesGeneration(
    Shader* shader, const resource::ShaderResource* resource) {
  shader->module_handles.Reserve(
      resource->descr.shader_module_resource_ids.GetSize());

  for (const auto shader_module_resource_id :
       resource->descr.shader_module_resource_ids) {
    const auto module_handle{
        shader_module_handler_->GetOrGenerate(shader_module_resource_id)};
    COMET_ASSERT(module_handle, "ShaderHandler::HandleShaderModulesGeneration",
                 "shader module handle generation failed",
                 "shader_module_resource_id", shader_module_resource_id,
                 "shader_resource_id", shader->id, "render_pass_handle",
                 shader->render_pass_handle);

    shader->module_handles.PushLast(module_handle);
  }
}

void ShaderHandler::HandleAttributesGeneration(
    Shader* shader, const resource::ShaderResource* resource) {
  COMET_ASSERT(shader != nullptr, "ShaderHandler::HandleAttributesGeneration",
               "shader is null");
  COMET_ASSERT(resource != nullptr, "ShaderHandler::HandleAttributesGeneration",
               "shader resource is null");

  u32 stride{0};
  PopulateVertexAttributes(resource->descr.vertex_layout,
                           shader->vertex_attributes, stride);
  shader->vertex_attribute_stride = static_cast<VertexAttributeStride>(stride);
}

void ShaderHandler::HandleBindingsGeneration(
    Shader* shader, const resource::ShaderResource* resource) {
  shader->bindings.Reserve(resource->descr.bindings.GetSize());

  for (const auto& binding_descr : resource->descr.bindings) {
    ShaderBinding binding{};
    binding.type = binding_descr.type;
    binding.scope = binding_descr.scope;
    binding.layout = binding_descr.layout;
    binding.stages = ResolveStageFlags(binding_descr.stages);
    binding.index = static_cast<ShaderBindingIndex>(shader->bindings.GetSize());
    binding.set = binding_descr.set;
    binding.binding = binding_descr.binding;
    binding.descriptor_count = binding_descr.descriptor_count;
    binding.fields = Array<ShaderField>{&general_allocator_};
    binding.image_semantic = binding_descr.image_semantic;

    binding.fields.Reserve(binding_descr.fields.GetSize());

    for (const auto& field_descr : binding_descr.fields) {
      ShaderField field{};
      field.type = field_descr.type;
      field.array_count = field_descr.array_count;
      binding.fields.PushLast(field);
    }

    ComputeBindingLayout(binding);

    if (binding.scope == ShaderBindingScope::Global &&
        binding.type == ShaderBindingType::UniformBuffer) {
      const auto align{static_cast<ShaderOffset>(
          context_->GetDevice()
              .GetProperties()
              .limits.minUniformBufferOffsetAlignment)};

      shader->global_ubo_data.size =
          AlignOffset(shader->global_ubo_data.size, align);
      binding.offset = shader->global_ubo_data.size;
      shader->global_ubo_data.size += binding.size;
      shader->global_ubo_data.stages |= binding.stages;
    }

    if (binding.scope == ShaderBindingScope::Material &&
        binding.type == ShaderBindingType::UniformBuffer) {
      const auto align{static_cast<ShaderOffset>(
          context_->GetDevice()
              .GetProperties()
              .limits.minUniformBufferOffsetAlignment)};

      shader->instance_ubo_data.size =
          AlignOffset(shader->instance_ubo_data.size, align);
      binding.offset = shader->instance_ubo_data.size;
      shader->instance_ubo_data.size += binding.size;
      shader->instance_ubo_data.stages |= binding.stages;
    }

    if (binding.type == ShaderBindingType::UniformBuffer &&
        binding.scope != ShaderBindingScope::Global &&
        binding.scope != ShaderBindingScope::Material) {
      COMET_ASSERT(false, "ShaderHandler::HandleBindingsGeneration",
                   "pass or draw uniform buffers are not implemented",
                   "shader_handle", shader->handle, "binding_index",
                   binding.index, "binding_scope",
                   GetShaderBindingScopeLabel(binding.scope),
                   "binding_scope_value", ToUnderlying(binding.scope),
                   "render_pass_handle", shader->render_pass_handle);
    }

    shader->bindings.PushLast(std::move(binding));
  }
}

void ShaderHandler::HandlePushConstantBlocksGeneration(
    Shader* shader, const resource::ShaderResource* resource) {
  shader->push_constant_blocks.Reserve(
      resource->descr.push_constants.GetSize());
  shader->push_constant_ranges.Reserve(
      resource->descr.push_constants.GetSize());

  ShaderOffset push_constant_cursor{0};

  for (const auto& block_descr : resource->descr.push_constants) {
    ShaderPushConstantBlock block{};
    block.index = static_cast<ShaderPushConstantIndex>(
        shader->push_constant_blocks.GetSize());
    block.stages = ResolveStageFlags(block_descr.stages);
    block.fields = Array<ShaderField>{&general_allocator_};
    block.fields.Reserve(block_descr.fields.GetSize());

    for (const auto& field_descr : block_descr.fields) {
      ShaderField field{};
      field.type = field_descr.type;
      field.array_count = field_descr.array_count;
      block.fields.PushLast(field);
    }

    ComputePushConstantBlockLayout(block);

    push_constant_cursor = AlignOffset(push_constant_cursor, 4);
    block.offset = push_constant_cursor;
    push_constant_cursor += block.size;

    VkPushConstantRange range{};
    range.stageFlags = block.stages;
    range.offset = static_cast<u32>(block.offset);
    range.size = static_cast<u32>(block.size);

    shader->push_constant_ranges.PushLast(range);
    shader->push_constant_blocks.PushLast(std::move(block));
  }

#ifdef COMET_DEBUG_RENDERING
  [[maybe_unused]] const auto max_push_constants{
      context_->GetDevice().GetProperties().limits.maxPushConstantsSize};

  COMET_ASSERT(push_constant_cursor <= max_push_constants,
               "ShaderHandler::HandlePushConstantBlocksGeneration",
               "push constant data exceeds device limit", "shader_handle",
               shader->handle, "push_constant_size", push_constant_cursor,
               "max_push_constants_size", max_push_constants,
               "render_pass_handle", shader->render_pass_handle);
#endif  // COMET_DEBUG_RENDERING
}

void ShaderHandler::HandleDescriptorSetLayoutsGeneration(Shader* shader) const {
  shader->layout_bindings.count = 0;

  for (const auto& binding : shader->bindings) {
    COMET_ASSERT(binding.set < kDescriptorSetMaxLayoutCount,
                 "ShaderHandler::HandleDescriptorSetLayoutsGeneration",
                 "descriptor set index out of bounds", "shader_handle",
                 shader->handle, "set", binding.set, "max_set_count",
                 kDescriptorSetMaxLayoutCount, "render_pass_handle",
                 shader->render_pass_handle);
    auto& set_layout{shader->layout_bindings.list[binding.set]};

    COMET_ASSERT(set_layout.binding_count < kDescriptorBindingMaxCount,
                 "ShaderHandler::HandleDescriptorSetLayoutsGeneration",
                 "too many bindings in descriptor set", "shader_handle",
                 shader->handle, "set", binding.set, "binding_count",
                 set_layout.binding_count, "max_binding_count",
                 kDescriptorBindingMaxCount, "render_pass_handle",
                 shader->render_pass_handle);
    auto& vk_binding{set_layout.bindings[set_layout.binding_count++]};

    vk_binding.binding = binding.binding;
    vk_binding.descriptorCount = binding.descriptor_count;
    vk_binding.descriptorType = GetVkDescriptorType(binding.type);
    vk_binding.stageFlags = binding.stages;

    shader->layout_bindings.count =
        math::Max(shader->layout_bindings.count, binding.set + 1);
  }

  for (u32 i{0}; i < shader->layout_bindings.count; ++i) {
    const auto info{init::GenerateDescriptorSetLayoutCreateInfo(
        shader->layout_bindings.list[i])};

    COMET_CHECK_VK(
        vkCreateDescriptorSetLayout(context_->GetDevice(), &info, nullptr,
                                    &shader->layout_handles[i]),
        "ShaderHandler::HandleDescriptorSetLayoutsGeneration",
        "descriptor set layout creation failed", "shader_handle",
        shader->handle, "set", i, "render_pass_handle",
        shader->render_pass_handle);
  }
}

void ShaderHandler::HandlePipelineGeneration(Shader* shader) const {
  [[maybe_unused]] bool has_graphics_stage{false};
  [[maybe_unused]] bool has_compute_stage{false};

  for (const auto module_handle : shader->module_handles) {
    const auto stage{shader_module_handler_->GetStage(module_handle)};

    if (stage == VK_SHADER_STAGE_COMPUTE_BIT) {
      has_compute_stage = true;
    } else {
      has_graphics_stage = true;
    }
  }

  COMET_ASSERT(!(has_graphics_stage && has_compute_stage),
               "ShaderHandler::HandlePipelineGeneration",
               "shader mixes graphics and compute stages", "shader_handle",
               shader->handle, "render_pass_handle",
               shader->render_pass_handle);

  PipelineLayoutDescr layout_descr{};
  layout_descr.descriptor_set_layout_handles = &shader->layout_handles;
  layout_descr.descriptor_set_layout_count = shader->layout_bindings.count;
  layout_descr.push_constant_ranges = &shader->push_constant_ranges;

  const auto pipeline_layout_handle{
      pipeline_handler_->GenerateLayout(layout_descr)};

  COMET_ASSERT(pipeline_layout_handle,
               "ShaderHandler::HandlePipelineGeneration",
               "pipeline layout handle is invalid");

  switch (shader->bind_type) {
    case PipelineBindType::Graphics: {
      COMET_ASSERT(has_graphics_stage,
                   "ShaderHandler::HandlePipelineGeneration",
                   "graphics shader has no graphics stages");
      COMET_ASSERT(!has_compute_stage,
                   "ShaderHandler::HandlePipelineGeneration",
                   "graphics shader contains compute stage");
      COMET_ASSERT(shader->render_pass_handle,
                   "ShaderHandler::HandlePipelineGeneration",
                   "graphics shader requires render pass");

      if (HasVertexStage(*shader, *shader_module_handler_)) {
        COMET_ASSERT(shader->vertex_layout != ShaderVertexLayout::None,
                     "ShaderHandler::HandlePipelineGeneration",
                     "vertex stage requires vertex layout");
      }

      HandleGraphicsPipelineGeneration(shader, pipeline_layout_handle);
      break;
    }

    case PipelineBindType::Compute: {
      COMET_ASSERT(has_compute_stage, "ShaderHandler::HandlePipelineGeneration",
                   "compute shader has no compute stage");
      COMET_ASSERT(!has_graphics_stage,
                   "ShaderHandler::HandlePipelineGeneration",
                   "compute shader contains graphics stage");

      HandleComputePipelineGeneration(shader, pipeline_layout_handle);
      break;
    }

    default: {
      COMET_ASSERT(false, "ShaderHandler::HandlePipelineGeneration",
                   "unsupported shader pipeline kind");
      break;
    }
  }
}

void ShaderHandler::HandleGraphicsPipelineGeneration(
    Shader* shader, PipelineLayoutHandle pipeline_layout_handle) const {
  GraphicsPipelineDescr pipeline_descr{};
  pipeline_descr.render_pass_handle = shader->render_pass_handle;
  pipeline_descr.layout_handle = pipeline_layout_handle;

  if (!shader->vertex_attributes.IsEmpty()) {
    pipeline_descr.vertex_input_binding_description.binding = 0;
    pipeline_descr.vertex_input_binding_description.stride =
        shader->vertex_attribute_stride;
    pipeline_descr.vertex_input_binding_description.inputRate =
        VK_VERTEX_INPUT_RATE_VERTEX;
    pipeline_descr.vertex_attributes = &shader->vertex_attributes;
  }

  pipeline_descr.shader_stages =
      frame::FrameArray<VkPipelineShaderStageCreateInfo>{};
  pipeline_descr.shader_stages.Reserve(shader->module_handles.GetSize());

  for (const auto module_handle : shader->module_handles) {
    const auto stage{shader_module_handler_->GetStage(module_handle)};
    if (stage == VK_SHADER_STAGE_COMPUTE_BIT) {
      continue;
    }

    pipeline_descr.shader_stages.PushLast(
        init::GeneratePipelineShaderStageCreateInfo(
            stage, shader_module_handler_->GetNativeHandle(module_handle)));
  }

  pipeline_descr.input_assembly_state =
      init::GeneratePipelineInputAssemblyStateCreateInfo(shader->topology);
  pipeline_descr.rasterization_state =
      init::GeneratePipelineRasterizationStateCreateInfo(shader->rasterizer);
  pipeline_descr.color_blend_attachment_state =
      init::GeneratePipelineColorBlendAttachmentState();
  pipeline_descr.multisample_state =
      init::GeneratePipelineMultisampleStateCreateInfo();

  const auto sample_count{
      shader->render_pass_handle
          ? render_pass_handler_->GetSamples(shader->render_pass_handle)
          : VK_SAMPLE_COUNT_1_BIT};

  pipeline_descr.multisample_state.rasterizationSamples = sample_count;

  const auto is_multisampled{sample_count != VK_SAMPLE_COUNT_1_BIT};
  const auto is_sample_rate_shading{is_multisampled &&
                                    context_->IsSampleRateShading()};

  pipeline_descr.multisample_state.sampleShadingEnable =
      is_sample_rate_shading ? VK_TRUE : VK_FALSE;
  pipeline_descr.multisample_state.minSampleShading =
      is_sample_rate_shading ? .2f : .0f;

  pipeline_descr.depth_stencil_state =
      init::GeneratePipelineDepthStencilStateCreateInfo(shader->depth_stencil);

  shader->graphics_pipeline = pipeline_handler_->Generate(pipeline_descr);
}

void ShaderHandler::HandleComputePipelineGeneration(
    Shader* shader, PipelineLayoutHandle pipeline_layout_handle) const {
  ComputePipelineDescr pipeline_descr{};
  pipeline_descr.layout_handle = pipeline_layout_handle;

  for (const auto module_handle : shader->module_handles) {
    const auto stage{shader_module_handler_->GetStage(module_handle)};
    if (stage != VK_SHADER_STAGE_COMPUTE_BIT) {
      continue;
    }

    if (pipeline_descr.shader_stage.stage == VK_SHADER_STAGE_COMPUTE_BIT) {
      COMET_LOG_ERROR(LoggerType::Rendering,
                      "ShaderHandler::HandleComputePipelineGeneration",
                      "multiple compute stages are not supported",
                      "shader_handle", shader->handle, "render_pass_handle",
                      shader->render_pass_handle);
      continue;
    }

    pipeline_descr.shader_stage = init::GeneratePipelineShaderStageCreateInfo(
        stage, shader_module_handler_->GetNativeHandle(module_handle));
  }

  shader->compute_pipeline = pipeline_handler_->Generate(pipeline_descr);
}

void ShaderHandler::HandleDescriptorPoolGeneration(Shader* shader) const {
  constexpr VkDescriptorPoolSize pool_sizes[6]{
      {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1024},
      {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1024},
      {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4096},
      {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 4096},
      {VK_DESCRIPTOR_TYPE_SAMPLER, 1024},
      {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1024},
  };

  shader->descriptor_pool_handle =
      GenerateDescriptorPool(context_->GetDevice(), 1024, pool_sizes, 6,
                             VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT);

  shader->global_descriptor_data.descriptor_pool_handle =
      shader->descriptor_pool_handle;
  shader->storage_descriptor_data.descriptor_pool_handle =
      shader->descriptor_pool_handle;
}

void ShaderHandler::HandleUboBufferGeneration(Shader* shader) {
  const auto align{context_->GetDevice()
                       .GetProperties()
                       .limits.minUniformBufferOffsetAlignment};

  shader->global_ubo_data.stride = static_cast<ShaderOffset>(memory::AlignSize(
      shader->global_ubo_data.size, static_cast<memory::Alignment>(align)));

  shader->instance_ubo_data.stride = static_cast<ShaderOffset>(
      memory::AlignSize(shader->instance_ubo_data.size,
                        static_cast<memory::Alignment>(align)));

#ifdef COMET_DEBUG
  [[maybe_unused]] const auto max_range{
      context_->GetDevice().GetProperties().limits.maxUniformBufferRange};

  COMET_ASSERT(shader->global_ubo_data.stride < max_range,
               "ShaderHandler::HandleUboBufferGeneration",
               "global ubo size exceeds limit", "shader_handle", shader->handle,
               "ubo_stride", shader->global_ubo_data.stride,
               "max_uniform_buffer_range", max_range, "render_pass_handle",
               shader->render_pass_handle);
  COMET_ASSERT(shader->instance_ubo_data.stride < max_range,
               "ShaderHandler::HandleUboBufferGeneration",
               "instance ubo size exceeds limit", "shader_handle",
               shader->handle, "ubo_stride", shader->instance_ubo_data.stride,
               "max_uniform_buffer_range", max_range, "render_pass_handle",
               shader->render_pass_handle);
#endif  // COMET_DEBUG

  const auto buffer_size{static_cast<VkDeviceSize>(
      shader->global_ubo_data.stride +
      shader->instance_ubo_data.stride * kMaxMaterialInstances)};

  if (buffer_size == 0) {
    return;
  }

  shader->uniform_buffers = Array<Buffer>{&general_allocator_};
  shader->uniform_buffers.Resize(context_->GetImageCount());

  for (usize i{0}; i < shader->uniform_buffers.GetSize(); ++i) {
    shader->uniform_buffers[i] = GenerateBuffer(
        context_->GetAllocatorHandle(), buffer_size,
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, 0, 0,
        VK_SHARING_MODE_EXCLUSIVE, "shader->uniform_buffers");
  }

  shader->global_ubo_data.offset = 0;
  shader->bound_global_ubo_offset = 0;
}

void ShaderHandler::ComputeBindingLayout(ShaderBinding& binding) const {
  if (!IsBufferBindingType(binding.type) || binding.fields.IsEmpty()) {
    binding.size = 0;
    binding.stride = 0;
    return;
  }

  ShaderOffset cursor{0};

  for (ShaderFieldIndex i{0}; i < binding.fields.GetSize(); ++i) {
    auto& field{binding.fields[i]};
    const auto info{
        GetFieldLayoutInfo(binding.layout, field.type, field.array_count)};

    cursor = AlignOffset(cursor, info.alignment);

    field.offset = cursor;
    field.element_size = info.element_size;
    field.aligned_size = info.aligned_size;
    field.stride = info.stride;
    field.size = info.total_size;

    cursor += field.size;
  }

  binding.size = cursor;
  binding.stride = cursor;
}

void ShaderHandler::ComputePushConstantBlockLayout(
    ShaderPushConstantBlock& block) const {
  ShaderOffset cursor{0};

  for (ShaderFieldIndex i{0}; i < block.fields.GetSize(); ++i) {
    auto& field{block.fields[i]};

    const auto alignment{GetStd430Alignment(field.type)};
    const auto element_size{GetShaderVariableTypeSize(field.type)};
    const auto aligned_size{static_cast<ShaderVariableSize>(memory::AlignSize(
        element_size, static_cast<memory::Alignment>(alignment)))};

    cursor = AlignOffset(cursor, alignment);

    field.offset = cursor;
    field.element_size = element_size;
    field.aligned_size = aligned_size;
    field.stride = aligned_size;
    field.size =
        field.array_count > 1
            ? static_cast<ShaderVariableSize>(field.stride * field.array_count)
            : aligned_size;

    cursor += field.size;
  }

  block.size = cursor;
}

void ShaderHandler::AllocateGlobalDescriptorSets(Shader* shader) {
  if (!internal::HasBindingInSet(shader, shaderconsts::kGlobalSet)) {
    return;
  }

  frame::FrameArray<VkDescriptorSetLayout> layout_buffer{};
  shader->global_descriptor_data.descriptor_set_handles.Resize(
      descriptor_set_layout_count_);

  for (u32 i{0}; i < descriptor_set_layout_count_; ++i) {
    layout_buffer.PushLast(shader->layout_handles[shaderconsts::kGlobalSet]);
  }

  [[maybe_unused]] const auto is_allocated{descriptor_handler_->Generate(
      layout_buffer.GetData(),
      shader->global_descriptor_data.descriptor_set_handles.GetData(),
      descriptor_set_layout_count_, DescriptorType::Static)};

  COMET_ASSERT(is_allocated, "ShaderHandler::AllocateGlobalDescriptorSets",
               "global descriptor set allocation failed", "shader_handle",
               shader->handle, "descriptor_set_count",
               descriptor_set_layout_count_, "render_pass_handle",
               shader->render_pass_handle);

  for (u32 image_index{0}; image_index < descriptor_set_layout_count_;
       ++image_index) {
    const auto set_handle{
        shader->global_descriptor_data.descriptor_set_handles[image_index]};

    for (const auto& binding : shader->bindings) {
      if (binding.set != shaderconsts::kGlobalSet ||
          binding.type != ShaderBindingType::UniformBuffer) {
        continue;
      }

      UpdateDescriptorSetBuffer(
          set_handle, binding, shader->uniform_buffers[image_index].handle,
          binding.size, shader->global_ubo_data.offset + binding.offset);
    }
  }
}

void ShaderHandler::AllocateStorageDescriptorSets(Shader* shader) {
  if (!internal::HasBindingInSet(shader, shaderconsts::kPassSet)) {
    return;
  }

  frame::FrameArray<VkDescriptorSetLayout> layout_buffer{};
  shader->storage_descriptor_data.descriptor_set_handles.Resize(
      descriptor_set_layout_count_);

  for (u32 i{0}; i < descriptor_set_layout_count_; ++i) {
    layout_buffer.PushLast(shader->layout_handles[shaderconsts::kPassSet]);
  }

  [[maybe_unused]] const auto is_allocated{descriptor_handler_->Generate(
      layout_buffer.GetData(),
      shader->storage_descriptor_data.descriptor_set_handles.GetData(),
      descriptor_set_layout_count_, DescriptorType::Static)};

  COMET_ASSERT(is_allocated, "ShaderHandler::AllocateStorageDescriptorSets",
               "pass descriptor set allocation failed", "shader_handle",
               shader->handle, "descriptor_set_count",
               descriptor_set_layout_count_, "render_pass_handle",
               shader->render_pass_handle);

  // Storage/pass descriptor sets are only allocated here.
  // Actual buffer/image bindings are provided later by the rendering pass.
}

void ShaderHandler::AllocateMaterialDescriptorSets(Shader* shader,
                                                   MaterialInstance& instance) {
  if (!internal::HasBindingInSet(shader, shaderconsts::kMaterialSet)) {
    return;
  }

  frame::FrameArray<VkDescriptorSetLayout> layout_buffer{};
  instance.descriptor_data.descriptor_set_handles.Resize(
      descriptor_set_layout_count_);
  instance.descriptor_data.descriptor_pool_handle =
      shader->descriptor_pool_handle;

  for (u32 i{0}; i < descriptor_set_layout_count_; ++i) {
    layout_buffer.PushLast(shader->layout_handles[shaderconsts::kMaterialSet]);
  }

  [[maybe_unused]] const auto is_allocated{descriptor_handler_->Generate(
      layout_buffer.GetData(),
      instance.descriptor_data.descriptor_set_handles.GetData(),
      descriptor_set_layout_count_, DescriptorType::Static)};

  COMET_ASSERT(is_allocated, "ShaderHandler::AllocateMaterialDescriptorSets",
               "material descriptor set allocation failed", "shader_handle",
               shader->handle, "descriptor_set_count",
               descriptor_set_layout_count_, "render_pass_handle",
               shader->render_pass_handle);
}

void ShaderHandler::CollectMaterialTextureMaps(
    const Material* material, const ShaderBinding& binding,
    Array<const TextureMap*>& texture_maps) const {
  COMET_ASSERT(material != nullptr, "ShaderHandler::CollectMaterialTextureMaps",
               "material is null");
  COMET_ASSERT(IsImageBindingType(binding.type),
               "ShaderHandler::CollectMaterialTextureMaps",
               "binding is not an image binding", "binding_set", binding.set,
               "binding_binding", binding.binding, "binding_index",
               binding.index, "binding_type",
               GetShaderBindingTypeLabel(binding.type), "binding_type_value",
               ToUnderlying(binding.type));

  texture_maps.Clear();
  texture_maps.Reserve(binding.descriptor_count);

  switch (binding.image_semantic) {
    case ShaderImageBindingSemantic::MaterialDiffuse:
      COMET_ASSERT(binding.descriptor_count == 1,
                   "ShaderHandler::CollectMaterialTextureMaps",
                   "material diffuse binding descriptor count mismatch",
                   "binding_set", binding.set, "binding_binding",
                   binding.binding, "binding_index", binding.index,
                   "expected_descriptor_count", 1u, "actual_descriptor_count",
                   binding.descriptor_count);

      texture_maps.PushLast(&material->diffuse_map);
      return;

    case ShaderImageBindingSemantic::MaterialSpecular:
      COMET_ASSERT(binding.descriptor_count == 1,
                   "ShaderHandler::CollectMaterialTextureMaps",
                   "material specular binding descriptor count mismatch",
                   "binding_set", binding.set, "binding_binding",
                   binding.binding, "binding_index", binding.index,
                   "expected_descriptor_count", 1u, "actual_descriptor_count",
                   binding.descriptor_count);

      texture_maps.PushLast(&material->specular_map);
      return;

    case ShaderImageBindingSemantic::MaterialNormal:
      COMET_ASSERT(binding.descriptor_count == 1,
                   "ShaderHandler::CollectMaterialTextureMaps",
                   "material normal binding descriptor count mismatch",
                   "binding_set", binding.set, "binding_binding",
                   binding.binding, "binding_index", binding.index,
                   "expected_descriptor_count", 1u, "actual_descriptor_count",
                   binding.descriptor_count);

      texture_maps.PushLast(&material->normal_map);
      return;

    case ShaderImageBindingSemantic::MaterialTextures:
      COMET_ASSERT(binding.descriptor_count == 3,
                   "ShaderHandler::CollectMaterialTextureMaps",
                   "material textures binding descriptor count mismatch",
                   "binding_set", binding.set, "binding_binding",
                   binding.binding, "binding_index", binding.index,
                   "expected_descriptor_count", 3u, "actual_descriptor_count",
                   binding.descriptor_count);

      texture_maps.PushLast(&material->diffuse_map);
      texture_maps.PushLast(&material->specular_map);
      texture_maps.PushLast(&material->normal_map);
      return;

    default:
      COMET_ASSERT(false, "ShaderHandler::CollectMaterialTextureMaps",
                   "unsupported material image semantic", "binding_set",
                   binding.set, "binding_binding", binding.binding,
                   "binding_index", binding.index, "image_semantic",
                   GetShaderImageBindingSemanticLabel(binding.image_semantic),
                   "image_semantic_value",
                   ToUnderlying(binding.image_semantic));
      return;
  }
}

void ShaderHandler::CollectMaterialImageDescriptors(
    const Material* material, const ShaderBinding& binding,
    Array<ShaderImageDescriptor>& descriptors) const {
  auto& texture_maps{*COMET_FRAME_ARRAY(const TextureMap*)};
  CollectMaterialTextureMaps(material, binding, texture_maps);

  auto& image_layouts{
      *COMET_FRAME_ARRAY_WITH_CAPACITY(VkImageLayout, texture_maps.GetSize())};

  for (const auto* texture_map : texture_maps) {
    COMET_ASSERT(texture_map != nullptr,
                 "ShaderHandler::CollectMaterialImageDescriptors",
                 "texture map is null", "binding_set", binding.set,
                 "binding_binding", binding.binding, "binding_index",
                 binding.index);

    VkImageLayout image_layout{VK_IMAGE_LAYOUT_UNDEFINED};

    if (binding.type == ShaderBindingType::CombinedImageSampler ||
        binding.type == ShaderBindingType::SampledImage) {
      COMET_ASSERT(texture_map->texture_handle,
                   "ShaderHandler::CollectMaterialImageDescriptors",
                   "binding requires texture handle", "binding_set",
                   binding.set, "binding_binding", binding.binding,
                   "binding_index", binding.index, "binding_type",
                   GetShaderBindingTypeLabel(binding.type),
                   "binding_type_value", ToUnderlying(binding.type));

      const auto* texture{texture_handler_->Get(texture_map->texture_handle)};
      COMET_ASSERT(texture_map->texture_handle,
                   "ShaderHandler::CollectMaterialImageDescriptors",
                   "texture is null", "binding_set", binding.set,
                   "binding_binding", binding.binding, "binding_index",
                   binding.index, "texture_handle",
                   texture_map->texture_handle);

      image_layout = GetDescriptorImageLayout(texture);
    } else if (binding.type == ShaderBindingType::StorageImage) {
      image_layout = VK_IMAGE_LAYOUT_GENERAL;
    } else {
      image_layout = VK_IMAGE_LAYOUT_UNDEFINED;
    }

    image_layouts.PushLast(image_layout);
  }

  GenerateImageDescriptors(binding.type, texture_maps, image_layouts,
                           descriptors);
}

void ShaderHandler::UpdateDescriptorSetImages(
    VkDescriptorSet set_handle, const ShaderBinding& binding,
    const ShaderImageDescriptor* descriptors, u32 descriptor_count) const {
  COMET_ASSERT(
      descriptors != nullptr, "ShaderHandler::UpdateDescriptorSetImages",
      "image descriptors are null", "binding_set", binding.set,
      "binding_binding", binding.binding, "binding_index", binding.index);
  COMET_ASSERT(descriptor_count > 0, "ShaderHandler::UpdateDescriptorSetImages",
               "image descriptor count is zero", "binding_index",
               binding.index);
  COMET_ASSERT(
      descriptor_count == binding.descriptor_count,
      "ShaderHandler::UpdateDescriptorSetImages", "descriptor count mismatch",
      "binding_set", binding.set, "binding_binding", binding.binding,
      "binding_index", binding.index, "expected_descriptor_count",
      binding.descriptor_count, "actual_descriptor_count", descriptor_count);
  COMET_ASSERT(IsImageBindingType(binding.type),
               "ShaderHandler::UpdateDescriptorSetImages",
               "binding is not an image binding", "binding_set", binding.set,
               "binding_binding", binding.binding, "binding_index",
               binding.index, "binding_type",
               GetShaderBindingTypeLabel(binding.type), "binding_type_value",
               ToUnderlying(binding.type));

  frame::FrameArray<VkDescriptorImageInfo> image_infos{};
  image_infos.Reserve(descriptor_count);

  for (u32 i{0}; i < descriptor_count; ++i) {
    const auto& descriptor{descriptors[i]};

    const auto* texture{descriptor.texture_handle
                            ? texture_handler_->Get(descriptor.texture_handle)
                            : nullptr};

    const auto* sampler{descriptor.sampler_handle
                            ? sampler_handler_->Get(descriptor.sampler_handle)
                            : nullptr};

    VkDescriptorImageInfo image_info{};

    switch (binding.type) {
      case ShaderBindingType::CombinedImageSampler:
        COMET_ASSERT(texture != nullptr,
                     "ShaderHandler::UpdateDescriptorSetImages",
                     "combined image sampler requires texture", "binding_set",
                     binding.set, "binding_binding", binding.binding,
                     "binding_index", binding.index, "descriptor_index", i);
        COMET_ASSERT(sampler != nullptr,
                     "ShaderHandler::UpdateDescriptorSetImages",
                     "combined image sampler requires sampler", "binding_set",
                     binding.set, "binding_binding", binding.binding,
                     "binding_index", binding.index, "descriptor_index", i);

        image_info.imageView = texture->image.image_view_handle;
        image_info.sampler = sampler->native_handle;
        image_info.imageLayout = descriptor.image_layout;
        break;

      case ShaderBindingType::SampledImage:
        COMET_ASSERT(texture != nullptr,
                     "ShaderHandler::UpdateDescriptorSetImages",
                     "sampled image requires texture", "binding_set",
                     binding.set, "binding_binding", binding.binding,
                     "binding_index", binding.index, "descriptor_index", i);

        image_info.imageView = texture->image.image_view_handle;
        image_info.sampler = VK_NULL_HANDLE;
        image_info.imageLayout = descriptor.image_layout;
        break;

      case ShaderBindingType::Sampler:
        COMET_ASSERT(sampler != nullptr,
                     "ShaderHandler::UpdateDescriptorSetImages",
                     "sampler binding requires sampler", "binding_set",
                     binding.set, "binding_binding", binding.binding,
                     "binding_index", binding.index, "descriptor_index", i);

        image_info.imageView = VK_NULL_HANDLE;
        image_info.sampler = sampler->native_handle;
        image_info.imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        break;

      case ShaderBindingType::StorageImage:
        COMET_ASSERT(texture != nullptr,
                     "ShaderHandler::UpdateDescriptorSetImages",
                     "storage image requires texture", "binding_set",
                     binding.set, "binding_binding", binding.binding,
                     "binding_index", binding.index, "descriptor_index", i);

        image_info.imageView = texture->image.image_view_handle;
        image_info.sampler = VK_NULL_HANDLE;
        image_info.imageLayout = descriptor.image_layout;
        break;

      default:
        COMET_ASSERT(false, "ShaderHandler::UpdateDescriptorSetImages",
                     "unsupported image binding type", "binding_set",
                     binding.set, "binding_binding", binding.binding,
                     "binding_index", binding.index, "binding_type",
                     GetShaderBindingTypeLabel(binding.type),
                     "binding_type_value", ToUnderlying(binding.type));
        break;
    }

    image_infos.PushLast(image_info);
  }

  const auto write{init::GenerateImageWriteDescriptorSet(
      GetVkDescriptorType(binding.type), set_handle, image_infos.GetData(),
      descriptor_count, binding.binding)};

  vkUpdateDescriptorSets(context_->GetDevice(), 1, &write, 0, VK_NULL_HANDLE);
}

void ShaderHandler::UpdateDescriptorSetBuffer(VkDescriptorSet set_handle,
                                              const ShaderBinding& binding,
                                              VkBuffer buffer_handle,
                                              VkDeviceSize range,
                                              VkDeviceSize offset) const {
  COMET_ASSERT(binding.type == ShaderBindingType::UniformBuffer ||
                   binding.type == ShaderBindingType::StorageBuffer,
               "ShaderHandler::UpdateDescriptorSetBuffer",
               "binding is not a buffer binding", "binding_set", binding.set,
               "binding_binding", binding.binding, "binding_index",
               binding.index, "binding_type",
               GetShaderBindingTypeLabel(binding.type), "binding_type_value",
               ToUnderlying(binding.type));
  COMET_ASSERT(buffer_handle != VK_NULL_HANDLE,
               "ShaderHandler::UpdateDescriptorSetBuffer",
               "buffer handle is null", "binding_set", binding.set,
               "binding_binding", binding.binding, "binding_index",
               binding.index, "binding_type",
               GetShaderBindingTypeLabel(binding.type), "binding_type_value",
               ToUnderlying(binding.type));

  const auto buffer_info{
      init::GenerateDescriptorBufferInfo(buffer_handle, offset, range)};

  const auto write{init::GenerateBufferWriteDescriptorSet(
      GetVkDescriptorType(binding.type), set_handle, &buffer_info,
      binding.binding)};

  vkUpdateDescriptorSets(context_->GetDevice(), 1, &write, 0, VK_NULL_HANDLE);
}

void ShaderHandler::UpdateInstanceUboDescriptors(Shader* shader,
                                                 MaterialInstance& instance,
                                                 u32 instance_index) {
  COMET_ASSERT(shader != nullptr, "ShaderHandler::UpdateInstanceUboDescriptors",
               "shader is null");
  COMET_ASSERT(instance.descriptor_data.descriptor_set_handles.GetSize() ==
                   descriptor_set_layout_count_,
               "ShaderHandler::UpdateInstanceUboDescriptors",
               "material descriptor set count mismatch", "shader_handle",
               shader->handle, "expected_descriptor_set_count",
               descriptor_set_layout_count_, "actual_descriptor_set_count",
               instance.descriptor_data.descriptor_set_handles.GetSize(),
               "render_pass_handle", shader->render_pass_handle);

  instance.offset = shader->instance_ubo_data.stride * instance_index;

  bool has_material_ubo_binding{false};

  for (const auto& binding : shader->bindings) {
    if (binding.set == shaderconsts::kMaterialSet &&
        binding.type == ShaderBindingType::UniformBuffer) {
      has_material_ubo_binding = true;
      break;
    }
  }

  if (!has_material_ubo_binding) {
    return;
  }

  COMET_ASSERT(
      shader->uniform_buffers.GetSize() == descriptor_set_layout_count_,
      "ShaderHandler::UpdateInstanceUboDescriptors",
      "uniform buffer count mismatch", "shader_handle", shader->handle,
      "expected_uniform_buffer_count", descriptor_set_layout_count_,
      "actual_uniform_buffer_count", shader->uniform_buffers.GetSize(),
      "render_pass_handle", shader->render_pass_handle);

  for (u32 image_index{0}; image_index < descriptor_set_layout_count_;
       ++image_index) {
    const auto set_handle{
        instance.descriptor_data.descriptor_set_handles[image_index]};

    for (const auto& binding : shader->bindings) {
      if (binding.set != shaderconsts::kMaterialSet) {
        continue;
      }

      if (binding.type != ShaderBindingType::UniformBuffer) {
        continue;
      }

      UpdateDescriptorSetBuffer(
          set_handle, binding, shader->uniform_buffers[image_index].handle,
          binding.size,
          shader->global_ubo_data.stride + instance.offset + binding.offset);
    }
  }
}

void ShaderHandler::WriteBindingFieldToUbo(
    Buffer& buffer, ShaderOffset base_offset, const ShaderBinding& binding,
    ShaderFieldIndex field_index, const void* value, usize value_size) const {
  ScopedMappedBuffer mapped{buffer};
  WriteBindingFieldToMappedUbo(buffer, base_offset, binding, field_index, value,
                               value_size);
}

void ShaderHandler::WriteBindingFieldToMappedUbo(
    Buffer& buffer, ShaderOffset base_offset, const ShaderBinding& binding,
    ShaderFieldIndex field_index, const void* value, usize value_size) const {
  COMET_ASSERT(field_index < binding.fields.GetSize(),
               "ShaderHandler::WriteBindingFieldToMappedUbo",
               "field index is invalid", "binding_index", binding.index,
               "field_index", field_index, "field_count",
               binding.fields.GetSize());
  COMET_ASSERT(value != nullptr, "ShaderHandler::WriteBindingFieldToMappedUbo",
               "field value is null", "binding_index", binding.index,
               "field_index", field_index);

  const auto& field{binding.fields[field_index]};
  const auto raw_size{static_cast<usize>(field.element_size)};
  const auto copy_size{value_size == 0 ? raw_size : value_size};

  COMET_ASSERT(copy_size <= field.size,
               "ShaderHandler::WriteBindingFieldToMappedUbo",
               "field update exceeds field size", "binding_index",
               binding.index, "field_index", field_index, "copy_size",
               copy_size, "field_size", field.size);

  CopyToBuffer(buffer, value, copy_size,
               base_offset + binding.offset + field.offset);
}

ShaderBindingImageRuntimeData* ShaderHandler::FindBindingRuntimeData(
    ShaderBindingRuntimeData& binding_data,
    ShaderBindingIndex binding_index) const {
  for (auto& runtime_binding : binding_data.image_bindings) {
    if (runtime_binding.binding_index == binding_index) {
      return &runtime_binding;
    }
  }

  return nullptr;
}

void ShaderHandler::DestroyBindingRuntimeData(
    ShaderBindingRuntimeData& binding_data) const {
  for (auto& runtime_binding : binding_data.image_bindings) {
    runtime_binding.descriptors.Release();
  }

  binding_data.image_bindings.Release();
}

const MaterialHandler* ShaderHandler::GetMaterialHandler() const {
  return material_handler_;
}
}  // namespace vk
}  // namespace rendering
}  // namespace comet