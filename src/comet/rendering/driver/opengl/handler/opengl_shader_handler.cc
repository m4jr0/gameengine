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
#include <algorithm>
#include <type_traits>
#include <utility>
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/c_string.h"
#include "comet/core/frame/frame_utils.h"
#include "comet/core/memory/memory_utils.h"
#include "comet/rendering/driver/opengl/opengl_debug.h"
#include "comet/resource/resource_manager.h"
#include "comet/resource/shader_resource.h"

namespace comet {
namespace rendering {
namespace gl {
ShaderHandler::ShaderHandler(const ShaderHandlerDescr& descr)
    : Handler{descr},
      shader_module_handler_{descr.shader_module_handler},
      material_handler_{descr.material_handler},
      mesh_handler_{descr.mesh_handler},
      texture_handler_{descr.texture_handler} {
  COMET_ASSERT(shader_module_handler_ != nullptr,
               "Shader module handler is null!");
  COMET_ASSERT(material_handler_ != nullptr, "Material handler is null!");
  COMET_ASSERT(mesh_handler_ != nullptr, "Mesh handler is null!");
  COMET_ASSERT(texture_handler_ != nullptr, "Texture handler is null!");
}

void ShaderHandler::Initialize() {
  Handler::Initialize();
  general_allocator_.Initialize();
  shader_instance_allocator_.Initialize();
  shaders_ = Map<ShaderId, Shader*>{&shader_instance_allocator_};
}

void ShaderHandler::Shutdown() {
  for (auto& it : shaders_) {
    Destroy(it.value, true);
  }

  shaders_.Destroy();
  bound_shader_handle_ = kInvalidShaderHandle;
  instance_id_handler_.Shutdown();
  shader_instance_allocator_.Destroy();
  general_allocator_.Destroy();
  Handler::Shutdown();
}

Shader* ShaderHandler::Generate(const ShaderDescr& descr) {
  const auto* shader_resource{
      resource::ResourceManager::Get().GetShaders()->Load(descr.resource_path)};
  COMET_ASSERT(shader_resource != nullptr, "Shader resource is null!");
  auto* shader{shader_instance_allocator_.AllocateOneAndPopulate<Shader>()};
  shader->id = shader_resource->id;
  shader->ref_count = 1;
  shader->is_wireframe = shader_resource->descr.is_wireframe;
  shader->cull_mode = GetGlCullMode(shader_resource->descr.cull_mode);
  shader->topology = GetGlPrimitiveTopology(shader_resource->descr.topology);

  shader->vertex_attributes = Array<VertexAttribute>{&general_allocator_};
  shader->uniforms = Array<ShaderUniform>{&general_allocator_};
  shader->constants = Array<ShaderConstant>{&general_allocator_};
  shader->storages = Array<ShaderStorage>{&general_allocator_};
  shader->modules = Array<const ShaderModule*>{&general_allocator_};
  shader->global_uniform_data.texture_maps =
      Array<const TextureMap*>{&general_allocator_};

  shader->instances.list = Array<MaterialInstance>{&shader_instance_allocator_};
  shader->instances.ids = Array<MaterialInstanceId>{&general_allocator_};

  HandleProgram(shader, shader_resource);
  HandleUniformsGeneration(shader, shader_resource);
  HandleConstantsGeneration(shader, shader_resource);
  HandleStorageGeneration(shader, shader_resource);
  HandleBindingsGeneration(shader);
  HandleUboBufferGeneration(shader);

  return shaders_.Emplace(shader->id, shader).value;
}

Shader* ShaderHandler::Get(ShaderId shader_id) {
  auto* shader{TryGet(shader_id)};
  COMET_ASSERT(shader != nullptr, "Requested shader does not exist: ",
               COMET_STRING_ID_LABEL(shader_id), "!");
  return shader;
}

Shader* ShaderHandler::TryGet(ShaderId shader_id) {
  auto shader_ptr{shaders_.TryGet(shader_id)};

  if (shader_ptr == nullptr) {
    return nullptr;
  }

  auto* shader{*shader_ptr};
  ++shader->ref_count;
  return shader;
}

void ShaderHandler::Bind(Shader* shader, ShaderBindType bind_type) {
  COMET_ASSERT(shader != nullptr, "Shader provided is null!");
  auto handle{ResolveHandle(shader, bind_type)};

  COMET_ASSERT(handle != kInvalidShaderHandle,
               "Invalid shader program handle!");

  if (handle == bound_shader_handle_) {
    return;
  }

  if (bound_vertex_attribute_handle_ != shader->vertex_attribute_handle) {
    glBindVertexArray(shader->vertex_attribute_handle);
  }

  mesh_handler_->Bind();
  RebindVertexAttributes(shader);

  glUseProgram(handle);
  bound_shader_handle_ = handle;

  if (bind_type == ShaderBindType::Graphics) {
    glPolygonMode(GL_FRONT_AND_BACK, shader->is_wireframe ? GL_LINE : GL_FILL);

    if (shader->cull_mode != GL_NONE) {
      glEnable(GL_CULL_FACE);
      glCullFace(shader->cull_mode);
    } else {
      glDisable(GL_CULL_FACE);
    }

    glBindVertexArray(shader->vertex_attribute_handle);
  }
}

void ShaderHandler::BindInstance(Shader* shader, MaterialId material_id) {
  BindInstance(shader, material_handler_->Get(material_id));
}

void ShaderHandler::BindInstance(Shader* shader, const Material* material) {
  shader->bound_instance_index =
      GetInstanceIndex(shader, material->instance_id);
  shader->bound_ubo_offset =
      shader->global_ubo_data.ubo_stride +
      shader->instances.list[shader->bound_instance_index].offset;

  glBindBufferRange(GL_UNIFORM_BUFFER, shader->uniform_buffer_indices.instance,
                    shader->uniform_buffer_handle, shader->bound_ubo_offset,
                    shader->instance_ubo_data.ubo_stride);
}

void ShaderHandler::Destroy(ShaderId shader_id) { Destroy(Get(shader_id)); }

void ShaderHandler::Destroy(Shader* shader) { Destroy(shader, false); }

void ShaderHandler::Reset() { bound_shader_handle_ = kInvalidShaderHandle; }

void ShaderHandler::UpdateGlobals(Shader* shader,
                                  const frame::FramePacket* packet) const {
  UpdateGlobals(shader, static_cast<FrameCount>(packet->frame_count),
                {&packet->projection_matrix, &packet->view_matrix});
}

void ShaderHandler::UpdateGlobals(Shader* shader, FrameCount frame_count,
                                  const ShaderGlobalsUpdate& update) const {
  auto global_buffer_index{shader->uniform_buffer_indices.global};

  if (global_buffer_index == kInvalidShaderUniformBufferIndex) {
    return;
  }

  if (shader->global_uniform_data.update_frame == frame_count) {
    return;
  }

  shader->bound_ubo_offset = shader->global_ubo_data.ubo_offset;
  glBindBufferRange(GL_UNIFORM_BUFFER, shader->uniform_buffer_indices.global,
                    shader->uniform_buffer_handle, shader->bound_ubo_offset,
                    shader->global_ubo_data.ubo_stride);
  SetUniform(shader, shader->uniform_indices.projection,
             update.projection_matrix);
  SetUniform(shader, shader->uniform_indices.view, update.view_matrix);
  shader->global_uniform_data.update_frame = frame_count;
}

void ShaderHandler::UpdateConstants(Shader* shader,
                                    const frame::FramePacket* packet) const {
  auto draw_count{static_cast<u32>(packet->draw_count)};
  UpdateConstants(shader, {&draw_count, nullptr});
}

void ShaderHandler::UpdateConstants(Shader* shader,
                                    const ShaderConstantsUpdate& update) const {
  if (update.draw_count != nullptr) {
    SetConstant(shader, shader->constant_indices.draw_count, update.draw_count);
  }

  if (update.count != nullptr) {
    SetConstant(shader, shader->constant_indices.count, update.count);
  }
}

void ShaderHandler::UpdateStorages(Shader* shader,
                                   const frame::FramePacket* packet) const {
  if (packet->rendering_data == nullptr) {
    return;
  }

  const auto* update{
      static_cast<const ShaderStoragesUpdate*>(packet->rendering_data)};
  UpdateStorages(shader, *update);
}

void ShaderHandler::UpdateStorages(Shader* shader,
                                   const ShaderStoragesUpdate& update) const {
  BindStorageBuffer(shader, shader->storage_indices.proxy_local_datas,
                    update.ssbo_proxy_local_datas_handle);

  BindStorageBuffer(shader, shader->storage_indices.proxy_ids,
                    update.ssbo_proxy_ids_handle);

  BindStorageBuffer(shader, shader->storage_indices.proxy_instances,
                    update.ssbo_proxy_instances_handle);

  BindStorageBuffer(shader, shader->storage_indices.indirect_proxies,
                    update.ssbo_indirect_proxies_handle);

  BindStorageBuffer(shader, shader->storage_indices.word_indices,
                    update.ssbo_word_indices_handle);

  BindStorageBuffer(shader, shader->storage_indices.source_words,
                    update.ssbo_source_words_handle);

  BindStorageBuffer(shader, shader->storage_indices.destination_words,
                    update.ssbo_destination_words_handle);

  BindStorageBuffer(shader, shader->storage_indices.matrix_palettes,
                    update.ssbo_matrix_palettes_handle);

#ifdef COMET_DEBUG_RENDERING
  BindStorageBuffer(shader, shader->storage_indices.debug_data,
                    update.ssbo_debug_data_handle);
#endif  // COMET_DEBUG_RENDERING

#ifdef COMET_DEBUG_CULLING
  BindStorageBuffer(shader, shader->storage_indices.debug_aabbs,
                    update.ssbo_debug_aabbs_handle);

  BindStorageBuffer(shader, shader->storage_indices.line_vertices,
                    update.ssbo_debug_lines_handle);
#endif  // COMET_DEBUG_CULLING
}

void ShaderHandler::UpdateInstance(Shader* shader, FrameCount frame_count,
                                   MaterialId material_id) {
  UpdateInstance(shader, frame_count, material_handler_->Get(material_id));
}

void ShaderHandler::UpdateInstance(Shader* shader, FrameCount frame_count,
                                   Material* material) {
  if (material->instance_update_frame != frame_count) {
    shader->bound_instance_index =
        GetInstanceIndex(shader, material->instance_id);
    shader->bound_ubo_offset =
        shader->global_ubo_data.ubo_stride +
        shader->instances.list[shader->bound_instance_index].offset;

    glBindBufferRange(GL_UNIFORM_BUFFER,
                      shader->uniform_buffer_indices.instance,
                      shader->uniform_buffer_handle, shader->bound_ubo_offset,
                      shader->instance_ubo_data.ubo_stride);

    SetUniform(shader, shader->uniform_indices.diffuse_map,
               &material->diffuse_map);
    SetUniform(shader, shader->uniform_indices.specular_map,
               &material->specular_map);
    SetUniform(shader, shader->uniform_indices.normal_map,
               &material->normal_map);
    SetUniform(shader, shader->uniform_indices.diffuse_color,
               &material->diffuse_color);
    SetUniform(shader, shader->uniform_indices.shininess, &material->shininess);
  }

  material->instance_update_frame = frame_count;
}

void ShaderHandler::SetUniform(Shader* shader, const ShaderUniform& uniform,
                               const void* value) const {
  if (uniform.type == ShaderVariableType::Sampler) {
    const TextureMap* texture_map;

    if (uniform.scope == ShaderUniformScope::Global) {
      texture_map = shader->global_uniform_data.texture_maps[uniform.location] =
          static_cast<const TextureMap*>(value);
    } else {
      texture_map = shader->instances.list[shader->bound_instance_index]
                        .uniform_data.texture_maps[uniform.location] =
          static_cast<const TextureMap*>(value);
    }

    glActiveTexture(GL_TEXTURE0 + texture_map->type);
    glBindTexture(GL_TEXTURE_2D, texture_map->texture_handle);
    glUniform1i(uniform.location, texture_map->type);
    return;
  }

  glBindBuffer(GL_UNIFORM_BUFFER, shader->uniform_buffer_handle);
  glBufferSubData(GL_UNIFORM_BUFFER, shader->bound_ubo_offset + uniform.offset,
                  uniform.size, value);
  glBindBuffer(GL_UNIFORM_BUFFER, kInvalidUniformBufferHandle);
}

void ShaderHandler::SetUniform(ShaderId id, const ShaderUniform& uniform,
                               const void* value) {
  auto* shader{Get(id)};
  COMET_ASSERT(shader != nullptr, "Tried to retrieve shader pass from ID ", id,
               ", but instance is null!");
  SetUniform(shader, uniform, value);
}

void ShaderHandler::SetUniform(Shader* shader, ShaderUniformIndex index,
                               const void* value) const {
  COMET_ASSERT(index < shader->uniforms.GetSize(),
               "Tried to set uniform at index #", index,
               " for shader pass, but uniform count is ",
               shader->uniforms.GetSize(), "!");
  SetUniform(shader, shader->uniforms[index], value);
}

void ShaderHandler::SetUniform(ShaderId id, ShaderUniformIndex index,
                               const void* value) {
  auto* shader{Get(id)};
  COMET_ASSERT(shader != nullptr, "Tried to retrieve shader pass from ID ", id,
               ", but instance is null!");
  SetUniform(shader, index, value);
}

void ShaderHandler::SetConstant(const ShaderConstant& constant,
                                const void* value) const {
  switch (constant.type) {
    case ShaderVariableType::B32:
      SetS32(constant.location, value);
      break;
    case ShaderVariableType::S32:
      SetS32(constant.location, value);
      break;
    case ShaderVariableType::U32:
      SetU32(constant.location, value);
      break;
    case ShaderVariableType::F32:
      SetF32(constant.location, value);
      break;
    case ShaderVariableType::B32Vec2:
      SetS32Vec2(constant.location, value);
      break;
    case ShaderVariableType::B32Vec3:
      SetS32Vec3(constant.location, value);
      break;
    case ShaderVariableType::B32Vec4:
      SetS32Vec4(constant.location, value);
      break;
    case ShaderVariableType::S32Vec2:
      SetS32Vec2(constant.location, value);
      break;
    case ShaderVariableType::S32Vec3:
      SetS32Vec3(constant.location, value);
      break;
    case ShaderVariableType::S32Vec4:
      SetS32Vec4(constant.location, value);
      break;
    case ShaderVariableType::U32Vec2:
      SetU32Vec2(constant.location, value);
      break;
    case ShaderVariableType::U32Vec3:
      SetU32Vec3(constant.location, value);
      break;
    case ShaderVariableType::U32Vec4:
      SetU32Vec4(constant.location, value);
      break;
    case ShaderVariableType::Vec2:
      SetF32Vec2(constant.location, value);
      break;
    case ShaderVariableType::Vec3:
      SetF32Vec3(constant.location, value);
      break;
    case ShaderVariableType::Vec4:
      SetF32Vec4(constant.location, value);
      break;
    case ShaderVariableType::Mat2x2:
      SetMat2(constant.location, value);
      break;
    case ShaderVariableType::Mat2x3:
      SetMat2x3(constant.location, value);
      break;
    case ShaderVariableType::Mat2x4:
      SetMat2x4(constant.location, value);
      break;
    case ShaderVariableType::Mat3x2:
      SetMat3x2(constant.location, value);
      break;
    case ShaderVariableType::Mat3x3:
      SetMat3(constant.location, value);
      break;
    case ShaderVariableType::Mat3x4:
      SetMat3x4(constant.location, value);
      break;
    case ShaderVariableType::Mat4x2:
      SetMat4x2(constant.location, value);
      break;
    case ShaderVariableType::Mat4x3:
      SetMat4x3(constant.location, value);
      break;
    case ShaderVariableType::Mat4x4:
      SetMat4(constant.location, value);
      break;
    case ShaderVariableType::Sampler:
      SetS32(constant.location, value);
      break;
    default:
      COMET_ASSERT("Unknown or unsupported shader constant type: ",
                   static_cast<std::underlying_type_t<ShaderVariableType>>(
                       constant.type),
                   "!");
      break;
  }
}

void ShaderHandler::SetConstant(ShaderId shader_id, ShaderConstantIndex index,
                                const void* value) const {
  SetConstant(Get(shader_id)->constants[index], value);
}

void ShaderHandler::SetConstant(const Shader* shader, ShaderConstantIndex index,
                                const void* value) const {
  SetConstant(shader->constants[index], value);
}

void ShaderHandler::BindMaterial(Material* material) {
  auto* shader{Get(material->shader_id)};

  MaterialInstance instance{};
  auto instance_texture_count{shader->instance_ubo_data.sampler_count};

  instance.uniform_data.texture_maps =
      Array<const TextureMap*>{&general_allocator_};

  if (instance_texture_count > 0) {
    instance.uniform_data.texture_maps.Resize(instance_texture_count);

    // TODO(m4jr0): Put texture maps in generic array.
    StaticArray<TextureMap*, 3> maps{
        &material->diffuse_map, &material->specular_map, &material->normal_map};
    memory::CopyMemory(instance.uniform_data.texture_maps.GetData(),
                       maps.GetData(), sizeof(TextureMap*) * maps.GetSize());
  }

  instance.offset =
      shader->instance_ubo_data.ubo_stride * shader->instances.list.GetSize();

  // Generate material instance.
  shader->instances.list.EmplaceBack(std::move(instance));
  material->instance_id = instance_id_handler_.Generate();
  shader->instances.ids.EmplaceBack(material->instance_id);
}

void ShaderHandler::UnbindMaterial(Material* material) {
  COMET_ASSERT(HasMaterial(material),
               "Trying to destroy dead material instance #",
               material->instance_id, "!");

  auto* shader{Get(material->shader_id)};
  auto index{GetInstanceIndex(shader, material)};
  auto& instances{shader->instances.list};

  auto& instance{instances[index]};
  instance.uniform_data.texture_maps.Clear();

  // Delete material instance.
  // Copy last instance in place of the deleted one + Remove last duplicate in
  // the list.
  auto& instance_ids{shader->instances.ids};
  COMET_ASSERT(instance_ids.GetSize() == instances.GetSize(),
               "Size mismatch between material IDs (", instance_ids.GetSize(),
               ") and instances (", instances.GetSize(),
               ")! Something very wrong must have happened!");
  auto max_index{instance_ids.GetSize() - 1};

  if (index != max_index) {
    instance_ids[index] = instance_ids[max_index];
    instances[index] = instances[max_index];
  }

  instance_ids.Resize(max_index);
  instances.Resize(max_index);
  instance_id_handler_.Destroy(material->instance_id);
  material->instance_id = kInvalidMaterialInstanceId;
}

bool ShaderHandler::HasMaterial(const Material* material) const {
  return instance_id_handler_.IsAlive(material->instance_id);
}

MaterialInstance& ShaderHandler::GetInstance(Shader* shader,
                                             const Material* material) {
  return shader->instances.list[GetInstanceIndex(shader, material)];
}

ShaderUniformSize ShaderHandler::GetShaderVariableTypeSize(
    ShaderVariableType type) {
  switch (type) {
    case ShaderVariableType::B32:
    case ShaderVariableType::S32:
    case ShaderVariableType::U32:
    case ShaderVariableType::F32:
      return 4;

    case ShaderVariableType::F64:
      return 8;

    case ShaderVariableType::B32Vec2:
    case ShaderVariableType::S32Vec2:
    case ShaderVariableType::U32Vec2:
    case ShaderVariableType::Vec2:
      return 8;

    case ShaderVariableType::B32Vec3:
    case ShaderVariableType::S32Vec3:
    case ShaderVariableType::U32Vec3:
    case ShaderVariableType::Vec3:
      return 12;

    case ShaderVariableType::B32Vec4:
    case ShaderVariableType::S32Vec4:
    case ShaderVariableType::U32Vec4:
    case ShaderVariableType::Vec4:
      return 16;

    case ShaderVariableType::F64Vec2:
      return 16;

    case ShaderVariableType::F64Vec3:
      return 24;

    case ShaderVariableType::F64Vec4:
      return 32;

    case ShaderVariableType::Mat2x2:
      return 8;

    case ShaderVariableType::Mat2x3:
    case ShaderVariableType::Mat3x2:
      return 24;

    case ShaderVariableType::Mat3x3:
      return 36;

    case ShaderVariableType::Mat2x4:
    case ShaderVariableType::Mat4x2:
      return 32;

    case ShaderVariableType::Mat3x4:
    case ShaderVariableType::Mat4x3:
      return 48;

    case ShaderVariableType::Mat4x4:
      return 64;

    case ShaderVariableType::Sampler:
    case ShaderVariableType::Image:
    case ShaderVariableType::Atomic:
      return kInvalidShaderUniformSize;

    default:
      COMET_ASSERT(
          false, "Unknown or unsupported shader data type: ",
          static_cast<std::underlying_type_t<ShaderVariableType>>(type), "!");
  }

  return kInvalidShaderUniformSize;
}

void ShaderHandler::AddStorage(Shader* shader,
                               const ShaderStorageDescr& descr) const {
  auto& storage{shader->storages.EmplaceBack()};
  HandleStorageIndexAndBinding(shader, descr, storage);

  auto property_count{
      static_cast<ShaderStoragePropertySize>(descr.properties.GetSize())};

  if (property_count > kMaxShaderStorageLayoutPropertyCount) {
    COMET_LOG_RENDERING_ERROR("Too many storage properties: ", property_count,
                              "! Max is ", kMaxShaderStorageLayoutPropertyCount,
                              ". Exceeding properties will be ignored.");
    property_count = kMaxShaderStorageLayoutPropertyCount;
  }

  storage.property_count = property_count;

  for (ShaderStoragePropertySize i{0}; i < storage.property_count; ++i) {
    const auto& property_descr{descr.properties[i]};
    auto& property{storage.properties[i]};
    property.type = property_descr.type;
    property.size = GetShaderVariableTypeSize(property.type);
  }
}

GLenum ShaderHandler::GetGlCullMode(CullMode cull_mode) {
  auto gl_cull_mode{GL_INVALID_VALUE};

  switch (cull_mode) {
    case CullMode::None:
      gl_cull_mode = GL_NONE;
      break;
    case CullMode::Front:
      gl_cull_mode = GL_FRONT;
      break;
    case CullMode::Back:
      gl_cull_mode = GL_BACK;
      break;
    case CullMode::FrontAndBack:
      gl_cull_mode = GL_FRONT_AND_BACK;
      break;
    default:
      COMET_ASSERT(false, "Unknown or unsupported cull mode provided: ",
                   static_cast<std::underlying_type_t<CullMode>>(cull_mode),
                   "!");
  }

  return gl_cull_mode;
}

GLenum ShaderHandler::GetGlPrimitiveTopology(PrimitiveTopology topology) {
  auto gl_topology{GL_INVALID_VALUE};

  switch (topology) {
    case PrimitiveTopology::Points:
      gl_topology = GL_POINTS;
      break;
    case PrimitiveTopology::Lines:
      gl_topology = GL_LINES;
      break;
    case PrimitiveTopology::LineStrip:
      gl_topology = GL_LINE_STRIP;
      break;
    case PrimitiveTopology::Triangles:
      gl_topology = GL_TRIANGLES;
      break;
    case PrimitiveTopology::TriangleStrip:
      gl_topology = GL_TRIANGLE_STRIP;
      break;
    default:
      COMET_ASSERT(
          false, "Unknown or unsupported primitive topology provided: ",
          static_cast<std::underlying_type_t<PrimitiveTopology>>(topology),
          "!");
  }

  return gl_topology;
}

// https://www.khronos.org/opengl/wiki/Vertex_Specification_Best_Practices
GLsizei ShaderHandler::GetGlVertexAttributeSize(
    ShaderVertexAttributeType type) {
  switch (type) {
    case ShaderVertexAttributeType::S8:
    case ShaderVertexAttributeType::U8:
    case ShaderVertexAttributeType::S32:
    case ShaderVertexAttributeType::U32:
    case ShaderVertexAttributeType::F32:
      return 4;

    case ShaderVertexAttributeType::F64:
      return 8;

    case ShaderVertexAttributeType::U8Vec2:
    case ShaderVertexAttributeType::S8Vec2:
    case ShaderVertexAttributeType::U8Vec4:
    case ShaderVertexAttributeType::S8Vec4:
    case ShaderVertexAttributeType::U16Vec2:
    case ShaderVertexAttributeType::S16Vec2:
    case ShaderVertexAttributeType::F16Vec2:
      return 4;

    case ShaderVertexAttributeType::U16Vec3:
    case ShaderVertexAttributeType::S16Vec3:
    case ShaderVertexAttributeType::F16Vec3:
    case ShaderVertexAttributeType::U16Vec4:
    case ShaderVertexAttributeType::S16Vec4:
    case ShaderVertexAttributeType::F16Vec4:
    case ShaderVertexAttributeType::U32Vec2:
    case ShaderVertexAttributeType::S32Vec2:
    case ShaderVertexAttributeType::F32Vec2:
      return 8;

    case ShaderVertexAttributeType::U32Vec3:
    case ShaderVertexAttributeType::S32Vec3:
    case ShaderVertexAttributeType::F32Vec3:
      return 12;

    case ShaderVertexAttributeType::U32Vec4:
    case ShaderVertexAttributeType::S32Vec4:
    case ShaderVertexAttributeType::F32Vec4:
    case ShaderVertexAttributeType::F64Vec2:
      return 16;

    case ShaderVertexAttributeType::F64Vec3:
      return 24;

    case ShaderVertexAttributeType::F64Vec4:
      return 32;

    case ShaderVertexAttributeType::Unknown:
    default:
      return 0;
  }
}

GLint ShaderHandler::GetGlComponentCount(ShaderVertexAttributeType type) {
  switch (type) {
    case ShaderVertexAttributeType::S8:
    case ShaderVertexAttributeType::S16:
    case ShaderVertexAttributeType::S32:
    case ShaderVertexAttributeType::U8:
    case ShaderVertexAttributeType::U16:
    case ShaderVertexAttributeType::U32:
    case ShaderVertexAttributeType::F16:
    case ShaderVertexAttributeType::F32:
    case ShaderVertexAttributeType::F64:
      return 1;

    case ShaderVertexAttributeType::U8Vec2:
    case ShaderVertexAttributeType::U16Vec2:
    case ShaderVertexAttributeType::U32Vec2:
    case ShaderVertexAttributeType::S8Vec2:
    case ShaderVertexAttributeType::S16Vec2:
    case ShaderVertexAttributeType::S32Vec2:
    case ShaderVertexAttributeType::F16Vec2:
    case ShaderVertexAttributeType::F32Vec2:
    case ShaderVertexAttributeType::F64Vec2:
      return 2;

    case ShaderVertexAttributeType::U8Vec3:
    case ShaderVertexAttributeType::U16Vec3:
    case ShaderVertexAttributeType::U32Vec3:
    case ShaderVertexAttributeType::S8Vec3:
    case ShaderVertexAttributeType::S16Vec3:
    case ShaderVertexAttributeType::S32Vec3:
    case ShaderVertexAttributeType::F16Vec3:
    case ShaderVertexAttributeType::F32Vec3:
    case ShaderVertexAttributeType::F64Vec3:
      return 3;

    case ShaderVertexAttributeType::U8Vec4:
    case ShaderVertexAttributeType::U16Vec4:
    case ShaderVertexAttributeType::U32Vec4:
    case ShaderVertexAttributeType::S8Vec4:
    case ShaderVertexAttributeType::S16Vec4:
    case ShaderVertexAttributeType::S32Vec4:
    case ShaderVertexAttributeType::F16Vec4:
    case ShaderVertexAttributeType::F32Vec4:
    case ShaderVertexAttributeType::F64Vec4:
      return 4;

    default:
      return 0;
  }
}

GLenum ShaderHandler::GetGlVertexAttributeType(ShaderVertexAttributeType type) {
  switch (type) {
    case ShaderVertexAttributeType::S8:
    case ShaderVertexAttributeType::S8Vec2:
    case ShaderVertexAttributeType::S8Vec3:
    case ShaderVertexAttributeType::S8Vec4:
      return GL_BYTE;

    case ShaderVertexAttributeType::S16:
    case ShaderVertexAttributeType::S16Vec2:
    case ShaderVertexAttributeType::S16Vec3:
    case ShaderVertexAttributeType::S16Vec4:
      return GL_SHORT;

    case ShaderVertexAttributeType::S32:
    case ShaderVertexAttributeType::S32Vec2:
    case ShaderVertexAttributeType::S32Vec3:
    case ShaderVertexAttributeType::S32Vec4:
      return GL_INT;

    case ShaderVertexAttributeType::U8:
    case ShaderVertexAttributeType::U8Vec2:
    case ShaderVertexAttributeType::U8Vec3:
    case ShaderVertexAttributeType::U8Vec4:
      return GL_UNSIGNED_BYTE;

    case ShaderVertexAttributeType::U16:
    case ShaderVertexAttributeType::U16Vec2:
    case ShaderVertexAttributeType::U16Vec3:
    case ShaderVertexAttributeType::U16Vec4:
      return GL_UNSIGNED_SHORT;

    case ShaderVertexAttributeType::U32:
    case ShaderVertexAttributeType::U32Vec2:
    case ShaderVertexAttributeType::U32Vec3:
    case ShaderVertexAttributeType::U32Vec4:
      return GL_UNSIGNED_INT;

    case ShaderVertexAttributeType::F16:
    case ShaderVertexAttributeType::F16Vec2:
    case ShaderVertexAttributeType::F16Vec3:
    case ShaderVertexAttributeType::F16Vec4:
      return GL_HALF_FLOAT;

    case ShaderVertexAttributeType::F32:
    case ShaderVertexAttributeType::F32Vec2:
    case ShaderVertexAttributeType::F32Vec3:
    case ShaderVertexAttributeType::F32Vec4:
      return GL_FLOAT;

    case ShaderVertexAttributeType::F64:
    case ShaderVertexAttributeType::F64Vec2:
    case ShaderVertexAttributeType::F64Vec3:
    case ShaderVertexAttributeType::F64Vec4:
      return GL_DOUBLE;

    default:
      return 0;
  }
}

bool ShaderHandler::IsIntegerVertexAttributeType(GLenum gl_type) {
  switch (gl_type) {
    case GL_BYTE:
    case GL_UNSIGNED_BYTE:
    case GL_SHORT:
    case GL_UNSIGNED_SHORT:
    case GL_INT:
    case GL_UNSIGNED_INT:
      return true;

    default:
      return false;
  }
}

void ShaderHandler::BindStorageBuffer(const Shader* shader,
                                      ShaderStorageIndex storage_index,
                                      GLuint buffer_handle) {
  if (storage_index == kInvalidShaderStorageIndex ||
      buffer_handle == kInvalidStorageBufferHandle) {
    return;
  }

  auto binding{shader->storages[storage_index].binding};
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding, buffer_handle);
}

void ShaderHandler::SetF32(s32 location, const void* value) {
  glUniform1f(location, *static_cast<const GLfloat*>(value));
}

void ShaderHandler::SetS32(s32 location, const void* value) {
  glUniform1i(location, *static_cast<const GLint*>(value));
}

void ShaderHandler::SetU32(s32 location, const void* value) {
  glUniform1ui(location, *static_cast<const GLuint*>(value));
}

void ShaderHandler::SetF32Vec2(s32 location, const void* value) {
  glUniform2fv(location, 1, static_cast<const GLfloat*>(value));
}

void ShaderHandler::SetF32Vec3(s32 location, const void* value) {
  glUniform3fv(location, 1, static_cast<const GLfloat*>(value));
}

void ShaderHandler::SetF32Vec4(s32 location, const void* value) {
  glUniform4fv(location, 1, static_cast<const GLfloat*>(value));
}

void ShaderHandler::SetS32Vec2(s32 location, const void* value) {
  glUniform2iv(location, 1, static_cast<const GLint*>(value));
}

void ShaderHandler::SetS32Vec3(s32 location, const void* value) {
  glUniform3iv(location, 1, static_cast<const GLint*>(value));
}

void ShaderHandler::SetS32Vec4(s32 location, const void* value) {
  glUniform4iv(location, 1, static_cast<const GLint*>(value));
}

void ShaderHandler::SetU32Vec2(s32 location, const void* value) {
  glUniform2uiv(location, 1, static_cast<const GLuint*>(value));
}

void ShaderHandler::SetU32Vec3(s32 location, const void* value) {
  glUniform3uiv(location, 1, static_cast<const GLuint*>(value));
}

void ShaderHandler::SetU32Vec4(s32 location, const void* value) {
  glUniform4uiv(location, 1, static_cast<const GLuint*>(value));
}

void ShaderHandler::SetMat2(s32 location, const void* value) {
  glUniformMatrix2fv(location, 1, GL_FALSE, static_cast<const GLfloat*>(value));
}

void ShaderHandler::SetMat3(s32 location, const void* value) {
  glUniformMatrix3fv(location, 1, GL_FALSE, static_cast<const GLfloat*>(value));
}

void ShaderHandler::SetMat4(s32 location, const void* value) {
  glUniformMatrix4fv(location, 1, GL_FALSE, static_cast<const GLfloat*>(value));
}

void ShaderHandler::SetMat2x3(s32 location, const void* value) {
  glUniformMatrix2x3fv(location, 1, GL_FALSE,
                       static_cast<const GLfloat*>(value));
}

void ShaderHandler::SetMat3x2(s32 location, const void* value) {
  glUniformMatrix3x2fv(location, 1, GL_FALSE,
                       static_cast<const GLfloat*>(value));
}

void ShaderHandler::SetMat2x4(s32 location, const void* value) {
  glUniformMatrix2x4fv(location, 1, GL_FALSE,
                       static_cast<const GLfloat*>(value));
}

void ShaderHandler::SetMat4x2(s32 location, const void* value) {
  glUniformMatrix4x2fv(location, 1, GL_FALSE,
                       static_cast<const GLfloat*>(value));
}

void ShaderHandler::SetMat3x4(s32 location, const void* value) {
  glUniformMatrix3x4fv(location, 1, GL_FALSE,
                       static_cast<const GLfloat*>(value));
}

void ShaderHandler::SetMat4x3(s32 location, const void* value) {
  glUniformMatrix4x3fv(location, 1, GL_FALSE,
                       static_cast<const GLfloat*>(value));
}

const Shader* ShaderHandler::Get(ShaderId shader_id) const {
  auto* shader{TryGet(shader_id)};
  COMET_ASSERT(shader != nullptr, "Requested shader does not exist: ",
               COMET_STRING_ID_LABEL(shader_id), "!");
  return shader;
}

const Shader* ShaderHandler::TryGet(ShaderId shader_id) const {
  auto shader{shaders_.TryGet(shader_id)};

  if (shader == nullptr) {
    return nullptr;
  }

  return *shader;
}

void ShaderHandler::Destroy(Shader* shader, bool is_destroying_handler) {
  if (!is_destroying_handler) {
    COMET_ASSERT(shader->ref_count > 0, "Shader has a reference count of 0!");

    if (--shader->ref_count > 0) {
      return;
    }
  }

  shader->global_uniform_data = {};
  shader->storage_data = {};
  shader->global_ubo_data = {};
  shader->instance_ubo_data = {};

  if (shader->uniform_buffer_handle != kInvalidUniformBufferHandle) {
    glDeleteBuffers(1, &shader->uniform_buffer_handle);
  }

  shader->instances.ids.Destroy();
  shader->instances.list.Destroy();
  shader->uniforms.Destroy();
  shader->constants.Destroy();
  shader->storages.Destroy();

  if (shader_module_handler_->IsInitialized()) {
    for (auto& module : shader->modules) {
      shader_module_handler_->Destroy(module->handle);
    }
  }

  shader->modules.Destroy();
  shader->constants.Destroy();

  if (!is_destroying_handler) {
    shaders_.Remove(shader->id);
  }

  shader_instance_allocator_.Deallocate(shader);
}

void ShaderHandler::RebindVertexAttributes(Shader* shader) const {
  auto vertex_buffer_handle{mesh_handler_->GetVertexBufferHandle()};
  auto index_buffer_handle{mesh_handler_->GetIndexBufferHandle()};

  if (shader->vertex_buffer_handle == vertex_buffer_handle &&
      shader->index_buffer_handle == index_buffer_handle) {
    return;
  }

  mesh_handler_->Bind();
  shader->vertex_buffer_handle = vertex_buffer_handle;
  shader->index_buffer_handle = index_buffer_handle;
  glBindVertexArray(shader->vertex_attribute_handle);

  for (const auto& attr : shader->vertex_attributes) {
    glEnableVertexAttribArray(attr.index);

    if (IsIntegerVertexAttributeType(attr.component_type)) {
      glVertexAttribIPointer(attr.index, attr.component_count,
                             attr.component_type, attr.stride, attr.offset);
    } else {
      glVertexAttribPointer(attr.index, attr.component_count,
                            attr.component_type, attr.is_normalized,
                            attr.stride, attr.offset);
    }
  }
  glBindVertexArray(kInvalidVertexAttributeHandle);
}

u32 ShaderHandler::GetInstanceIndex(const Shader* shader,
                                    const Material* material) const {
  return GetInstanceIndex(shader, material->instance_id);
}

u32 ShaderHandler::GetInstanceIndex(
    const Shader* shader, const MaterialInstanceId instance_id) const {
  auto& instance_ids{shader->instances.ids};
  auto it{std::find(instance_ids.begin(), instance_ids.end(), instance_id)};
  COMET_ASSERT(it != instance_ids.end(),
               "Unable to find bound material instance (ID: ", instance_id,
               ") in shader ", COMET_STRING_ID_LABEL(shader->id), "!");
  return static_cast<u32>(it - instance_ids.begin());
}

void ShaderHandler::HandleProgram(
    Shader* shader, const resource::ShaderResource* shader_resource) {
  HandleShaderModulesGeneration(shader, shader_resource);
  HandleAttributesGeneration(shader, shader_resource);
  HandleProgramGeneration(shader);
}

void ShaderHandler::HandleProgramGeneration(Shader* shader) const {
  auto is_graphics{false};
  auto is_compute{false};

  for (const auto& module : shader->modules) {
    if (module->type == GL_COMPUTE_SHADER) {
      is_compute = true;
    } else {
      is_graphics = true;
    }

    if (is_compute && is_graphics) {
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
  auto handle{bind_type == ShaderBindType::Compute ? shader->compute_handle
                                                   : shader->graphics_handle};

  for (auto& shader_module : shader->modules) {
    if (shader_module->bind_type == bind_type) {
      shader_module_handler_->Attach(shader, shader_module->handle);
    }
  }

  glLinkProgram(handle);

#ifdef COMET_DEBUG
  auto result{GL_FALSE};
  GLsizei info_log_len{0};

  // Check the program.
  glGetProgramiv(handle, GL_LINK_STATUS, &result);
  glGetProgramiv(handle, GL_INFO_LOG_LENGTH, &info_log_len);

  if (info_log_len > 0) {
    frame::FrameArray<GLchar> error_message{};
    error_message.Resize(info_log_len + 1);
    glGetProgramInfoLog(handle, info_log_len, nullptr, &error_message[0]);
    COMET_ASSERT(false, "Error while creating the shader program: ",
                 error_message.GetData());
  }
#endif  // COMET_DEBUG

  for (const auto& shader_module : shader->modules) {
    if (shader_module->bind_type == bind_type) {
      shader_module_handler_->Detach(shader, shader_module->handle);
    }
  }
}

void ShaderHandler::HandleShaderModulesGeneration(
    Shader* shader, const resource::ShaderResource* resource) const {
  shader->modules.Reserve(resource->descr.shader_module_paths.GetSize());

  for (const auto& shader_module_path : resource->descr.shader_module_paths) {
    const auto* shader_module{
        shader_module_handler_->GetOrGenerate(shader_module_path)};
    shader->modules.PushBack(shader_module);
  }
}

void ShaderHandler::HandleAttributesGeneration(
    Shader* shader, const resource::ShaderResource* resource) const {
  COMET_ASSERT(shader->vertex_attribute_handle == kInvalidVertexAttributeHandle,
               "Vertex attributes already generated!");
  glGenVertexArrays(1, &shader->vertex_attribute_handle);
  glBindVertexArray(shader->vertex_attribute_handle);

  mesh_handler_->Bind();
  GLsizei stride{0};

  for (auto& descr : resource->descr.vertex_attributes) {
    stride += GetGlVertexAttributeSize(descr.type);
  }

  GLsizeiptr offset{0};
  auto vertex_attribute_count{
      static_cast<GLuint>(resource->descr.vertex_attributes.GetSize())};
  shader->vertex_attributes.Reserve(vertex_attribute_count);

  for (GLuint i{0}; i < vertex_attribute_count; ++i) {
    const auto& descr{resource->descr.vertex_attributes[i]};

    auto component_count{GetGlComponentCount(descr.type)};
    auto type{GetGlVertexAttributeType(descr.type)};

    glEnableVertexAttribArray(i);

    if (IsIntegerVertexAttributeType(type)) {
      glVertexAttribIPointer(i, component_count, type, stride,
                             reinterpret_cast<const void*>(offset));
    } else {
      auto is_normalized{static_cast<GLboolean>(GL_FALSE)};
      glVertexAttribPointer(i, component_count, type, is_normalized, stride,
                            reinterpret_cast<const void*>(offset));
    }

    offset += GetGlVertexAttributeSize(descr.type);
  }

  RebindVertexAttributes(shader);

  glBindVertexArray(kInvalidVertexAttributeHandle);
  glBindBuffer(GL_ARRAY_BUFFER, kInvalidStorageHandle);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, kInvalidStorageHandle);
}

void ShaderHandler::HandleUniformsGeneration(
    Shader* shader, const resource::ShaderResource* resource) const {
  if (shader->graphics_handle != kInvalidShaderHandle) {
    shader->global_ubo_data.uniform_block_index =
        glGetUniformBlockIndex(shader->graphics_handle, "GlobalUbo");
    shader->instance_ubo_data.uniform_block_index =
        glGetUniformBlockIndex(shader->graphics_handle, "LocalUbo");
  }

  u32 instance_texture_count{0};

  for (const auto& uniform_descr : resource->descr.uniforms) {
    if (uniform_descr.type == ShaderVariableType::Sampler) {
      HandleSamplerGeneration(shader, uniform_descr, instance_texture_count);
      continue;
    }

    AddUniform(shader, uniform_descr, 0);
  }
}

void ShaderHandler::HandleConstantsGeneration(
    Shader* shader, const resource::ShaderResource* resource) const {
  for (const auto& constant_descr : resource->descr.constants) {
    AddConstant(shader, constant_descr);
  }
}

void ShaderHandler::HandleStorageGeneration(
    Shader* shader, const resource::ShaderResource* resource) const {
  for (const auto& storage_descr : resource->descr.storages) {
    if (!IsEmpty(storage_descr.engine_define,
                 storage_descr.engine_define_len) &&
        !resource::IsShaderEngineDefineSet(storage_descr.engine_define,
                                           storage_descr.engine_define_len)) {
      continue;
    }

    AddStorage(shader, storage_descr);
  }
}

void ShaderHandler::HandleBindingsGeneration(Shader* shader) const {
  shader->global_ubo_data.uniform_count = 0;
  shader->global_ubo_data.sampler_count = 0;
  shader->instance_ubo_data.uniform_count = 0;
  shader->instance_ubo_data.sampler_count = 0;
  shader->storage_data.count = 0;

  for (const auto& uniform : shader->uniforms) {
    if (uniform.scope == ShaderUniformScope::Global) {
      if (uniform.type == ShaderVariableType::Sampler) {
        ++shader->global_ubo_data.sampler_count;
      } else {
        ++shader->global_ubo_data.uniform_count;
      }
    } else if (uniform.scope == ShaderUniformScope::Instance) {
      if (uniform.type == ShaderVariableType::Sampler) {
        ++shader->instance_ubo_data.sampler_count;
      } else {
        ++shader->instance_ubo_data.uniform_count;
      }
    } else {
      COMET_ASSERT(false, "Unknown or unsupported uniform scope: ",
                   static_cast<std::underlying_type_t<ShaderUniformScope>>(
                       uniform.scope),
                   "!");
    }
  }

  ShaderUniformBufferIndex uniform_bufer_index{0};

  if (shader->global_ubo_data.uniform_count > 0 ||
      shader->global_ubo_data.sampler_count > 0) {
    shader->uniform_buffer_indices.global = uniform_bufer_index++;
  }

  if (shader->instance_ubo_data.uniform_count > 0 ||
      shader->instance_ubo_data.sampler_count > 0) {
    shader->uniform_buffer_indices.instance = uniform_bufer_index++;
  }
}

void ShaderHandler::HandleSamplerGeneration(
    Shader* shader, const ShaderUniformDescr& uniform_descr,
    u32& instance_texture_count) const {
  auto location{kInvalidShaderUniformLocation};

  if (uniform_descr.scope == ShaderUniformScope::Global) {
    COMET_ASSERT(shader->global_uniform_data.texture_maps.GetSize() ==
                     kMaxShaderTextureMapCount,
                 "Max texture map count reached: ", kMaxShaderTextureMapCount,
                 " for shader ", COMET_STRING_ID_LABEL(shader->id), "!");

    location = static_cast<ShaderUniformLocation>(
        shader->global_uniform_data.texture_maps.GetSize() - 1);
    shader->global_uniform_data.texture_maps.Resize(
        shader->global_uniform_data.texture_maps.GetSize() + 1);
  } else {
    COMET_ASSERT(instance_texture_count <= kMaxShaderTextureMapCount,
                 "Max texture map count reached: ", kMaxShaderTextureMapCount,
                 " for shader ", COMET_STRING_ID_LABEL(shader->id), "!");
    location = static_cast<ShaderUniformLocation>(instance_texture_count++);
  }

  COMET_ASSERT(location != kInvalidShaderUniformLocation,
               "Location found is invalid for shader ",
               COMET_STRING_ID_LABEL(shader->id), "!");
  AddUniform(shader, uniform_descr, location);
}

ShaderUniformIndex ShaderHandler::HandleUniformIndex(
    Shader* shader, const ShaderUniformDescr& uniform_descr) const {
  ShaderUniformIndex* index{nullptr};

  if (AreStringsEqual(uniform_descr.name, uniform_descr.name_len,
                      kUniformNameProjection.data(),
                      kUniformNameProjection.size())) {
    index = &shader->uniform_indices.projection;
  } else if (AreStringsEqual(uniform_descr.name, uniform_descr.name_len,
                             kUniformNameView.data(),
                             kUniformNameView.size())) {
    index = &shader->uniform_indices.view;
  } else if (AreStringsEqual(uniform_descr.name, uniform_descr.name_len,
                             kUniformNameAmbientColor.data(),
                             kUniformNameAmbientColor.size())) {
    index = &shader->uniform_indices.ambient_color;
  } else if (AreStringsEqual(uniform_descr.name, uniform_descr.name_len,
                             kUniformNameViewPos.data(),
                             kUniformNameViewPos.size())) {
    index = &shader->uniform_indices.view_pos;
  } else if (AreStringsEqual(uniform_descr.name, uniform_descr.name_len,
                             kUniformNameDiffuseColor.data(),
                             kUniformNameDiffuseColor.size())) {
    index = &shader->uniform_indices.diffuse_color;
  } else if (AreStringsEqual(uniform_descr.name, uniform_descr.name_len,
                             kUniformNameDiffuseMap.data(),
                             kUniformNameDiffuseMap.size())) {
    index = &shader->uniform_indices.diffuse_map;
  } else if (AreStringsEqual(uniform_descr.name, uniform_descr.name_len,
                             kUniformNameSpecularMap.data(),
                             kUniformNameSpecularMap.size())) {
    index = &shader->uniform_indices.specular_map;
  } else if (AreStringsEqual(uniform_descr.name, uniform_descr.name_len,
                             kUniformNameNormalMap.data(),
                             kUniformNameNormalMap.size())) {
    index = &shader->uniform_indices.normal_map;
  } else if (AreStringsEqual(uniform_descr.name, uniform_descr.name_len,
                             kUniformNameShininess.data(),
                             kUniformNameShininess.size())) {
    index = &shader->uniform_indices.shininess;
  }

  COMET_ASSERT(index != nullptr, "Unable to find uniform index with name \"",
               uniform_descr.name, "\"!");
  COMET_ASSERT(*index == kInvalidShaderUniformIndex,
               "Uniform index with name \"", uniform_descr.name,
               "\" was already bound!");
  *index = static_cast<ShaderUniformIndex>(shader->uniforms.GetSize());
  return *index;
}

ShaderConstantIndex ShaderHandler::HandleConstantIndex(
    Shader* shader, const ShaderConstantDescr& constant_descr) const {
  ShaderConstantIndex* index{nullptr};

  if (AreStringsEqual(constant_descr.name, constant_descr.name_len,
                      kConstantNameDrawCount.data(),
                      kConstantNameDrawCount.size())) {
    index = &shader->constant_indices.draw_count;
  } else if (AreStringsEqual(constant_descr.name, constant_descr.name_len,
                             kConstantNameCount.data(),
                             kConstantNameCount.size())) {
    index = &shader->constant_indices.count;
  }

  COMET_ASSERT(index != nullptr, "Unable to find constant index with name \"",
               constant_descr.name, "\"!");
  COMET_ASSERT(*index == kInvalidShaderConstantIndex,
               "Uniform index with name \"", constant_descr.name,
               "\" was already bound!");
  *index = static_cast<ShaderUniformIndex>(shader->constants.GetSize());
  return *index;
}

void ShaderHandler::HandleStorageIndexAndBinding(
    Shader* shader, const ShaderStorageDescr& storage_descr,
    ShaderStorage& storage) const {
  ShaderStorageIndex* index{nullptr};

  if (AreStringsEqual(storage_descr.name, storage_descr.name_len,
                      kStorageNameProxyLocalDatas.data(),
                      kStorageNameProxyLocalDatas.size())) {
    index = &shader->storage_indices.proxy_local_datas;
    storage.binding = kStorageBindingProxyLocalDatas;
  } else if (AreStringsEqual(storage_descr.name, storage_descr.name_len,
                             kStorageNameProxyIds.data(),
                             kStorageNameProxyIds.size())) {
    index = &shader->storage_indices.proxy_ids;
    storage.binding = kStorageBindingProxyIds;
  } else if (AreStringsEqual(storage_descr.name, storage_descr.name_len,
                             kStorageNameProxyInstances.data(),
                             kStorageNameProxyInstances.size())) {
    index = &shader->storage_indices.proxy_instances;
    storage.binding = kStorageBindingProxyInstances;
  } else if (AreStringsEqual(storage_descr.name, storage_descr.name_len,
                             kStorageNameIndirectProxies.data(),
                             kStorageNameIndirectProxies.size())) {
    index = &shader->storage_indices.indirect_proxies;
    storage.binding = kStorageBindingIndirectProxies;
  } else if (AreStringsEqual(storage_descr.name, storage_descr.name_len,
                             kStorageNameWordIndices.data(),
                             kStorageNameWordIndices.size())) {
    index = &shader->storage_indices.word_indices;
    storage.binding = kStorageBindingWordIndices;
  } else if (AreStringsEqual(storage_descr.name, storage_descr.name_len,
                             kStorageNameSourceWords.data(),
                             kStorageNameSourceWords.size())) {
    index = &shader->storage_indices.source_words;
    storage.binding = kStorageBindingSourceWords;
  } else if (AreStringsEqual(storage_descr.name, storage_descr.name_len,
                             kStorageNameDestinationWords.data(),
                             kStorageNameDestinationWords.size())) {
    index = &shader->storage_indices.destination_words;
    storage.binding = kStorageBindingDestinationWords;
  } else if (AreStringsEqual(storage_descr.name, storage_descr.name_len,
                             kStorageNameMatrixPalettes.data(),
                             kStorageNameMatrixPalettes.size())) {
    index = &shader->storage_indices.matrix_palettes;
    storage.binding = kStorageBindingMatrixPalettes;
  }
#ifdef COMET_DEBUG_RENDERING
  else if (AreStringsEqual(storage_descr.name, storage_descr.name_len,
                           kStorageNameDebugData.data(),
                           kStorageNameDebugData.size())) {
    index = &shader->storage_indices.debug_data;
    storage.binding = kStorageBindingDebugData;
  }
#endif  // COMET_DEBUG_RENDERING
#ifdef COMET_DEBUG_CULLING
  else if (AreStringsEqual(storage_descr.name, storage_descr.name_len,
                           kStorageNameDebugAabbs.data(),
                           kStorageNameDebugAabbs.size())) {
    index = &shader->storage_indices.debug_aabbs;
    storage.binding = kStorageBindingDebugAabbs;
  } else if (AreStringsEqual(storage_descr.name, storage_descr.name_len,
                             kStorageNameLineVertices.data(),
                             kStorageNameLineVertices.size())) {
    index = &shader->storage_indices.line_vertices;
    storage.binding = kStorageBindingLineVertices;
  }
#endif  // COMET_DEBUG_CULLING

  COMET_ASSERT(index != nullptr, "Unable to find storage index with name \"",
               storage_descr.name, "\"!");
  COMET_ASSERT(*index == kInvalidShaderStorageIndex,
               "Storage index with name \"", storage_descr.name,
               "\" was already bound!");
  *index = static_cast<ShaderStorageIndex>(shader->storages.GetSize() - 1);
  storage.index = *index;
}

void ShaderHandler::HandleUboBufferGeneration(Shader* shader) const {
  s32 align;
  glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &align);
  shader->global_ubo_data.ubo_stride = memory::AlignSize(
      shader->global_ubo_data.ubo_size, static_cast<memory::Alignment>(align));
  shader->instance_ubo_data.ubo_stride =
      memory::AlignSize(shader->instance_ubo_data.ubo_size,
                        static_cast<memory::Alignment>(align));
  auto buffer_size{static_cast<GLsizeiptr>(
      shader->global_ubo_data.ubo_stride +
      shader->instance_ubo_data.ubo_stride * kMaxMaterialInstances)};

#ifdef COMET_DEBUG
  s32 max_range;
  glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &max_range);
  COMET_ASSERT(
      shader->global_ubo_data.ubo_stride < static_cast<sptrdiff>(max_range),
      "Shader global UBO data is too big!");
  COMET_ASSERT(
      shader->instance_ubo_data.ubo_stride < static_cast<sptrdiff>(max_range),
      "Shader instance UBO data is too big!");
#endif  // COMET_DEBUG

  if (buffer_size > 0) {
    UniformBufferHandle uniform_buffer_handle;

    glGenBuffers(1, &uniform_buffer_handle);
    glBindBuffer(GL_UNIFORM_BUFFER, uniform_buffer_handle);
    COMET_GL_SET_UNIFORM_BUFFER_DEBUG_LABEL(uniform_buffer_handle,
                                            "uniform_buffer_handle");
    glBufferData(GL_UNIFORM_BUFFER, buffer_size, nullptr, GL_DYNAMIC_COPY);
    glBindBuffer(GL_UNIFORM_BUFFER, kInvalidUniformBufferHandle);
    shader->uniform_buffer_handle = uniform_buffer_handle;
  }
}

void ShaderHandler::AddUniform(Shader* shader, const ShaderUniformDescr& descr,
                               ShaderUniformLocation location) const {
  COMET_ASSERT(shader->uniforms.GetSize() + 1 <= kMaxShaderUniformCount,
               "Too many uniforms to be added to shader ",
               COMET_STRING_ID_LABEL(shader->id), "! Max uniform count is ",
               kMaxShaderUniformCount, ".");
  ShaderUniform uniform{};
  uniform.scope = descr.scope;
  uniform.type = descr.type;
  uniform.index = HandleUniformIndex(shader, descr);
  auto is_sampler{uniform.type == ShaderVariableType::Sampler};

  if (is_sampler) {
    uniform.location = location;
  } else {
    uniform.location = uniform.index;
  }

  auto size{GetShaderVariableTypeSize(uniform.type)};

  uniform.offset =
      is_sampler ? 0
      : uniform.scope == ShaderUniformScope::Global
          ? static_cast<ShaderOffset>(shader->global_ubo_data.ubo_size)
          : static_cast<ShaderOffset>(shader->instance_ubo_data.ubo_size);
  uniform.size = is_sampler ? 0
                            : static_cast<ShaderUniformSize>(memory::AlignSize(
                                  size, static_cast<memory::Alignment>(
                                            GetStd140Alignment(uniform.type))));

  shader->uniforms.PushBack(uniform);

  if (is_sampler) {
    if (uniform.scope == ShaderUniformScope::Global) {
      ++shader->global_ubo_data.sampler_count;
    } else if (uniform.scope == ShaderUniformScope::Instance) {
      ++shader->instance_ubo_data.sampler_count;
    }
  } else {
    if (uniform.scope == ShaderUniformScope::Global) {
      shader->global_ubo_data.ubo_size += uniform.size;
    } else if (uniform.scope == ShaderUniformScope::Instance) {
      shader->instance_ubo_data.ubo_size += uniform.size;
    }

    uniform.location = uniform.scope == ShaderUniformScope::Global
                           ? static_cast<ShaderUniformLocation>(
                                 shader->global_ubo_data.uniform_block_index)
                           : static_cast<ShaderUniformLocation>(
                                 shader->instance_ubo_data.uniform_block_index);
  }
}

void ShaderHandler::AddConstant(Shader* shader,
                                const ShaderConstantDescr& descr) const {
  COMET_ASSERT(shader->constants.GetSize() + 1 <= kMaxShaderUniformCount,
               "Too many constants to be added to shader ",
               COMET_STRING_ID_LABEL(shader->id), "! Max constant count is ",
               kMaxShaderConstantCount, ".");
  ShaderConstant constant{};
  constant.type = descr.type;
  constant.index = HandleConstantIndex(shader, descr);

  auto size{GetShaderVariableTypeSize(constant.type)};

  if (descr.stages == ShaderStageFlagBits::kShaderStageFlagBitsCompute) {
    constant.location = static_cast<ShaderConstantLocation>(
        glGetUniformLocation(shader->compute_handle, descr.name));
  } else {
    constant.location = static_cast<ShaderConstantLocation>(
        glGetUniformLocation(shader->graphics_handle, descr.name));
  }

  constant.size = static_cast<ShaderUniformSize>(memory::AlignSize(
      size, static_cast<memory::Alignment>(GetStd430Alignment(constant.type))));

  shader->constants.PushBack(constant);
}
}  // namespace gl
}  // namespace rendering
}  // namespace comet