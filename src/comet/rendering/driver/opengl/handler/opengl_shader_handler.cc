// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "opengl_shader_handler.h"
////////////////////////////////////////////////////////////////////////////////

// External. ///////////////////////////////////////////////////////////////////
#include <type_traits>
#include <utility>
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/frame/frame_utils.h"
#include "comet/core/memory/memory_utils.h"
#include "comet/geometry/geometry_common.h"
#include "comet/math/vector.h"
#include "comet/rendering/driver/opengl/data/opengl_shader_data.h"
#include "comet/rendering/driver/opengl/opengl_debug.h"
#include "comet/rendering/driver/opengl/utils/opengl_common_utils.h"
#include "comet/rendering/driver/opengl/utils/opengl_shader_utils.h"
#include "comet/resource/resource_manager.h"

namespace comet {
namespace rendering {
namespace gl {
namespace internal {
bool HasBindingInSet(const Shader* shader, u32 set) {
  for (auto& binding : shader->bindings) {
    if (binding.set == set) {
      return true;
    }
  }

  return false;
}

bool IsImageBindingType(ShaderBindingType type) {
  switch (type) {
    case ShaderBindingType::CombinedImageSampler:
    case ShaderBindingType::SampledImage:
    case ShaderBindingType::Sampler:
    case ShaderBindingType::StorageImage:
      return true;
    default:
      return false;
  }
}

bool IsBufferBindingType(ShaderBindingType type) {
  switch (type) {
    case ShaderBindingType::UniformBuffer:
    case ShaderBindingType::StorageBuffer:
      return true;
    default:
      return false;
  }
}

GLenum GetGlImageAccess(ShaderBindingType type) {
  switch (type) {
    case ShaderBindingType::StorageImage:
      return GL_READ_WRITE;
    default:
      return GL_READ_ONLY;
  }
}
}  // namespace internal

ShaderHandler::ShaderHandler(const ShaderHandlerDescr& descr)
    : Handler{descr},
      shader_module_handler_{descr.shader_module_handler},
      material_handler_{descr.material_handler},
      texture_handler_{descr.texture_handler} {
  COMET_ASSERT(shader_module_handler_ != nullptr,
               "Shader module handler is null!");
  COMET_ASSERT(material_handler_ != nullptr, "Material handler is null!");
  COMET_ASSERT(texture_handler_ != nullptr, "Texture handler is null!");
}

void ShaderHandler::Initialize() {
  Handler::Initialize();
  general_allocator_.Initialize();
  shader_instance_allocator_.Initialize();

  shaders_ = Array<Shader*>{&shader_instance_allocator_};
  shader_ids_ = Map<resource::ResourceId, ShaderHandle>{&general_allocator_};

  material_handler_->SetDestroyCallback(
      [](Material* material, void* user_data) {
        auto* shader_handler{static_cast<ShaderHandler*>(user_data)};
        shader_handler->UnbindMaterialFromAllShaders(material);
      },
      this);
}

void ShaderHandler::Shutdown() {
  material_handler_->SetDestroyCallback(nullptr, nullptr);

  for (auto* shader : shaders_) {
    if (shader != nullptr) {
      Destroy(shader, true);
    }
  }

  shader_ids_.Destroy();
  shaders_.Destroy();

  bound_shader_ = nullptr;
  bound_vertex_attribute_handle_ = kInvalidVertexAttributeHandle;

  bound_array_buffer_handle_ = kInvalidStorageHandle;
  bound_element_array_buffer_handle_ = kInvalidStorageHandle;
  bound_program_handle_ = kInvalidShaderHandle;

  shader_instance_allocator_.Destroy();
  general_allocator_.Destroy();
  Handler::Shutdown();
}

Shader* ShaderHandler::Get(ShaderHandle shader_handle) {
  COMET_ASSERT(shader_handle_handler_.IsAlive(shader_handle),
               "Shader handle is invalid!");
  auto* shader{TryGet(shader_handle)};
  COMET_ASSERT(shader != nullptr, "Requested shader does not exist!");
  return shader;
}

const Shader* ShaderHandler::Get(ShaderHandle shader_handle) const {
  COMET_ASSERT(shader_handle_handler_.IsAlive(shader_handle),
               "Shader handle is invalid!");
  auto* shader{TryGet(shader_handle)};
  COMET_ASSERT(shader != nullptr, "Requested shader does not exist!");
  return shader;
}

Shader* ShaderHandler::TryGet(ShaderHandle shader_handle) {
  if (!shader_handle_handler_.IsAlive(shader_handle)) {
    return nullptr;
  }

  return shaders_[gid::GetIndex(shader_handle)];
}

const Shader* ShaderHandler::TryGet(ShaderHandle shader_handle) const {
  if (!shader_handle_handler_.IsAlive(shader_handle)) {
    return nullptr;
  }

  return shaders_[gid::GetIndex(shader_handle)];
}

Shader* ShaderHandler::GetOrGenerate(const ShaderDescr& descr) {
  return GetOrGenerate(descr.shader_id);
}

Shader* ShaderHandler::GetOrGenerate(resource::ResourceId shader_id) {
  COMET_ASSERT(shader_id != resource::kInvalidResourceId,
               "Shader resource id is invalid!");

  if (auto* shader_handle{shader_ids_.TryGet(shader_id)};
      shader_handle != nullptr) {
    auto* shader{Get(*shader_handle)};
    ++shader->ref_count;
    return shader;
  }

  auto* shader_resource{
      resource::ResourceManager::Get().GetShaders()->Load(shader_id)};
  COMET_ASSERT(shader_resource != nullptr, "Shader resource is null!");

  auto* shader{Generate(shader_resource)};
  shader_ids_.Emplace(shader_id, shader->handle);
  return shader;
}

void ShaderHandler::UnbindMaterialFromAllShaders(Material* material) {
  COMET_ASSERT(material != nullptr, "Material is null!");

  for (auto* shader : shaders_) {
    if (shader == nullptr) {
      continue;
    }

    if (!HasMaterial(shader, material)) {
      continue;
    }

    UnbindMaterial(shader, material);
  }
}

void ShaderHandler::Destroy(ShaderHandle shader_handle) {
  Destroy(Get(shader_handle));
}

void ShaderHandler::Destroy(Shader* shader) { Destroy(shader, false); }

ShaderBindingIndex ShaderHandler::GetBindingIndex(const Shader* shader, u32 set,
                                                  u32 binding) const {
  COMET_ASSERT(shader != nullptr, "Shader is null!");

  for (ShaderBindingIndex i{0}; i < shader->bindings.GetSize(); ++i) {
    auto& shader_binding{shader->bindings[i]};

    if (shader_binding.set == set && shader_binding.binding == binding) {
      return i;
    }
  }

  COMET_ASSERT(false, "Unable to find shader binding in set ", set,
               ", binding ", binding, "!");
  return 0;
}

void ShaderHandler::Bind(const Shader* shader, ShaderBindType bind_type) const {
  COMET_ASSERT(shader != nullptr, "Shader is null!");

  auto program{ResolveHandle(shader, bind_type)};
  COMET_ASSERT(program != kInvalidShaderHandle, "Invalid shader program!");

  if (bound_program_handle_ != program) {
    glUseProgram(program);
    bound_program_handle_ = program;
  }

  if (bind_type == ShaderBindType::Graphics) {
    glPolygonMode(GL_FRONT_AND_BACK,
                  shader->rasterizer.is_wireframe ? GL_LINE : GL_FILL);

    auto gl_cull_mode{GetGlCullMode(shader->rasterizer.cull_mode)};
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

  for (auto& binding : shader->bindings) {
    if (binding.set != shaderconsts::kGlobalSet) {
      continue;
    }

    if (binding.type != ShaderBindingType::UniformBuffer) {
      continue;
    }

    BindBufferBinding(binding, shader->uniform_buffer_handle, binding.size,
                      shader->global_ubo_data.offset + binding.offset);
  }

  bound_shader_ = shader;
}

void ShaderHandler::BindInstance(Shader* shader, MaterialId material_id) {
  BindInstance(shader, material_handler_->Get(material_id));
}

void ShaderHandler::BindInstance(Shader* shader, const Material* material) {
  COMET_ASSERT(shader != nullptr, "Shader is null!");
  COMET_ASSERT(material != nullptr, "Material is null!");

  if (!internal::HasBindingInSet(shader, shaderconsts::kMaterialSet)) {
    return;
  }

  auto& instance{GetInstance(shader, material)};
  shader->bound_instance_ubo_offset =
      shader->global_ubo_data.stride + instance.offset;

  for (auto& binding : shader->bindings) {
    if (binding.set != shaderconsts::kMaterialSet) {
      continue;
    }

    if (binding.type == ShaderBindingType::UniformBuffer) {
      BindBufferBinding(binding, shader->uniform_buffer_handle, binding.size,
                        shader->bound_instance_ubo_offset + binding.offset);
      continue;
    }

    if (!internal::IsImageBindingType(binding.type)) {
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

void ShaderHandler::BindVertexSource(Shader* shader,
                                     const ShaderVertexSource& source) const {
  COMET_ASSERT(shader != nullptr, "Shader is null!");
  COMET_ASSERT(shader->vertex_attribute_handle != kInvalidVertexAttributeHandle,
               "Vertex attribute handle is invalid!");
  COMET_ASSERT(source.vertex_buffer_handle != kInvalidStorageHandle,
               "Vertex buffer handle is invalid!");

  if (bound_vertex_attribute_handle_ != shader->vertex_attribute_handle) {
    glBindVertexArray(shader->vertex_attribute_handle);
    bound_vertex_attribute_handle_ = shader->vertex_attribute_handle;
    bound_element_array_buffer_handle_ = kInvalidStorageHandle;
  }

  if (shader->has_vertex_source_binding &&
      shader->vertex_source_id == source.vertex_source_id) {
    return;
  }

  if (bound_array_buffer_handle_ != source.vertex_buffer_handle) {
    glBindBuffer(GL_ARRAY_BUFFER, source.vertex_buffer_handle);
    bound_array_buffer_handle_ = source.vertex_buffer_handle;
  }

  if (source.has_index_buffer) {
    COMET_ASSERT(source.index_buffer_handle != kInvalidStorageHandle,
                 "Index buffer handle is invalid!");

    if (bound_element_array_buffer_handle_ != source.index_buffer_handle) {
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, source.index_buffer_handle);
      bound_element_array_buffer_handle_ = source.index_buffer_handle;
    }
  } else {
    if (bound_element_array_buffer_handle_ != kInvalidStorageHandle) {
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
      bound_element_array_buffer_handle_ = kInvalidStorageHandle;
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

void ShaderHandler::Reset() {
  bound_shader_ = nullptr;
  bound_vertex_attribute_handle_ = kInvalidVertexAttributeHandle;

  bound_array_buffer_handle_ = kInvalidStorageHandle;
  bound_element_array_buffer_handle_ = kInvalidStorageHandle;
  bound_program_handle_ = kInvalidShaderHandle;
}

void ShaderHandler::UpdatePass(Shader* shader,
                               const ShaderPassUpdate& update) const {
  COMET_ASSERT(shader != nullptr, "Shader is null!");

  if (update.buffer_bindings != nullptr && !update.buffer_bindings->IsEmpty()) {
    for (auto& buffer_update : *update.buffer_bindings) {
      COMET_ASSERT(buffer_update.binding_index < shader->bindings.GetSize(),
                   "Invalid binding index in pass buffer update!");

      auto& binding{shader->bindings[buffer_update.binding_index]};

      COMET_ASSERT(binding.scope == ShaderBindingScope::Pass,
                   "Pass buffer update targets non-pass binding!");
      COMET_ASSERT(binding.type == ShaderBindingType::UniformBuffer ||
                       binding.type == ShaderBindingType::StorageBuffer,
                   "Pass buffer update targets non-buffer binding!");

      UpdateBindingBuffer(binding, buffer_update.buffer_handle,
                          buffer_update.buffer_size,
                          buffer_update.buffer_offset);
    }
  }

  if (update.image_bindings != nullptr && !update.image_bindings->IsEmpty()) {
    for (auto& image_update : *update.image_bindings) {
      COMET_ASSERT(image_update.binding_index < shader->bindings.GetSize(),
                   "Invalid binding index in pass image update!");

      auto& binding{shader->bindings[image_update.binding_index]};

      COMET_ASSERT(binding.scope == ShaderBindingScope::Pass,
                   "Pass image update targets non-pass binding!");
      COMET_ASSERT(internal::IsImageBindingType(binding.type),
                   "Pass image update targets non-image binding!");
      COMET_ASSERT(image_update.descriptors != nullptr,
                   "Pass image update descriptors are null!");
      COMET_ASSERT(image_update.descriptor_count > 0,
                   "Pass image update descriptor count is 0!");

      UpdateBindingImages(binding, image_update.descriptors,
                          image_update.descriptor_count);
    }
  }
}

void ShaderHandler::UpdateGlobals(Shader* shader,
                                  const ShaderGlobalUpdate& update) const {
  COMET_ASSERT(shader != nullptr, "Shader is null!");
  shader->bound_global_ubo_offset = shader->global_ubo_data.offset;

  if (update.field_updates != nullptr && !update.field_updates->IsEmpty()) {
    for (auto& field_update : *update.field_updates) {
      COMET_ASSERT(field_update.binding_index < shader->bindings.GetSize(),
                   "Invalid binding index in global field update!");

      auto& binding{shader->bindings[field_update.binding_index]};

      COMET_ASSERT(binding.scope == ShaderBindingScope::Global,
                   "Global field update targets non-global binding!");
      COMET_ASSERT(binding.type == ShaderBindingType::UniformBuffer,
                   "Global field update targets non-UBO binding!");

      WriteBindingFieldToUbo(shader, shader->bound_global_ubo_offset, binding,
                             field_update.field_index, field_update.data,
                             field_update.size);
    }
  }

  if (update.buffer_bindings != nullptr && !update.buffer_bindings->IsEmpty()) {
    for (auto& buffer_update : *update.buffer_bindings) {
      COMET_ASSERT(buffer_update.binding_index < shader->bindings.GetSize(),
                   "Invalid binding index in global buffer update!");

      auto& binding{shader->bindings[buffer_update.binding_index]};

      COMET_ASSERT(binding.scope == ShaderBindingScope::Global,
                   "Global buffer update targets non-global binding!");
      COMET_ASSERT(binding.type == ShaderBindingType::UniformBuffer ||
                       binding.type == ShaderBindingType::StorageBuffer,
                   "Global buffer update targets non-buffer binding!");

      UpdateBindingBuffer(binding, buffer_update.buffer_handle,
                          buffer_update.buffer_size,
                          buffer_update.buffer_offset);
    }
  }

  if (update.image_bindings != nullptr && !update.image_bindings->IsEmpty()) {
    for (auto& image_update : *update.image_bindings) {
      COMET_ASSERT(image_update.binding_index < shader->bindings.GetSize(),
                   "Invalid binding index in global image update!");

      auto& binding{shader->bindings[image_update.binding_index]};

      COMET_ASSERT(binding.scope == ShaderBindingScope::Global,
                   "Global image update targets non-global binding!");
      COMET_ASSERT(internal::IsImageBindingType(binding.type),
                   "Global image update targets non-image binding!");
      COMET_ASSERT(image_update.descriptors != nullptr,
                   "Global image update descriptors are null!");
      COMET_ASSERT(image_update.descriptor_count > 0,
                   "Global image update descriptor count is 0!");

      UpdateBindingImages(binding, image_update.descriptors,
                          image_update.descriptor_count);
    }
  }
}

void ShaderHandler::UpdateInstance(Shader* shader, MaterialId material_id,
                                   const ShaderInstanceUpdate& update) {
  UpdateInstance(shader, material_handler_->Get(material_id), update);
}

void ShaderHandler::UpdateInstance(Shader* shader, Material* material,
                                   const ShaderInstanceUpdate& update) {
  COMET_ASSERT(shader != nullptr, "Shader is null!");
  COMET_ASSERT(material != nullptr, "Material is null!");

  auto& instance{GetInstance(shader, material)};
  shader->bound_instance_ubo_offset =
      shader->global_ubo_data.stride + instance.offset;

  if (update.field_updates != nullptr && !update.field_updates->IsEmpty()) {
    for (auto& field_update : *update.field_updates) {
      COMET_ASSERT(field_update.binding_index < shader->bindings.GetSize(),
                   "Invalid binding index in instance field update!");

      auto& binding{shader->bindings[field_update.binding_index]};

      COMET_ASSERT(binding.scope == ShaderBindingScope::Material,
                   "Instance field update targets non-material binding!");
      COMET_ASSERT(binding.type == ShaderBindingType::UniformBuffer,
                   "Instance field update targets non-UBO binding!");

      WriteBindingFieldToUbo(shader, shader->bound_instance_ubo_offset, binding,
                             field_update.field_index, field_update.data,
                             field_update.size);
    }
  }

  if (update.buffer_bindings != nullptr && !update.buffer_bindings->IsEmpty()) {
    for (auto& buffer_update : *update.buffer_bindings) {
      COMET_ASSERT(buffer_update.binding_index < shader->bindings.GetSize(),
                   "Invalid binding index in instance buffer update!");

      auto& binding{shader->bindings[buffer_update.binding_index]};

      COMET_ASSERT(binding.scope == ShaderBindingScope::Material,
                   "Instance buffer update targets non-material binding!");
      COMET_ASSERT(binding.type == ShaderBindingType::UniformBuffer ||
                       binding.type == ShaderBindingType::StorageBuffer,
                   "Instance buffer update targets non-buffer binding!");

      UpdateBindingBuffer(binding, buffer_update.buffer_handle,
                          buffer_update.buffer_size,
                          buffer_update.buffer_offset);
    }
  }

  if (update.image_bindings != nullptr && !update.image_bindings->IsEmpty()) {
    for (auto& image_update : *update.image_bindings) {
      COMET_ASSERT(image_update.binding_index < shader->bindings.GetSize(),
                   "Invalid binding index in instance image update!");

      auto& binding{shader->bindings[image_update.binding_index]};

      COMET_ASSERT(binding.scope == ShaderBindingScope::Material,
                   "Instance image update targets non-material binding!");
      COMET_ASSERT(internal::IsImageBindingType(binding.type),
                   "Instance image update targets non-image binding!");
      COMET_ASSERT(image_update.descriptors != nullptr,
                   "Instance image update descriptors are null!");
      COMET_ASSERT(image_update.descriptor_count > 0,
                   "Instance image update descriptor count is 0!");

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
        instance.binding_data.image_bindings.EmplaceBack(
            std::move(new_runtime_binding));
      } else {
        runtime_binding->descriptors.Clear();
        runtime_binding->descriptors.PushFromRange(
            image_update.descriptors, image_update.descriptor_count);
      }
    }
  }
}

void ShaderHandler::PushConstants(
    Shader* shader, const ShaderPushConstantsUpdate& update) const {
  COMET_ASSERT(shader != nullptr, "Shader is null!");

  if (update.blocks == nullptr || update.blocks->IsEmpty()) {
    return;
  }

  if (shader->uniform_buffer_handle == kInvalidUniformBufferHandle) {
    return;
  }

  auto push_constant_base{GetPushConstantBaseOffset(shader)};

  glBindBuffer(GL_UNIFORM_BUFFER, shader->uniform_buffer_handle);

  for (auto& block_update : *update.blocks) {
    COMET_ASSERT(
        block_update.block_index < shader->push_constant_blocks.GetSize(),
        "Invalid push constant block index!");
    COMET_ASSERT(block_update.data != nullptr,
                 "Push constant block data is null!");

    auto& block{shader->push_constant_blocks[block_update.block_index]};
    COMET_ASSERT(block_update.size <= static_cast<usize>(block.size),
                 "Push constant update exceeds block size!");

    auto block_offset{push_constant_base + block.offset};

    glBufferSubData(GL_UNIFORM_BUFFER, block_offset,
                    static_cast<GLsizeiptr>(block_update.size),
                    block_update.data);

    glBindBufferRange(GL_UNIFORM_BUFFER,
                      shaderconsts::kPushConstantBindingOffset + block.index,
                      shader->uniform_buffer_handle, block_offset, block.size);
  }

  glBindBuffer(GL_UNIFORM_BUFFER, kInvalidUniformBufferHandle);
}

void ShaderHandler::BindMaterial(Shader* shader, Material* material) {
  COMET_ASSERT(shader != nullptr, "Shader is null!");
  COMET_ASSERT(material != nullptr, "Material is null!");
  COMET_ASSERT(!HasMaterial(shader, material),
               "Material is already bound to this shader!");

  if (!internal::HasBindingInSet(shader, shaderconsts::kMaterialSet)) {
    return;
  }

  MaterialInstance instance{};
  instance.material_id = material->id;
  instance.descriptor_data.binding_indices =
      Array<ShaderBindingIndex>{&general_allocator_};
  instance.binding_data.image_bindings =
      Array<ShaderBindingImageRuntimeData>{&general_allocator_};

  u32 material_image_binding_count{0};

  for (auto& binding : shader->bindings) {
    if (binding.set == shaderconsts::kMaterialSet &&
        internal::IsImageBindingType(binding.type)) {
      ++material_image_binding_count;
    }
  }

  instance.binding_data.image_bindings.Reserve(material_image_binding_count);

  auto instance_index{static_cast<u32>(shader->instances.list.GetSize())};

  AllocateMaterialDescriptorSets(shader, instance);
  UpdateInstanceUboBindings(shader, instance, instance_index);

  auto& resolved_image_descriptors{*COMET_FRAME_ARRAY(ShaderImageDescriptor)};

  for (auto& binding : shader->bindings) {
    if (binding.set != shaderconsts::kMaterialSet) {
      continue;
    }

    if (!internal::IsImageBindingType(binding.type)) {
      continue;
    }

    CollectMaterialImageDescriptors(material, binding,
                                    resolved_image_descriptors);

    ShaderBindingImageRuntimeData runtime_binding{};
    runtime_binding.binding_index = binding.index;
    runtime_binding.descriptors =
        Array<ShaderImageDescriptor>{&general_allocator_};
    runtime_binding.descriptors.PushFromRange(resolved_image_descriptors);
    instance.binding_data.image_bindings.EmplaceBack(
        std::move(runtime_binding));
  }

  shader->instances.indices.Emplace(material->id, instance_index);
  shader->instances.list.EmplaceBack(std::move(instance));
}

void ShaderHandler::UnbindMaterial(Shader* shader, Material* material) {
  COMET_ASSERT(shader != nullptr, "Shader is null!");
  COMET_ASSERT(material != nullptr, "Material is null!");
  COMET_ASSERT(HasMaterial(shader, material),
               "Trying to unbind a dead material instance!");

  auto* index_ptr{shader->instances.indices.TryGet(material->id)};
  COMET_ASSERT(index_ptr != nullptr, "Material instance index is null!");

  auto index{*index_ptr};
  auto& instances{shader->instances.list};
  auto& instance{instances[index]};

  DestroyBindingRuntimeData(instance.binding_data);

  auto last_index{static_cast<u32>(instances.GetSize() - 1)};
  auto removed_material_id{material->id};

  if (index != last_index) {
    instances[index] = std::move(instances[last_index]);
    auto& moved_instance{instances[index]};
    shader->instances.indices[moved_instance.material_id] = index;
    UpdateInstanceUboBindings(shader, moved_instance, index);
  }

  instances.Resize(last_index);
  shader->instances.indices.Remove(removed_material_id);
}

bool ShaderHandler::HasMaterial(const Shader* shader,
                                const Material* material) const {
  COMET_ASSERT(shader != nullptr, "Shader is null!");
  COMET_ASSERT(material != nullptr, "Material is null!");
  return shader->instances.indices.TryGet(material->id) != nullptr;
}

MaterialInstance& ShaderHandler::GetInstance(Shader* shader,
                                             const Material* material) {
  COMET_ASSERT(shader != nullptr, "Shader is null!");
  COMET_ASSERT(material != nullptr, "Material is null!");

  auto* index{shader->instances.indices.TryGet(material->id)};
  COMET_ASSERT(index != nullptr, "Material is not bound to this shader!");

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

GLenum ShaderHandler::GetGlCullMode(CullMode cull_mode) {
  switch (cull_mode) {
    case CullMode::None:
      return GL_NONE;
    case CullMode::Front:
      return GL_FRONT;
    case CullMode::Back:
      return GL_BACK;
    case CullMode::FrontAndBack:
      return GL_FRONT_AND_BACK;
    default:
      COMET_ASSERT(false, "Unsupported cull mode!");
      return 0;
  }
}

GLenum ShaderHandler::GetGlPrimitiveTopology(PrimitiveTopology topology) {
  switch (topology) {
    case PrimitiveTopology::Points:
      return GL_POINTS;
    case PrimitiveTopology::Lines:
      return GL_LINES;
    case PrimitiveTopology::LineStrip:
      return GL_LINE_STRIP;
    case PrimitiveTopology::Triangles:
      return GL_TRIANGLES;
    case PrimitiveTopology::TriangleStrip:
      return GL_TRIANGLE_STRIP;
    default:
      COMET_ASSERT(false, "Unsupported primitive topology!");
      return 0;
  }
}

GLenum ShaderHandler::GetGlCompareOp(CompareOp compare_op) {
  switch (compare_op) {
    case CompareOp::Never:
      return GL_NEVER;
    case CompareOp::Less:
      return GL_LESS;
    case CompareOp::Equal:
      return GL_EQUAL;
    case CompareOp::LessOrEqual:
      return GL_LEQUAL;
    case CompareOp::Greater:
      return GL_GREATER;
    case CompareOp::NotEqual:
      return GL_NOTEQUAL;
    case CompareOp::GreaterOrEqual:
      return GL_GEQUAL;
    case CompareOp::Always:
      return GL_ALWAYS;
    default:
      COMET_ASSERT(false, "Unsupported compare op!");
      return 0;
  }
}

Alignment ShaderHandler::GetBindingFieldAlignment(ShaderMemoryLayout layout,
                                                  ShaderVariableType type) {
  switch (layout) {
    case ShaderMemoryLayout::Std140:
      return GetStd140Alignment(type);
    case ShaderMemoryLayout::Std430:
      return GetStd430Alignment(type);
    case ShaderMemoryLayout::Packed:
      return GetScalarAlignment(type);
    default:
      COMET_ASSERT(false, "Unknown shader memory layout!");
      return kInvalidAlignment;
  }
}

ShaderOffset ShaderHandler::AlignOffset(ShaderOffset offset,
                                        Alignment alignment) {
  return static_cast<ShaderOffset>(
      memory::AlignSize(offset, static_cast<memory::Alignment>(alignment)));
}

ShaderFieldLayoutInfo ShaderHandler::GetFieldLayoutInfo(
    ShaderMemoryLayout layout, ShaderVariableType type, u32 array_count) {
  ShaderFieldLayoutInfo info{};

  info.element_size = GetShaderVariableTypeSize(type);
  COMET_ASSERT(info.element_size != kInvalidShaderVariableSize,
               "Invalid shader variable type size!");

  info.alignment = GetBindingFieldAlignment(layout, type);
  info.aligned_size = static_cast<ShaderVariableSize>(memory::AlignSize(
      info.element_size, static_cast<memory::Alignment>(info.alignment)));
  info.stride = info.aligned_size;
  info.total_size =
      array_count > 1
          ? static_cast<ShaderVariableSize>(info.stride * array_count)
          : info.aligned_size;

  return info;
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
      COMET_ASSERT(false, "Unsupported shader vertex layout!");
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

  attributes.PushBack(VertexAttribute{
      .index = 0,
      .component_count = 3,
      .component_type = GL_FLOAT,
      .is_normalized = GL_FALSE,
      .stride = stride,
      .offset = reinterpret_cast<const void*>(
          offsetof(geometry::SkinnedVertex, position)),
  });

  attributes.PushBack(VertexAttribute{
      .index = 1,
      .component_count = 3,
      .component_type = GL_FLOAT,
      .is_normalized = GL_FALSE,
      .stride = stride,
      .offset = reinterpret_cast<const void*>(
          offsetof(geometry::SkinnedVertex, normal)),
  });

  attributes.PushBack(VertexAttribute{
      .index = 2,
      .component_count = 4,
      .component_type = GL_FLOAT,
      .is_normalized = GL_FALSE,
      .stride = stride,
      .offset = reinterpret_cast<const void*>(
          offsetof(geometry::SkinnedVertex, tangent)),
  });

  attributes.PushBack(VertexAttribute{
      .index = 3,
      .component_count = 2,
      .component_type = GL_FLOAT,
      .is_normalized = GL_FALSE,
      .stride = stride,
      .offset =
          reinterpret_cast<const void*>(offsetof(geometry::SkinnedVertex, uv)),
  });

  attributes.PushBack(VertexAttribute{
      .index = 4,
      .component_count = 4,
      .component_type = GL_FLOAT,
      .is_normalized = GL_FALSE,
      .stride = stride,
      .offset = reinterpret_cast<const void*>(
          offsetof(geometry::SkinnedVertex, color)),
  });

  attributes.PushBack(VertexAttribute{
      .index = 5,
      .component_count = 4,
      .component_type = GL_UNSIGNED_SHORT,
      .is_normalized = GL_FALSE,
      .stride = stride,
      .offset = reinterpret_cast<const void*>(
          offsetof(geometry::SkinnedVertex, joint_indices)),
  });

  attributes.PushBack(VertexAttribute{
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
  attributes.Reserve(1);

  stride = static_cast<VertexAttributeStride>(sizeof(math::Vec4));

  attributes.PushBack(VertexAttribute{
      .index = 0,
      .component_count = 4,
      .component_type = GL_FLOAT,
      .is_normalized = GL_FALSE,
      .stride = stride,
      .offset = nullptr,
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
    COMET_LOG_RENDERING_DEBUG("Skipping zero-sized buffer binding at slot ",
                              binding.api_binding, ".");
#endif  // COMET_DEBUG_RENDERING
    return;
  }

  auto target{binding.type == ShaderBindingType::UniformBuffer
                  ? GL_UNIFORM_BUFFER
                  : GL_SHADER_STORAGE_BUFFER};

  glBindBufferRange(target, binding.api_binding, buffer_handle,
                    static_cast<GLintptr>(buffer_offset),
                    static_cast<GLsizeiptr>(buffer_size));
}

void ShaderHandler::BindImageBinding(const ShaderBinding& binding,
                                     const ShaderImageDescriptor* descriptors,
                                     u32 descriptor_count) {
  COMET_ASSERT(descriptors != nullptr, "Image descriptors are null!");
  COMET_ASSERT(descriptor_count == binding.descriptor_count,
               "Descriptor count mismatch for image binding!");

  for (u32 i{0}; i < descriptor_count; ++i) {
    auto unit{binding.api_binding + i};
    auto& descriptor{descriptors[i]};

    switch (binding.type) {
      case ShaderBindingType::CombinedImageSampler:
        COMET_ASSERT(descriptor.texture != nullptr,
                     "Combined image sampler requires a texture!");
        COMET_ASSERT(descriptor.sampler != nullptr,
                     "Combined image sampler requires a sampler!");
        glBindTextureUnit(unit, descriptor.texture->handle);
        glBindSampler(unit, descriptor.sampler->handle);
        break;

      case ShaderBindingType::SampledImage:
        COMET_ASSERT(descriptor.texture != nullptr,
                     "Sampled image requires a texture!");
        glBindTextureUnit(unit, descriptor.texture->handle);
        break;

      case ShaderBindingType::Sampler:
        COMET_ASSERT(descriptor.sampler != nullptr,
                     "Sampler binding requires a sampler!");
        glBindSampler(unit, descriptor.sampler->handle);
        break;

      case ShaderBindingType::StorageImage:
        COMET_ASSERT(descriptor.texture != nullptr,
                     "Storage image requires a texture!");
        glBindImageTexture(unit, descriptor.texture->handle, 0, GL_FALSE, 0,
                           internal::GetGlImageAccess(binding.type),
                           descriptor.texture->internal_format);
        break;

      default:
        COMET_ASSERT(false, "Unsupported image binding type!");
        break;
    }
  }
}

Shader* ShaderHandler::Generate(
    const resource::ShaderResource* shader_resource) {
  COMET_ASSERT(shader_resource != nullptr, "Shader resource is null!");

  auto* shader{shader_instance_allocator_.AllocateOneAndPopulate<Shader>()};
  auto shader_handle{shader_handle_handler_.Generate()};

  shader->handle = shader_handle;
  shader->id = shader_resource->id;
  shader->ref_count = 1;

  auto& rasterizer_resource{shader_resource->descr.rasterizer};
  shader->rasterizer = {
      .is_wireframe = rasterizer_resource.is_wireframe,
      .is_depth_bias = rasterizer_resource.is_depth_bias,
      .cull_mode = rasterizer_resource.cull_mode,
  };

  auto& depth_stencil_resource{shader_resource->descr.depth_stencil};
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
  shader->modules = Array<const ShaderModule*>{&general_allocator_};

  shader->global_descriptor_data.binding_indices =
      Array<ShaderBindingIndex>{&general_allocator_};
  shader->storage_descriptor_data.binding_indices =
      Array<ShaderBindingIndex>{&general_allocator_};

  shader->instances.list = Array<MaterialInstance>{&shader_instance_allocator_};
  shader->instances.indices = Map<MaterialId, u32>{&general_allocator_};

  HandleShaderModulesGeneration(shader, shader_resource);
  HandleProgramGeneration(shader);
  HandleAttributesGeneration(shader, shader_resource);
  HandleBindingsGeneration(shader, shader_resource);
  HandlePushConstantBlocksGeneration(shader, shader_resource);
  HandleUboBufferGeneration(shader);

  AllocateGlobalDescriptorSets(shader);
  AllocateStorageDescriptorSets(shader);

  auto shader_index{gid::GetIndex(shader_handle)};
  shaders_.Resize(shader_index + 1);
  shaders_[shader_index] = shader;

  return shader;
}

void ShaderHandler::Destroy(Shader* shader, bool is_destroying_handler) {
  COMET_ASSERT(shader != nullptr, "Shader is null!");

  auto shader_index{gid::GetIndex(shader->handle)};

  if (!is_destroying_handler) {
    COMET_ASSERT(shader->ref_count > 0, "Shader ref count is 0!");

    if (--shader->ref_count > 0) {
      return;
    }

    shader_ids_.Remove(shader->id);
    shader_handle_handler_.Destroy(shader->handle);
  }

  if (shader_index < shaders_.GetSize()) {
    shaders_[shader_index] = nullptr;
  }

  for (auto& instance : shader->instances.list) {
    DestroyBindingRuntimeData(instance.binding_data);
    instance.descriptor_data.binding_indices.Destroy();
  }

  shader->instances.list.Destroy();
  shader->instances.indices.Destroy();

  shader->global_descriptor_data.binding_indices.Destroy();
  shader->storage_descriptor_data.binding_indices.Destroy();
  shader->global_descriptor_data.update_frame = kInvalidFrameCount;
  shader->storage_descriptor_data.update_frame = kInvalidFrameCount;

  shader->global_ubo_data = {};
  shader->instance_ubo_data = {};

  if (shader->uniform_buffer_handle != kInvalidUniformBufferHandle) {
    glDeleteBuffers(1, &shader->uniform_buffer_handle);
    shader->uniform_buffer_handle = kInvalidUniformBufferHandle;
  }

  if (shader->vertex_attribute_handle != kInvalidVertexAttributeHandle) {
    glDeleteVertexArrays(1, &shader->vertex_attribute_handle);
    shader->vertex_attribute_handle = kInvalidVertexAttributeHandle;
  }

  for (auto& binding : shader->bindings) {
    binding.fields.Destroy();
  }

  for (auto& block : shader->push_constant_blocks) {
    block.fields.Destroy();
  }

  shader->vertex_attributes.Destroy();
  shader->bindings.Destroy();
  shader->push_constant_blocks.Destroy();

  if (shader_module_handler_->IsInitialized()) {
    for (auto& module : shader->modules) {
      shader_module_handler_->Destroy(module->id);
    }
  }

  shader->modules.Destroy();

  if (shader->graphics_handle != kInvalidShaderHandle) {
    glDeleteProgram(shader->graphics_handle);
    shader->graphics_handle = kInvalidShaderHandle;
  }

  if (shader->compute_handle != kInvalidShaderHandle) {
    glDeleteProgram(shader->compute_handle);
    shader->compute_handle = kInvalidShaderHandle;
  }

  shader_instance_allocator_.Deallocate(shader);
}

void ShaderHandler::DestroyBindingRuntimeData(
    ShaderBindingRuntimeData& binding_data) const {
  for (auto& runtime_binding : binding_data.image_bindings) {
    runtime_binding.descriptors.Destroy();
  }

  binding_data.image_bindings.Destroy();
}

void ShaderHandler::HandleShaderModulesGeneration(
    Shader* shader, const resource::ShaderResource* resource) {
  shader->modules.Reserve(resource->descr.shader_module_paths.GetSize());

  for (auto& shader_module_path : resource->descr.shader_module_paths) {
    shader->modules.PushBack(
        shader_module_handler_->GetOrGenerate(shader_module_path));
  }
}

void ShaderHandler::HandleProgramGeneration(Shader* shader) const {
  auto is_graphics{false};
  auto is_compute{false};

  for (auto& module : shader->modules) {
    if (module->type == GL_COMPUTE_SHADER) {
      is_compute = true;
    } else {
      is_graphics = true;
    }

    if (is_graphics && is_compute) {
      break;
    }
  }

  if (is_graphics) {
    shader->graphics_handle = glCreateProgram();
    HandleProgramCompilation(shader, ShaderBindType::Graphics);
  }

  if (is_compute) {
    shader->compute_handle = glCreateProgram();
    HandleProgramCompilation(shader, ShaderBindType::Compute);
  }
}

void ShaderHandler::HandleProgramCompilation(Shader* shader,
                                             ShaderBindType bind_type) const {
  auto handle{ResolveHandle(shader, bind_type)};
  COMET_ASSERT(handle != kInvalidShaderHandle, "Invalid program handle!");

  for (auto& module : shader->modules) {
    if (module->bind_type == bind_type) {
      shader_module_handler_->Attach(shader, module->id);
    }
  }

  glLinkProgram(handle);

#ifdef COMET_DEBUG
  GLint result{GL_FALSE};
  GLint info_log_len{0};
  glGetProgramiv(handle, GL_LINK_STATUS, &result);
  glGetProgramiv(handle, GL_INFO_LOG_LENGTH, &info_log_len);

  if (info_log_len > 0) {
    auto& error_message{*COMET_FRAME_ARRAY(GLchar)};
    error_message.Resize(info_log_len + 1);
    glGetProgramInfoLog(handle, info_log_len, nullptr, error_message.GetData());
    COMET_ASSERT(
        false, "Error while linking shader program: ", error_message.GetData());
  }
#endif  // COMET_DEBUG

  for (auto& module : shader->modules) {
    if (module->bind_type == bind_type) {
      shader_module_handler_->Detach(shader, module->id);
    }
  }
}

void ShaderHandler::HandleAttributesGeneration(
    Shader* shader, const resource::ShaderResource* resource) {
  COMET_ASSERT(shader != nullptr, "Shader is null!");
  COMET_ASSERT(resource != nullptr, "Shader resource is null!");

  PopulateVertexAttributes(resource->descr.vertex_layout,
                           shader->vertex_attributes,
                           shader->vertex_attribute_stride);

  if (shader->vertex_attributes.IsEmpty()) {
    return;
  }

  glGenVertexArrays(1, &shader->vertex_attribute_handle);
  glBindVertexArray(shader->vertex_attribute_handle);
  glBindVertexArray(kInvalidVertexAttributeHandle);
}

void ShaderHandler::HandleBindingsGeneration(
    Shader* shader, const resource::ShaderResource* resource) {
  shader->bindings.Reserve(resource->descr.bindings.GetSize());

  for (auto& binding_descr : resource->descr.bindings) {
    ShaderBinding binding{};
    binding.type = binding_descr.type;
    binding.scope = binding_descr.scope;
    binding.layout = binding_descr.layout;
    binding.stages = ResolveStageFlags(binding_descr.stages);
    binding.index = static_cast<ShaderBindingIndex>(shader->bindings.GetSize());
    binding.set = binding_descr.set;
    binding.binding = binding_descr.binding;
    binding.api_binding = ResolveBinding(binding.set, binding.binding);
    binding.descriptor_count = binding_descr.descriptor_count;
    binding.image_semantic = binding_descr.image_semantic;
    binding.fields = Array<ShaderField>{&general_allocator_};
    binding.fields.Reserve(binding_descr.fields.GetSize());

    for (auto& field_descr : binding_descr.fields) {
      ShaderField field{};
      field.type = field_descr.type;
      field.array_count = field_descr.array_count;
      binding.fields.PushBack(field);
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
      COMET_ASSERT(false, "Pass/Draw uniform buffers are not implemented yet!");
    }

    shader->bindings.PushBack(std::move(binding));
  }
}

void ShaderHandler::HandlePushConstantBlocksGeneration(
    Shader* shader, const resource::ShaderResource* resource) {
  shader->push_constant_blocks.Reserve(
      resource->descr.push_constants.GetSize());

  ShaderOffset push_constant_cursor{0};

  for (auto& block_descr : resource->descr.push_constants) {
    ShaderPushConstantBlock block{};
    block.index = static_cast<ShaderPushConstantIndex>(
        shader->push_constant_blocks.GetSize());
    block.stages = ResolveStageFlags(block_descr.stages);
    block.fields = Array<ShaderField>{&general_allocator_};
    block.fields.Reserve(block_descr.fields.GetSize());

    for (auto& field_descr : block_descr.fields) {
      ShaderField field{};
      field.type = field_descr.type;
      field.array_count = field_descr.array_count;
      block.fields.PushBack(field);
    }

    ComputePushConstantBlockLayout(block);

    push_constant_cursor = AlignOffset(push_constant_cursor, 4);
    block.offset = push_constant_cursor;
    push_constant_cursor += block.size;

    shader->push_constant_blocks.PushBack(std::move(block));
  }
}

void ShaderHandler::ComputeBindingLayout(ShaderBinding& binding) const {
  if (!internal::IsBufferBindingType(binding.type) ||
      binding.fields.IsEmpty()) {
    binding.size = 0;
    binding.stride = 0;
    return;
  }

  ShaderOffset cursor{0};

  for (ShaderFieldIndex i{0}; i < binding.fields.GetSize(); ++i) {
    auto& field{binding.fields[i]};
    auto info{
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

    auto info{GetFieldLayoutInfo(ShaderMemoryLayout::Std140, field.type,
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
               "Shader global UBO data is too big!");
  COMET_ASSERT(shader->instance_ubo_data.stride < max_range,
               "Shader instance UBO data is too big!");
#endif

  ShaderOffset push_constant_total_size{0};

  for (auto& block : shader->push_constant_blocks) {
    push_constant_total_size =
        math::Max(push_constant_total_size,
                  static_cast<ShaderOffset>(block.offset + block.size));
  }

  auto buffer_size{static_cast<GLsizeiptr>(shader->global_ubo_data.stride +
                                           shader->instance_ubo_data.stride *
                                               kMaxMaterialInstances +
                                           push_constant_total_size)};

  if (buffer_size == 0) {
    return;
  }

  glGenBuffers(1, &shader->uniform_buffer_handle);
  glBindBuffer(GL_UNIFORM_BUFFER, shader->uniform_buffer_handle);
  COMET_GL_SET_UNIFORM_BUFFER_DEBUG_LABEL(shader->uniform_buffer_handle,
                                          "shader_uniform_buffer");
  glBufferData(GL_UNIFORM_BUFFER, buffer_size, nullptr, GL_DYNAMIC_DRAW);
  glBindBuffer(GL_UNIFORM_BUFFER, kInvalidUniformBufferHandle);

  shader->global_ubo_data.offset = 0;
  shader->bound_global_ubo_offset = 0;
}

void ShaderHandler::AllocateGlobalDescriptorSets(Shader* shader) {
  shader->global_descriptor_data.binding_indices.Clear();

  for (auto& binding : shader->bindings) {
    if (binding.set == shaderconsts::kGlobalSet) {
      shader->global_descriptor_data.binding_indices.PushBack(binding.index);
    }
  }
}

void ShaderHandler::AllocateStorageDescriptorSets(Shader* shader) {
  shader->storage_descriptor_data.binding_indices.Clear();

  for (auto& binding : shader->bindings) {
    if (binding.set == shaderconsts::kPassSet) {
      shader->storage_descriptor_data.binding_indices.PushBack(binding.index);
    }
  }
}

void ShaderHandler::AllocateMaterialDescriptorSets(Shader* shader,
                                                   MaterialInstance& instance) {
  instance.descriptor_data.binding_indices.Clear();

  for (auto& binding : shader->bindings) {
    if (binding.set == shaderconsts::kMaterialSet) {
      instance.descriptor_data.binding_indices.PushBack(binding.index);
    }
  }
}

void ShaderHandler::CollectMaterialTextureMaps(
    const Material* material, const ShaderBinding& binding,
    Array<const TextureMap*>& texture_maps) const {
  COMET_ASSERT(material != nullptr, "Material is null!");
  COMET_ASSERT(internal::IsImageBindingType(binding.type),
               "Binding is not an image binding!");

  texture_maps.Clear();
  texture_maps.Reserve(binding.descriptor_count);

  switch (binding.image_semantic) {
    case ShaderImageBindingSemantic::MaterialDiffuse:
      COMET_ASSERT(binding.descriptor_count == 1,
                   "MaterialDiffuse binding must use exactly 1 descriptor!");
      texture_maps.PushBack(&material->diffuse_map);
      return;

    case ShaderImageBindingSemantic::MaterialSpecular:
      COMET_ASSERT(binding.descriptor_count == 1,
                   "MaterialSpecular binding must use exactly 1 descriptor!");
      texture_maps.PushBack(&material->specular_map);
      return;

    case ShaderImageBindingSemantic::MaterialNormal:
      COMET_ASSERT(binding.descriptor_count == 1,
                   "MaterialNormal binding must use exactly 1 descriptor!");
      texture_maps.PushBack(&material->normal_map);
      return;

    case ShaderImageBindingSemantic::MaterialTextures:
      COMET_ASSERT(binding.descriptor_count == 3,
                   "MaterialTextures binding must use exactly 3 descriptors!");
      texture_maps.PushBack(&material->diffuse_map);
      texture_maps.PushBack(&material->specular_map);
      texture_maps.PushBack(&material->normal_map);
      return;

    default:
      COMET_ASSERT(false, "Unsupported material image binding semantic!");
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

  for (auto* texture_map : texture_maps) {
    ShaderImageDescriptor descriptor{};
    descriptor.texture = texture_map->texture;
    descriptor.sampler = texture_map->sampler;
    descriptors.PushBack(descriptor);
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

void ShaderHandler::WriteBindingFieldToUbo(
    Shader* shader, ShaderOffset base_offset, const ShaderBinding& binding,
    ShaderFieldIndex field_index, const void* value, usize value_size) const {
  COMET_ASSERT(shader != nullptr, "Shader is null!");
  COMET_ASSERT(field_index < binding.fields.GetSize(),
               "Invalid field index for binding!");
  COMET_ASSERT(value != nullptr, "Field value is null!");
  COMET_ASSERT(shader->uniform_buffer_handle != kInvalidUniformBufferHandle,
               "Shader uniform buffer handle is invalid!");

  auto& field{binding.fields[field_index]};
  auto raw_size{static_cast<usize>(field.element_size)};
  auto copy_size{value_size == 0 ? raw_size : value_size};

  COMET_ASSERT(copy_size <= field.size,
               "Field update size exceeds field size!");

  glBindBuffer(GL_UNIFORM_BUFFER, shader->uniform_buffer_handle);
  glBufferSubData(GL_UNIFORM_BUFFER,
                  base_offset + binding.offset + field.offset,
                  static_cast<GLsizeiptr>(copy_size), value);
  glBindBuffer(GL_UNIFORM_BUFFER, kInvalidUniformBufferHandle);
}

void ShaderHandler::UpdateInstanceUboBindings(Shader* shader,
                                              MaterialInstance& instance,
                                              u32 instance_index) {
  COMET_ASSERT(shader != nullptr, "Shader is null!");

  instance.offset = shader->instance_ubo_data.stride * instance_index;

  for (auto& binding : shader->bindings) {
    if (binding.set != shaderconsts::kMaterialSet) {
      continue;
    }

    if (binding.type != ShaderBindingType::UniformBuffer) {
      continue;
    }

    BindBufferBinding(
        binding, shader->uniform_buffer_handle, binding.size,
        shader->global_ubo_data.stride + instance.offset + binding.offset);
  }
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

ShaderOffset ShaderHandler::GetPushConstantBaseOffset(
    const Shader* shader) const {
  return shader->global_ubo_data.stride +
         shader->instance_ubo_data.stride * kMaxMaterialInstances;
}
}  // namespace gl
}  // namespace rendering
}  // namespace comet