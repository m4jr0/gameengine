// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/render/driver/opengl/handler/opengl_shader_handler.h"
////////////////////////////////////////////////////////////////////////////////

// External. ///////////////////////////////////////////////////////////////////
#include <utility>
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/frame/frame_container.h"
#include "comet/core/memory/memory_utils.h"
#include "comet/core/type_trait.h"
#include "comet/geometry/type/mesh.h"
#include "comet/math/vector.h"
#include "comet/rendering/driver/opengl/opengl_debug.h"
#include "comet/rendering/driver/opengl/type/opengl_shader.h"
#include "comet/rendering/driver/opengl/utils/opengl_shader_utils.h"
#include "comet/rendering/label/pipeline_label.h"
#include "comet/rendering/label/shader_label.h"
#include "comet/rendering/type/common.h"
#include "comet/rendering/utils/shader_utils.h"
#include "comet/resource/resource_manager.h"

namespace comet {
namespace rendering {
namespace gl {
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
      material_handler_{descr.material_handler},
      texture_handler_{descr.texture_handler},
      sampler_handler_{descr.sampler_handler} {
  COMET_ASSERT(shader_module_handler_ != nullptr,
               "ShaderHandler::ShaderHandler", "shader module handler is null");
  COMET_ASSERT(material_handler_ != nullptr, "ShaderHandler::ShaderHandler",
               "material handler is null");
  COMET_ASSERT(texture_handler_ != nullptr, "ShaderHandler::ShaderHandler",
               "texture handler is null");
  COMET_ASSERT(sampler_handler_ != nullptr, "ShaderHandler::ShaderHandler",
               "sampler handler is null");
}

ShaderHandle ShaderHandler::GetOrGenerate(const ShaderDescr& descr) {
  return GetOrGenerate(descr.shader_resource_id, descr.bind_type);
}

ShaderHandle ShaderHandler::GetOrGenerate(
    resource::ShaderResourceId shader_resource_id, PipelineBindType bind_type) {
  COMET_ASSERT(shader_resource_id.IsValid(), "ShaderHandler::GetOrGenerate",
               "shader resource id is invalid");

  ShaderKey key{};
  key.shader_resource_id = shader_resource_id;
  key.bind_type = bind_type;

  if (const auto handle{shaders_.TryAcquire(key)}; handle) {
    return handle;
  }

  ShaderHandle generated_handle{ShaderHandle::Invalid()};
  auto* shader_resource_handler{resource::ResourceManager::Get().GetShaders()};

  const auto is_loaded{shader_resource_handler->WithTemporaryLoad(
      shader_resource_id,
      [this, &generated_handle, shader_resource_id,
       bind_type](const resource::ShaderResource* shader_resource) {
        ShaderDescr descr{};
        descr.shader_resource_id = shader_resource_id;
        descr.bind_type = bind_type;

        auto* shader{GenerateShader(descr, shader_resource)};
        COMET_ASSERT(shader != nullptr, "ShaderHandler::GetOrGenerate",
                     "generated shader is null", "shader_resource_id",
                     shader_resource_id);

        ShaderKey key{};
        key.shader_resource_id = shader_resource_id;
        key.bind_type = bind_type;

        generated_handle = shaders_.Create(key, shader);
        COMET_ASSERT(generated_handle, "ShaderHandler::GetOrGenerate",
                     "shader instance creation failed", "shader_resource_id",
                     shader_resource_id, "bind_type",
                     GetPipelineBindTypeLabel(bind_type), "bind_type_value",
                     ToUnderlying(bind_type));

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

void ShaderHandler::Bind(ShaderHandle handle) {
  const auto* shader{Get(handle)};

  const auto program_native_handle{shader->program_native_handle};
  COMET_ASSERT(program_native_handle != kInvalidGlNativeProgramHandle,
               "ShaderHandler::Bind", "shader program handle is invalid",
               "shader_handle", handle, "bind_type",
               GetPipelineBindTypeLabel(shader->bind_type), "bind_type_value",
               ToUnderlying(shader->bind_type));

  if (bound_program_native_handle_ != program_native_handle) {
    glUseProgram(program_native_handle);
    bound_program_native_handle_ = program_native_handle;
  }

  if (shader->bind_type == PipelineBindType::Graphics) {
    glPolygonMode(GL_FRONT_AND_BACK,
                  shader->rasterizer.is_wireframe ? GL_LINE : GL_FILL);

    const auto gl_cull_mode{GetGlCullMode(shader->rasterizer.cull_mode)};

    if (gl_cull_mode == GL_NONE) {
      glDisable(GL_CULL_FACE);
    } else {
      glEnable(GL_CULL_FACE);
      glCullFace(gl_cull_mode);
    }

    if (shader->depth_stencil.is_depth_test) {
      glEnable(GL_DEPTH_TEST);
      glDepthFunc(GetGlCompareOp(shader->depth_stencil.compare_op));
    } else {
      glDisable(GL_DEPTH_TEST);
    }

    glDepthMask(shader->depth_stencil.is_depth_write ? GL_TRUE : GL_FALSE);

    if (shader->rasterizer.is_depth_bias) {
      glEnable(GL_POLYGON_OFFSET_FILL);
      glPolygonOffset(1.0f, 1.0f);
    } else {
      glDisable(GL_POLYGON_OFFSET_FILL);
    }
  }

  for (const auto& binding : shader->bindings) {
    if (binding.set != shaderconsts::kGlobalSet) {
      continue;
    }

    if (binding.type != ShaderBindingType::UniformBuffer) {
      continue;
    }

    BindBufferBinding(binding, shader->uniform_buffer_native_handle,
                      binding.size,
                      shader->global_ubo_data.offset + binding.offset);
  }

  bound_shader_ = shader;
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
               "ShaderHandler::BindInstance",
               "materials are only supported on graphics shaders",
               "shader_handle", shader->handle);

  if (!internal::HasBindingInSet(shader, shaderconsts::kMaterialSet)) {
#ifdef COMET_DEBUG_RENDERING
    COMET_LOG_WARNING(LoggerType::Rendering, "ShaderHandler::BindInstance",
                      "shader has no material set", "shader_handle",
                      shader->handle);
#endif  // COMET_DEBUG_RENDERING
    return;
  }

  auto& instance{GetInstance(shader, material)};
  shader->bound_instance_ubo_offset =
      shader->global_ubo_data.stride + instance.offset;

  for (const auto& binding : shader->bindings) {
    if (binding.set != shaderconsts::kMaterialSet) {
      continue;
    }

    if (binding.type == ShaderBindingType::UniformBuffer) {
      BindBufferBinding(binding, shader->uniform_buffer_native_handle,
                        binding.size,
                        shader->bound_instance_ubo_offset + binding.offset);
      continue;
    }

    if (!IsImageBindingType(binding.type)) {
      continue;
    }

    auto* runtime_binding{
        FindBindingRuntimeData(instance.binding_data, binding.index)};

    if (runtime_binding == nullptr || runtime_binding->descriptors.IsEmpty()) {
      continue;
    }

    BindImageBinding(binding, runtime_binding->descriptors.GetData(),
                     static_cast<u32>(runtime_binding->descriptors.GetSize()));
  }
}

void ShaderHandler::BindVertexSource(ShaderHandle handle,
                                     const ShaderVertexSource& source) {
  auto* shader{Get(handle)};
  COMET_ASSERT(shader != nullptr, "ShaderHandler::BindVertexSource",
               "shader is null", "shader_handle", handle);
  COMET_ASSERT(shader->bind_type == PipelineBindType::Graphics,
               "ShaderHandler::BindVertexSource",
               "vertex source can only be bound on graphics shaders",
               "shader_handle", handle);

  COMET_ASSERT(shader->vertex_attribute_native_handle !=
                   kInvalidGlNativeVertexAttributeHandle,
               "ShaderHandler::BindVertexSource",
               "vertex array handle is invalid", "shader_handle", handle);

  COMET_ASSERT(
      source.vertex_buffer_native_handle != kInvalidGlNativeStorageHandle,
      "ShaderHandler::BindVertexSource", "vertex buffer handle is invalid",
      "shader_handle", handle);

  if (bound_vertex_attribute_native_handle_ !=
      shader->vertex_attribute_native_handle) {
    glBindVertexArray(shader->vertex_attribute_native_handle);
    bound_vertex_attribute_native_handle_ =
        shader->vertex_attribute_native_handle;
    bound_element_array_buffer_native_handle_ = kInvalidGlNativeStorageHandle;
  }

  if (shader->has_vertex_source_binding &&
      shader->vertex_source_id == source.vertex_source_id) {
    return;
  }

  if (bound_array_buffer_native_handle_ != source.vertex_buffer_native_handle) {
    glBindBuffer(GL_ARRAY_BUFFER, source.vertex_buffer_native_handle);
    bound_array_buffer_native_handle_ = source.vertex_buffer_native_handle;
  }

  if (source.has_index_buffer) {
    COMET_ASSERT(
        source.index_buffer_native_handle != kInvalidGlNativeStorageHandle,
        "ShaderHandler::BindVertexSource", "index buffer handle is invalid",
        "shader_handle", handle);

    if (bound_element_array_buffer_native_handle_ !=
        source.index_buffer_native_handle) {
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, source.index_buffer_native_handle);
      bound_element_array_buffer_native_handle_ =
          source.index_buffer_native_handle;
    }
  } else {
    if (bound_element_array_buffer_native_handle_ !=
        kInvalidGlNativeStorageHandle) {
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
      bound_element_array_buffer_native_handle_ = kInvalidGlNativeStorageHandle;
    }
  }

  for (const auto& attr : shader->vertex_attributes) {
    glEnableVertexAttribArray(attr.index);

    switch (attr.component_type) {
      case GL_BYTE:
      case GL_UNSIGNED_BYTE:
      case GL_SHORT:
      case GL_UNSIGNED_SHORT:
      case GL_INT:
      case GL_UNSIGNED_INT:
        glVertexAttribIPointer(attr.index, attr.component_count,
                               attr.component_type, attr.stride, attr.offset);
        break;

      default:
        glVertexAttribPointer(attr.index, attr.component_count,
                              attr.component_type, attr.is_normalized,
                              attr.stride, attr.offset);
        break;
    }
  }

  shader->has_vertex_source_binding = true;
  shader->vertex_source_id = source.vertex_source_id;
}

void ShaderHandler::UpdateGlobals(ShaderHandle handle,
                                  const ShaderGlobalUpdate& update) {
  auto* shader{Get(handle)};
  shader->bound_global_ubo_offset = shader->global_ubo_data.offset;

  if (update.field_updates != nullptr && !update.field_updates->IsEmpty()) {
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

      WriteBindingFieldToUbo(shader, shader->bound_global_ubo_offset, binding,
                             field_update.field_index, field_update.data,
                             field_update.size);
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

      UpdateBindingBuffer(binding, buffer_update.buffer_handle,
                          buffer_update.buffer_size,
                          buffer_update.buffer_offset);
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

      UpdateBindingImages(binding, image_update.descriptors,
                          image_update.descriptor_count);
    }
  }
}

void ShaderHandler::UpdatePass(ShaderHandle handle,
                               const ShaderPassUpdate& update) {
  auto* shader{Get(handle)};

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

      UpdateBindingBuffer(binding, buffer_update.buffer_handle,
                          buffer_update.buffer_size,
                          buffer_update.buffer_offset);
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

      UpdateBindingImages(binding, image_update.descriptors,
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
               "shader_handle", shader->handle);
  COMET_ASSERT(shader->bind_type == PipelineBindType::Graphics,
               "ShaderHandler::UpdateInstance",
               "materials are only supported on graphics shaders",
               "shader_handle", shader->handle);

  auto& instance{GetInstance(shader, material)};
  shader->bound_instance_ubo_offset =
      shader->global_ubo_data.stride + instance.offset;

  if (update.field_updates != nullptr && !update.field_updates->IsEmpty()) {
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

      WriteBindingFieldToUbo(shader, shader->bound_instance_ubo_offset, binding,
                             field_update.field_index, field_update.data,
                             field_update.size);
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

      UpdateBindingBuffer(binding, buffer_update.buffer_handle,
                          buffer_update.buffer_size,
                          buffer_update.buffer_offset);
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

      UpdateBindingImages(binding, image_update.descriptors,
                          image_update.descriptor_count);

      auto* runtime_binding{
          FindBindingRuntimeData(instance.binding_data, binding.index)};

      if (runtime_binding == nullptr) {
        ShaderBindingImageRuntimeData new_runtime_binding{};
        new_runtime_binding.binding_index = binding.index;
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

void ShaderHandler::PushConstants(ShaderHandle handle,
                                  const ShaderPushConstantsUpdate& update) {
  auto* shader{Get(handle)};

  if (update.blocks == nullptr || update.blocks->IsEmpty()) {
    return;
  }

  if (shader->uniform_buffer_native_handle ==
      kInvalidGlNativeUniformBufferHandle) {
    return;
  }

  const auto push_constant_base{GetPushConstantBaseOffset(shader)};

  glBindBuffer(GL_UNIFORM_BUFFER, shader->uniform_buffer_native_handle);

  for (const auto& block_update : *update.blocks) {
    COMET_ASSERT(
        block_update.block_index < shader->push_constant_blocks.GetSize(),
        "ShaderHandler::PushConstants", "push constant block index is invalid",
        "shader_handle", handle, "block_index", block_update.block_index,
        "block_count", shader->push_constant_blocks.GetSize());
    COMET_ASSERT(block_update.data != nullptr, "ShaderHandler::PushConstants",
                 "push constant block data is null", "shader_handle", handle,
                 "block_index", block_update.block_index);

    const auto& block{shader->push_constant_blocks[block_update.block_index]};
    COMET_ASSERT(block_update.size <= static_cast<usize>(block.size),
                 "ShaderHandler::PushConstants",
                 "push constant update exceeds block size", "shader_handle",
                 handle, "block_index", block_update.block_index, "update_size",
                 block_update.size, "block_size", block.size);

    const auto block_offset{push_constant_base + block.offset};

    glBufferSubData(GL_UNIFORM_BUFFER, block_offset,
                    static_cast<GLsizeiptr>(block_update.size),
                    block_update.data);

    glBindBufferRange(GL_UNIFORM_BUFFER,
                      shaderconsts::kPushConstantBindingOffset + block.index,
                      shader->uniform_buffer_native_handle, block_offset,
                      block.size);
  }

  glBindBuffer(GL_UNIFORM_BUFFER, kInvalidGlNativeUniformBufferHandle);
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

GLenum ShaderHandler::GetTopology(ShaderHandle handle) const {
  return Get(handle)->topology;
}

void ShaderHandler::Reset() {
  bound_shader_ = nullptr;
  bound_vertex_attribute_native_handle_ = kInvalidGlNativeVertexAttributeHandle;
  bound_array_buffer_native_handle_ = kInvalidGlNativeStorageHandle;
  bound_element_array_buffer_native_handle_ = kInvalidGlNativeStorageHandle;
  bound_program_native_handle_ = kInvalidGlNativeProgramHandle;
}

void ShaderHandler::OnInitialize() {
  general_allocator_.Initialize();
  shader_instance_allocator_.Initialize();
  shaders_.Initialize();

  material_handler_->SetDestroyCallback(
      [](const Material* material, void* user_data) {
        auto* shader_handler{static_cast<ShaderHandler*>(user_data)};
        shader_handler->UnbindMaterialFromAllShaders(material);
      },
      this);
}

void ShaderHandler::OnShutdown() {
  material_handler_->SetDestroyCallback(nullptr, nullptr);

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
                        key.shader_resource_id, "bind_type",
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
                 shader->id);

    DestroyShader(shader);
  }

  shaders_.Destroy();
  Reset();

  shader_instance_allocator_.Destroy();
  general_allocator_.Destroy();
}

Shader* ShaderHandler::Get(ShaderHandle handle) {
  auto* shader{shaders_.TryGet(handle)};
  COMET_ASSERT(shader != nullptr, "ShaderHandler::Get", "shader not found",
               "shader_handle", handle);
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
               shader->handle, "material_resource_id", material->id);

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

  shader->topology = GetGlPrimitiveTopology(shader_resource->descr.topology);
  shader->vertex_layout = shader_resource->descr.vertex_layout;

  shader->vertex_attributes = Array<VertexAttribute>{&general_allocator_};
  shader->bindings = Array<ShaderBinding>{&general_allocator_};
  shader->push_constant_blocks =
      Array<ShaderPushConstantBlock>{&general_allocator_};
  shader->module_handles = Array<ShaderModuleHandle>{&general_allocator_};

  shader->global_descriptor_data.binding_indices =
      Array<ShaderBindingIndex>{&general_allocator_};
  shader->storage_descriptor_data.binding_indices =
      Array<ShaderBindingIndex>{&general_allocator_};

  shader->instances.list = Array<MaterialInstance>{&shader_instance_allocator_};
  shader->instances.indices =
      Map<resource::MaterialResourceId, u32>{&general_allocator_};

  HandleShaderModulesGeneration(shader, shader_resource);
  HandleAttributesGeneration(shader, shader_resource);
  HandleBindingsGeneration(shader, shader_resource);
  HandlePushConstantBlocksGeneration(shader, shader_resource);
  HandleProgramGeneration(shader);
  HandleUboBufferGeneration(shader);

  AllocateGlobalDescriptorSets(shader);
  AllocateStorageDescriptorSets(shader);

  return shader;
}

void ShaderHandler::DestroyShader(Shader* shader) {
  COMET_ASSERT(shader != nullptr, "ShaderHandler::DestroyShader",
               "shader is null");

  for (auto& instance : shader->instances.list) {
    DestroyBindingRuntimeData(instance.binding_data);
    instance.descriptor_data.binding_indices.Release();
  }

  shader->instances.list.Release();
  shader->instances.indices.Release();

  shader->global_descriptor_data.binding_indices.Release();
  shader->storage_descriptor_data.binding_indices.Release();
  shader->global_descriptor_data.update_frame = kInvalidFrameCount;
  shader->storage_descriptor_data.update_frame = kInvalidFrameCount;

  shader->global_ubo_data = {};
  shader->instance_ubo_data = {};

  if (shader->uniform_buffer_native_handle !=
      kInvalidGlNativeUniformBufferHandle) {
    glDeleteBuffers(1, &shader->uniform_buffer_native_handle);
    shader->uniform_buffer_native_handle = kInvalidGlNativeUniformBufferHandle;
  }

  if (shader->vertex_attribute_native_handle !=
      kInvalidGlNativeVertexAttributeHandle) {
    glDeleteVertexArrays(1, &shader->vertex_attribute_native_handle);
    shader->vertex_attribute_native_handle =
        kInvalidGlNativeVertexAttributeHandle;
  }

  for (auto& binding : shader->bindings) {
    binding.fields.Release();
  }

  for (auto& block : shader->push_constant_blocks) {
    block.fields.Release();
  }

  shader->vertex_attributes.Release();
  shader->bindings.Release();
  shader->push_constant_blocks.Release();

  if (shader_module_handler_->IsInitialized()) {
    for (const auto module_handle : shader->module_handles) {
      shader_module_handler_->Destroy(module_handle);
    }
  }

  shader->module_handles.Release();

  if (shader->program_native_handle != kInvalidGlNativeProgramHandle) {
    glDeleteProgram(shader->program_native_handle);
    shader->program_native_handle = kInvalidGlNativeProgramHandle;
  }

  shader->handle.Invalidate();
  shader_instance_allocator_.Deallocate(shader);
}

void ShaderHandler::PopulateVertexAttributes(ShaderVertexLayout layout,
                                             Array<VertexAttribute>& attributes,
                                             VertexAttributeStride& stride) {
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
    Array<VertexAttribute>& attributes, VertexAttributeStride& stride) {
  attributes.Clear();
  attributes.Reserve(7);

  stride = static_cast<VertexAttributeStride>(sizeof(geometry::SkinnedVertex));

  attributes.PushLast(VertexAttribute{
      .index = 0,
      .component_count = 3,
      .component_type = GL_FLOAT,
      .is_normalized = GL_FALSE,
      .stride = stride,
      .offset = reinterpret_cast<const void*>(
          offsetof(geometry::SkinnedVertex, position)),
  });

  attributes.PushLast(VertexAttribute{
      .index = 1,
      .component_count = 3,
      .component_type = GL_FLOAT,
      .is_normalized = GL_FALSE,
      .stride = stride,
      .offset = reinterpret_cast<const void*>(
          offsetof(geometry::SkinnedVertex, normal)),
  });

  attributes.PushLast(VertexAttribute{
      .index = 2,
      .component_count = 4,
      .component_type = GL_FLOAT,
      .is_normalized = GL_FALSE,
      .stride = stride,
      .offset = reinterpret_cast<const void*>(
          offsetof(geometry::SkinnedVertex, tangent)),
  });

  attributes.PushLast(VertexAttribute{
      .index = 3,
      .component_count = 2,
      .component_type = GL_FLOAT,
      .is_normalized = GL_FALSE,
      .stride = stride,
      .offset =
          reinterpret_cast<const void*>(offsetof(geometry::SkinnedVertex, uv)),
  });

  attributes.PushLast(VertexAttribute{
      .index = 4,
      .component_count = 4,
      .component_type = GL_FLOAT,
      .is_normalized = GL_FALSE,
      .stride = stride,
      .offset = reinterpret_cast<const void*>(
          offsetof(geometry::SkinnedVertex, color)),
  });

  attributes.PushLast(VertexAttribute{
      .index = 5,
      .component_count = 4,
      .component_type = GL_UNSIGNED_SHORT,
      .is_normalized = GL_FALSE,
      .stride = stride,
      .offset = reinterpret_cast<const void*>(
          offsetof(geometry::SkinnedVertex, joint_indices)),
  });

  attributes.PushLast(VertexAttribute{
      .index = 6,
      .component_count = 4,
      .component_type = GL_FLOAT,
      .is_normalized = GL_FALSE,
      .stride = stride,
      .offset = reinterpret_cast<const void*>(
          offsetof(geometry::SkinnedVertex, joint_weights)),
  });
}

void ShaderHandler::PopulateDebugLineVertexAttributes(
    Array<VertexAttribute>& attributes, VertexAttributeStride& stride) {
  attributes.Clear();
  attributes.Reserve(2);

  stride = static_cast<VertexAttributeStride>(sizeof(GpuDebugLineVertex));

  attributes.PushLast(VertexAttribute{
      .index = 0,
      .component_count = 4,
      .component_type = GL_FLOAT,
      .is_normalized = GL_FALSE,
      .stride = stride,
      .offset =
          reinterpret_cast<const void*>(offsetof(GpuDebugLineVertex, position)),
  });

  attributes.PushLast(VertexAttribute{
      .index = 1,
      .component_count = 4,
      .component_type = GL_FLOAT,
      .is_normalized = GL_FALSE,
      .stride = stride,
      .offset =
          reinterpret_cast<const void*>(offsetof(GpuDebugLineVertex, color)),
  });
}

void ShaderHandler::BindBufferBinding(const ShaderBinding& binding,
                                      GLuint buffer_handle, usize buffer_size,
                                      usize buffer_offset) {
  if (buffer_handle == kInvalidStorageBufferHandle) {
    return;
  }

  if (buffer_size == 0) {
#ifdef COMET_DEBUG_RENDERING
    COMET_LOG_DEBUG(LoggerType::Rendering, "ShaderHandler::BindBufferBinding",
                    "skipping zero-sized buffer binding", "api_binding",
                    binding.api_binding, "binding_index", binding.index);
#endif  // COMET_DEBUG_RENDERING
    return;
  }

  const auto target{binding.type == ShaderBindingType::UniformBuffer
                        ? GL_UNIFORM_BUFFER
                        : GL_SHADER_STORAGE_BUFFER};

  glBindBufferRange(target, binding.api_binding, buffer_handle,
                    static_cast<GLintptr>(buffer_offset),
                    static_cast<GLsizeiptr>(buffer_size));
}

void ShaderHandler::BindImageBinding(const ShaderBinding& binding,
                                     const ShaderImageDescriptor* descriptors,
                                     u32 descriptor_count) const {
  COMET_ASSERT(descriptors != nullptr, "ShaderHandler::BindImageBinding",
               "image descriptors are null", "binding_set", binding.set,
               "binding_binding", binding.binding, "binding_index",
               binding.index);
  COMET_ASSERT(descriptor_count == binding.descriptor_count,
               "ShaderHandler::BindImageBinding", "descriptor count mismatch",
               "binding_set", binding.set, "binding_binding", binding.binding,
               "binding_index", binding.index, "expected_descriptor_count",
               binding.descriptor_count, "actual_descriptor_count",
               descriptor_count);

  for (u32 i{0}; i < descriptor_count; ++i) {
    const auto unit{binding.api_binding + i};
    const auto& descriptor{descriptors[i]};

    const auto* texture{descriptor.texture_handle
                            ? texture_handler_->Get(descriptor.texture_handle)
                            : nullptr};

    const auto* sampler{descriptor.sampler_handle
                            ? sampler_handler_->Get(descriptor.sampler_handle)
                            : nullptr};

    switch (binding.type) {
      case ShaderBindingType::CombinedImageSampler:
        COMET_ASSERT(texture != nullptr, "ShaderHandler::BindImageBinding",
                     "combined image sampler requires texture", "binding_set",
                     binding.set, "binding_binding", binding.binding,
                     "binding_index", binding.index, "descriptor_index", i);
        COMET_ASSERT(sampler != nullptr, "ShaderHandler::BindImageBinding",
                     "combined image sampler requires sampler", "binding_set",
                     binding.set, "binding_binding", binding.binding,
                     "binding_index", binding.index, "descriptor_index", i);

        glBindTextureUnit(unit, texture->native_handle);
        glBindSampler(unit, sampler->native_handle);
        break;

      case ShaderBindingType::SampledImage:
        COMET_ASSERT(texture != nullptr, "ShaderHandler::BindImageBinding",
                     "sampled image requires texture", "binding_set",
                     binding.set, "binding_binding", binding.binding,
                     "binding_index", binding.index, "descriptor_index", i);

        glBindTextureUnit(unit, texture->native_handle);
        break;

      case ShaderBindingType::Sampler:
        COMET_ASSERT(sampler != nullptr, "ShaderHandler::BindImageBinding",
                     "sampler binding requires sampler", "binding_set",
                     binding.set, "binding_binding", binding.binding,
                     "binding_index", binding.index, "descriptor_index", i);

        glBindSampler(unit, sampler->native_handle);
        break;

      case ShaderBindingType::StorageImage:
        COMET_ASSERT(texture != nullptr, "ShaderHandler::BindImageBinding",
                     "storage image requires texture", "binding_set",
                     binding.set, "binding_binding", binding.binding,
                     "binding_index", binding.index, "descriptor_index", i);

        glBindImageTexture(unit, texture->native_handle, 0, GL_FALSE, 0,
                           GetGlImageAccess(binding.type),
                           texture->internal_format);
        break;

      default:
        COMET_ASSERT(false, "ShaderHandler::BindImageBinding",
                     "unsupported image binding type", "binding_set",
                     binding.set, "binding_binding", binding.binding,
                     "binding_index", binding.index, "binding_type",
                     GetShaderBindingTypeLabel(binding.type),
                     "binding_type_value", ToUnderlying(binding.type));
        break;
    }
  }
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
               "material_resource_id", material->id);
  COMET_ASSERT(shader->bind_type == PipelineBindType::Graphics,
               "ShaderHandler::BindMaterial",
               "materials are only supported on graphics shaders",
               "shader_handle", shader->handle);
  COMET_ASSERT(shader->instances.list.GetSize() < kMaxMaterialInstances,
               "ShaderHandler::BindMaterial",
               "maximum material instance count reached", "shader_handle",
               shader->handle, "material_instance_count",
               shader->instances.list.GetSize(), "max_material_instances",
               kMaxMaterialInstances);

  if (!internal::HasBindingInSet(shader, shaderconsts::kMaterialSet)) {
    COMET_ASSERT(false, "ShaderHandler::BindMaterial",
                 "shader has no material set", "shader_handle", shader->handle);
    return;
  }

  MaterialInstance instance{};
  instance.material_resource_id = material->id;
  instance.descriptor_data.binding_indices =
      Array<ShaderBindingIndex>{&general_allocator_};
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
  UpdateInstanceUboBindings(shader, instance, instance_index);

  auto& resolved_image_descriptors{*COMET_FRAME_ARRAY(ShaderImageDescriptor)};

  for (const auto& binding : shader->bindings) {
    if (binding.set != shaderconsts::kMaterialSet) {
      continue;
    }

    if (!IsImageBindingType(binding.type)) {
      continue;
    }

    CollectMaterialImageDescriptors(material, binding,
                                    resolved_image_descriptors);

    ShaderBindingImageRuntimeData runtime_binding{};
    runtime_binding.binding_index = binding.index;
    runtime_binding.descriptors =
        Array<ShaderImageDescriptor>{&general_allocator_};
    runtime_binding.descriptors.PushFromRange(resolved_image_descriptors);
    instance.binding_data.image_bindings.EmplaceLast(
        std::move(runtime_binding));
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
               "material_resource_id", material->id);
  COMET_ASSERT(shader->bind_type == PipelineBindType::Graphics,
               "ShaderHandler::UnbindMaterial",
               "materials are only supported on graphics shaders",
               "shader_handle", shader->handle);

  auto* index_ptr{shader->instances.indices.TryGet(material->id)};
  COMET_ASSERT(index_ptr != nullptr, "ShaderHandler::UnbindMaterial",
               "material instance index is null", "shader_handle",
               shader->handle, "material_resource_id", material->id);

  const auto index{*index_ptr};
  auto& instances{shader->instances.list};
  auto& instance{instances[index]};

  DestroyBindingRuntimeData(instance.binding_data);

  const auto last_index{static_cast<u32>(instances.GetSize() - 1)};
  const auto removed_material_resource_id{material->id};

  if (index != last_index) {
    instances[index] = std::move(instances[last_index]);
    auto& moved_instance{instances[index]};
    shader->instances.indices.Set(moved_instance.material_resource_id, index);
    UpdateInstanceUboBindings(shader, moved_instance, index);
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
        shader_module_handler_->Generate(shader_module_resource_id)};
    COMET_ASSERT(module_handle, "ShaderHandler::HandleShaderModulesGeneration",
                 "shader module handle generation failed",
                 "shader_module_resource_id", shader_module_resource_id,
                 "shader_resource_id", shader->id);

    shader->module_handles.PushLast(module_handle);
  }
}

void ShaderHandler::HandleProgramGeneration(Shader* shader) const {
  [[maybe_unused]] bool has_graphics_stage{false};
  [[maybe_unused]] bool has_compute_stage{false};

  for (const auto module_handle : shader->module_handles) {
    const auto stage{shader_module_handler_->GetStage(module_handle)};

    if (stage == GL_COMPUTE_SHADER) {
      has_compute_stage = true;
    } else {
      has_graphics_stage = true;
    }
  }

  COMET_ASSERT(!(has_graphics_stage && has_compute_stage),
               "ShaderHandler::HandleProgramGeneration",
               "shader mixes graphics and compute stages", "shader_handle",
               shader->handle);

  switch (shader->bind_type) {
    case PipelineBindType::Graphics:
      COMET_ASSERT(has_graphics_stage, "ShaderHandler::HandleProgramGeneration",
                   "graphics shader has no graphics stages", "shader_handle",
                   shader->handle);
      COMET_ASSERT(!has_compute_stage, "ShaderHandler::HandleProgramGeneration",
                   "graphics shader contains compute stage", "shader_handle",
                   shader->handle);
      break;

    case PipelineBindType::Compute:
      COMET_ASSERT(has_compute_stage, "ShaderHandler::HandleProgramGeneration",
                   "compute shader has no compute stage", "shader_handle",
                   shader->handle);
      COMET_ASSERT(!has_graphics_stage,
                   "ShaderHandler::HandleProgramGeneration",
                   "compute shader contains graphics stage", "shader_handle",
                   shader->handle);
      break;

    default:
      COMET_ASSERT(false, "ShaderHandler::HandleProgramGeneration",
                   "unsupported shader bind type", "shader_handle",
                   shader->handle, "bind_type",
                   GetPipelineBindTypeLabel(shader->bind_type),
                   "bind_type_value", ToUnderlying(shader->bind_type));
      return;
  }

  shader->program_native_handle = glCreateProgram();
  COMET_ASSERT(shader->program_native_handle != kInvalidGlNativeProgramHandle,
               "ShaderHandler::HandleProgramGeneration",
               "program creation failed", "shader_handle", shader->handle,
               "bind_type", GetPipelineBindTypeLabel(shader->bind_type),
               "bind_type_value", ToUnderlying(shader->bind_type));

  HandleProgramCompilation(shader);
}

void ShaderHandler::HandleProgramCompilation(Shader* shader) const {
  const auto native_handle{shader->program_native_handle};
  COMET_ASSERT(native_handle != kInvalidGlNativeProgramHandle,
               "ShaderHandler::HandleProgramCompilation",
               "program handle is invalid", "shader_handle", shader->handle,
               "bind_type", GetPipelineBindTypeLabel(shader->bind_type),
               "bind_type_value", ToUnderlying(shader->bind_type));

  for (const auto module_handle : shader->module_handles) {
    if (shader_module_handler_->GetBindType(module_handle) ==
        shader->bind_type) {
      shader_module_handler_->Attach(shader, module_handle);
    }
  }

  glLinkProgram(native_handle);

#ifdef COMET_DEBUG
  GLint result{GL_FALSE};
  GLint info_log_len{0};
  glGetProgramiv(native_handle, GL_LINK_STATUS, &result);
  glGetProgramiv(native_handle, GL_INFO_LOG_LENGTH, &info_log_len);

  if (info_log_len > 0) {
    auto& error_message{*COMET_FRAME_ARRAY(GLchar)};
    error_message.Resize(info_log_len + 1);
    glGetProgramInfoLog(native_handle, info_log_len, nullptr,
                        error_message.GetData());

    if (result != GL_TRUE) {
      COMET_ASSERT(false, "ShaderHandler::HandleProgramCompilation",
                   "program link failed", "shader_handle", shader->handle,
                   "bind_type", GetPipelineBindTypeLabel(shader->bind_type),
                   "bind_type_value", ToUnderlying(shader->bind_type),
                   "info_log", error_message.GetData());
    }
  }
#endif  // COMET_DEBUG

  for (const auto module_handle : shader->module_handles) {
    if (shader_module_handler_->GetBindType(module_handle) ==
        shader->bind_type) {
      shader_module_handler_->Detach(shader, module_handle);
    }
  }
}

void ShaderHandler::HandleAttributesGeneration(
    Shader* shader, const resource::ShaderResource* resource) {
  COMET_ASSERT(shader != nullptr, "ShaderHandler::HandleAttributesGeneration",
               "shader is null");
  COMET_ASSERT(resource != nullptr, "ShaderHandler::HandleAttributesGeneration",
               "shader resource is null");

  PopulateVertexAttributes(resource->descr.vertex_layout,
                           shader->vertex_attributes,
                           shader->vertex_attribute_stride);

  if (shader->vertex_attributes.IsEmpty()) {
    return;
  }

  glGenVertexArrays(1, &shader->vertex_attribute_native_handle);
  glBindVertexArray(shader->vertex_attribute_native_handle);
  glBindVertexArray(kInvalidGlNativeVertexAttributeHandle);
}

void ShaderHandler::HandleBindingsGeneration(
    Shader* shader, const resource::ShaderResource* resource) {
  shader->bindings.Reserve(resource->descr.bindings.GetSize());

  for (const auto& binding_descr : resource->descr.bindings) {
    ShaderBinding binding{};
    binding.type = binding_descr.type;
    binding.scope = binding_descr.scope;
    binding.layout = binding_descr.layout;
    binding.stages = binding_descr.stages;
    binding.index = static_cast<ShaderBindingIndex>(shader->bindings.GetSize());
    binding.set = binding_descr.set;
    binding.binding = binding_descr.binding;
    binding.api_binding = ResolveBinding(binding.set, binding.binding);
    binding.descriptor_count = binding_descr.descriptor_count;
    binding.image_semantic = binding_descr.image_semantic;
    binding.fields = Array<ShaderField>{&general_allocator_};
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
      GLint align{0};
      glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &align);

      shader->global_ubo_data.size =
          AlignOffset(shader->global_ubo_data.size, align);
      binding.offset = shader->global_ubo_data.size;
      shader->global_ubo_data.size += binding.size;
      shader->global_ubo_data.stages |= binding.stages;
    }

    if (binding.scope == ShaderBindingScope::Material &&
        binding.type == ShaderBindingType::UniformBuffer) {
      GLint align{0};
      glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &align);

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
                   "binding_scope_value", ToUnderlying(binding.scope));
    }

    shader->bindings.PushLast(std::move(binding));
  }
}

void ShaderHandler::HandlePushConstantBlocksGeneration(
    Shader* shader, const resource::ShaderResource* resource) {
  shader->push_constant_blocks.Reserve(
      resource->descr.push_constants.GetSize());

  ShaderOffset push_constant_cursor{0};

  for (const auto& block_descr : resource->descr.push_constants) {
    ShaderPushConstantBlock block{};
    block.index = static_cast<ShaderPushConstantIndex>(
        shader->push_constant_blocks.GetSize());
    block.stages = block_descr.stages;
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

    shader->push_constant_blocks.PushLast(std::move(block));
  }
}

void ShaderHandler::HandleUboBufferGeneration(Shader* shader) const {
  GLint align{0};
  glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &align);

  shader->global_ubo_data.stride = static_cast<ShaderOffset>(memory::AlignSize(
      shader->global_ubo_data.size, static_cast<memory::Alignment>(align)));

  shader->instance_ubo_data.stride = static_cast<ShaderOffset>(
      memory::AlignSize(shader->instance_ubo_data.size,
                        static_cast<memory::Alignment>(align)));

#ifdef COMET_DEBUG
  GLint max_range{0};
  glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &max_range);

  COMET_ASSERT(shader->global_ubo_data.stride < max_range,
               "ShaderHandler::HandleUboBufferGeneration",
               "global ubo size exceeds limit", "shader_handle", shader->handle,
               "ubo_stride", shader->global_ubo_data.stride,
               "max_uniform_block_size", max_range);
  COMET_ASSERT(shader->instance_ubo_data.stride < max_range,
               "ShaderHandler::HandleUboBufferGeneration",
               "instance ubo size exceeds limit", "shader_handle",
               shader->handle, "ubo_stride", shader->instance_ubo_data.stride,
               "max_uniform_block_size", max_range);
#endif  // COMET_DEBUG

  ShaderOffset push_constant_total_size{0};

  for (const auto& block : shader->push_constant_blocks) {
    push_constant_total_size =
        math::Max(push_constant_total_size,
                  static_cast<ShaderOffset>(block.offset + block.size));
  }

  const auto buffer_size{static_cast<GLsizeiptr>(
      shader->global_ubo_data.stride +
      shader->instance_ubo_data.stride * kMaxMaterialInstances +
      push_constant_total_size)};

  if (buffer_size == 0) {
    return;
  }

  glGenBuffers(1, &shader->uniform_buffer_native_handle);
  glBindBuffer(GL_UNIFORM_BUFFER, shader->uniform_buffer_native_handle);
  COMET_GL_SET_UNIFORM_BUFFER_DEBUG_LABEL(shader->uniform_buffer_native_handle,
                                          "shader_uniform_buffer");
  glBufferData(GL_UNIFORM_BUFFER, buffer_size, nullptr, GL_DYNAMIC_DRAW);
  glBindBuffer(GL_UNIFORM_BUFFER, kInvalidGlNativeUniformBufferHandle);

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
  ComputeUniformBlockLayout(block);
}

void ShaderHandler::ComputeUniformBlockLayout(
    ShaderPushConstantBlock& block) const {
  ShaderOffset cursor{0};
  Alignment block_alignment{16};

  for (ShaderFieldIndex i{0}; i < block.fields.GetSize(); ++i) {
    auto& field{block.fields[i]};

    const auto info{GetFieldLayoutInfo(ShaderMemoryLayout::Std140, field.type,
                                       field.array_count)};

    cursor = AlignOffset(cursor, info.alignment);

    field.offset = cursor;
    field.element_size = info.element_size;
    field.aligned_size = info.aligned_size;
    field.stride = info.stride;
    field.size = info.total_size;

    cursor += field.size;
    block_alignment = math::Max(block_alignment, info.alignment);
  }

  block_alignment = math::Max<Alignment>(block_alignment, 16);
  block.size = AlignOffset(cursor, block_alignment);
}

void ShaderHandler::AllocateGlobalDescriptorSets(Shader* shader) {
  shader->global_descriptor_data.binding_indices.Clear();

  for (const auto& binding : shader->bindings) {
    if (binding.set == shaderconsts::kGlobalSet) {
      shader->global_descriptor_data.binding_indices.PushLast(binding.index);
    }
  }
}

void ShaderHandler::AllocateStorageDescriptorSets(Shader* shader) {
  shader->storage_descriptor_data.binding_indices.Clear();

  for (const auto& binding : shader->bindings) {
    if (binding.set == shaderconsts::kPassSet) {
      shader->storage_descriptor_data.binding_indices.PushLast(binding.index);
    }
  }
}

void ShaderHandler::AllocateMaterialDescriptorSets(Shader* shader,
                                                   MaterialInstance& instance) {
  instance.descriptor_data.binding_indices.Clear();

  for (const auto& binding : shader->bindings) {
    if (binding.set == shaderconsts::kMaterialSet) {
      instance.descriptor_data.binding_indices.PushLast(binding.index);
    }
  }
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

  descriptors.Clear();
  descriptors.Reserve(texture_maps.GetSize());

  for (const auto* texture_map : texture_maps) {
    ShaderImageDescriptor descriptor{};
    descriptor.texture_handle = texture_map->texture_handle;
    descriptor.sampler_handle = texture_map->sampler_handle;
    descriptors.PushLast(descriptor);
  }
}

void ShaderHandler::UpdateBindingBuffer(const ShaderBinding& binding,
                                        GLuint buffer_handle, usize buffer_size,
                                        usize buffer_offset) const {
  BindBufferBinding(binding, buffer_handle, buffer_size, buffer_offset);
}

void ShaderHandler::UpdateBindingImages(
    const ShaderBinding& binding, const ShaderImageDescriptor* descriptors,
    u32 descriptor_count) const {
  BindImageBinding(binding, descriptors, descriptor_count);
}

void ShaderHandler::UpdateInstanceUboBindings(Shader* shader,
                                              MaterialInstance& instance,
                                              u32 instance_index) {
  COMET_ASSERT(shader != nullptr, "ShaderHandler::UpdateInstanceUboBindings",
               "shader is null");

  instance.offset = shader->instance_ubo_data.stride * instance_index;

  for (const auto& binding : shader->bindings) {
    if (binding.set != shaderconsts::kMaterialSet) {
      continue;
    }

    if (binding.type != ShaderBindingType::UniformBuffer) {
      continue;
    }

    BindBufferBinding(
        binding, shader->uniform_buffer_native_handle, binding.size,
        shader->global_ubo_data.stride + instance.offset + binding.offset);
  }
}

void ShaderHandler::WriteBindingFieldToUbo(
    Shader* shader, ShaderOffset base_offset, const ShaderBinding& binding,
    ShaderFieldIndex field_index, const void* value, usize value_size) const {
  COMET_ASSERT(shader != nullptr, "ShaderHandler::WriteBindingFieldToUbo",
               "shader is null");
  COMET_ASSERT(field_index < binding.fields.GetSize(),
               "ShaderHandler::WriteBindingFieldToUbo",
               "field index is invalid", "binding_index", binding.index,
               "field_index", field_index, "field_count",
               binding.fields.GetSize());
  COMET_ASSERT(value != nullptr, "ShaderHandler::WriteBindingFieldToUbo",
               "field value is null", "binding_index", binding.index,
               "field_index", field_index);
  COMET_ASSERT(shader->uniform_buffer_native_handle !=
                   kInvalidGlNativeUniformBufferHandle,
               "ShaderHandler::WriteBindingFieldToUbo",
               "uniform buffer handle is invalid", "shader_handle",
               shader->handle);

  const auto& field{binding.fields[field_index]};
  const auto raw_size{static_cast<usize>(field.element_size)};
  const auto copy_size{value_size == 0 ? raw_size : value_size};

  COMET_ASSERT(copy_size <= field.size, "ShaderHandler::WriteBindingFieldToUbo",
               "field update exceeds field size", "binding_index",
               binding.index, "field_index", field_index, "copy_size",
               copy_size, "field_size", field.size);

  glBindBuffer(GL_UNIFORM_BUFFER, shader->uniform_buffer_native_handle);
  glBufferSubData(GL_UNIFORM_BUFFER,
                  base_offset + binding.offset + field.offset,
                  static_cast<GLsizeiptr>(copy_size), value);
  glBindBuffer(GL_UNIFORM_BUFFER, kInvalidGlNativeUniformBufferHandle);
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

ShaderOffset ShaderHandler::GetPushConstantBaseOffset(
    const Shader* shader) const {
  return shader->global_ubo_data.stride +
         shader->instance_ubo_data.stride * kMaxMaterialInstances;
}

const MaterialHandler* ShaderHandler::GetMaterialHandler() const {
  return material_handler_;
}
}  // namespace gl
}  // namespace rendering
}  // namespace comet
