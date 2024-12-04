// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "opengl_shader_handler.h"

#include "comet/core/c_string.h"
#include "comet/core/memory/memory.h"
#include "comet/resource/resource_manager.h"
#include "comet/resource/shader_resource.h"

namespace comet {
namespace rendering {
namespace gl {
ShaderHandler::ShaderHandler(const ShaderHandlerDescr& descr)
    : Handler{descr},
      shader_module_handler_{descr.shader_module_handler},
      texture_handler_{descr.texture_handler} {
  COMET_ASSERT(shader_module_handler_ != nullptr,
               "Shader module handler is null!");
  COMET_ASSERT(texture_handler_ != nullptr, "Texture handler is null!");
}

void ShaderHandler::Shutdown() {
  std::vector<UniformBufferHandle> ubo_handles{};
  ubo_handles.reserve(shaders_.size());

  for (auto& it : shaders_) {
    auto& shader{it.second};

    if (shader.ubo_handle != kInvalidUniformBufferHandle) {
      ubo_handles.push_back(shader.ubo_handle);
    }

    Destroy(it.second, true);
  }

  if (ubo_handles.size() > 0) {
    glDeleteBuffers(static_cast<s32>(ubo_handles.size()), ubo_handles.data());
  }

  shaders_.clear();
  bound_shader_ = nullptr;
  instance_id_handler_.Shutdown();
  Handler::Shutdown();
}

Shader* ShaderHandler::Generate(const ShaderDescr& descr) {
  const auto* shader_resource{
      resource::ResourceManager::Get().Load<resource::ShaderResource>(
          descr.resource_path)};
  COMET_ASSERT(shader_resource != nullptr, "Shader resource is null!");
  COMET_ASSERT(shader_resource->descr.vertex_attributes.size() == 0,
               "Custom vertex attributes are not supported on OpenGL!");
  Shader shader{};
  shader.id = shader_resource->id;
  shader.is_wireframe = shader_resource->descr.is_wireframe;
  shader.cull_mode = GetCullMode(shader_resource->descr.cull_mode);
  shader.handle = glCreateProgram();
  shader.modules.reserve(shader_resource->descr.shader_module_paths.size());

  const auto& shader_module_paths{shader_resource->descr.shader_module_paths};

  for (const auto& module_path : shader_module_paths) {
    const auto* shader_module{
        shader_module_handler_->GetOrGenerate(module_path)};
    shader_module_handler_->Attach(shader, shader_module->handle);
    shader.modules.push_back(shader_module);
  }

  glLinkProgram(shader.handle);

#ifdef COMET_DEBUG
  auto result{GL_FALSE};
  GLsizei info_log_len{0};

  // Check the program.
  glGetProgramiv(shader.handle, GL_LINK_STATUS, &result);
  glGetProgramiv(shader.handle, GL_INFO_LOG_LENGTH, &info_log_len);

  if (info_log_len > 0) {
    std::vector<GLchar> error_message(info_log_len + 1);
    glGetProgramInfoLog(shader.handle, info_log_len, nullptr,
                        &error_message[0]);
    COMET_ASSERT(false, "Error while creating the shader program: ",
                 error_message.data());
  }
#endif  // COMET_DEBUG

  for (const auto& shader_module : shader.modules) {
    shader_module_handler_->Detach(shader, shader_module->handle);
  }

  HandleUniformsGeneration(shader, *shader_resource);
  HandleUniformCount(shader);
  HandleBufferGeneration(shader);

#ifdef COMET_DEBUG
  const auto shader_id{shader.id};
#endif  // COMET_DEBUG
  auto insert_pair{shaders_.emplace(shader.id, std::move(shader))};
  COMET_ASSERT(insert_pair.second,
               "Could not insert shader: ", COMET_STRING_ID_LABEL(shader_id),
               "!");
  return &insert_pair.first->second;
}

Shader* ShaderHandler::Get(ShaderId shader_id) {
  auto* shader{TryGet(shader_id)};
  COMET_ASSERT(shader != nullptr, "Requested shader does not exist: ",
               COMET_STRING_ID_LABEL(shader_id), "!");
  return shader;
}

Shader* ShaderHandler::TryGet(ShaderId shader_id) {
  auto it{shaders_.find(shader_id)};

  if (it == shaders_.end()) {
    return nullptr;
  }

  return &it->second;
}

void ShaderHandler::Destroy(ShaderId shader_id) { Destroy(*Get(shader_id)); }

void ShaderHandler::Destroy(Shader& shader) { Destroy(shader, false); }

void ShaderHandler::Bind(Shader& shader) {
  if (&shader == bound_shader_) {
    return;
  }

  glPolygonMode(GL_FRONT_AND_BACK, shader.is_wireframe ? GL_LINE : GL_FILL);
  glCullFace(shader.cull_mode);
  glUseProgram(shader.handle);
  bound_shader_ = &shader;
}

void ShaderHandler::BindGlobal(Shader& shader) const {
  shader.bound_ubo_offset = shader.global_ubo_data.ubo_offset;
  glBindBufferRange(GL_UNIFORM_BUFFER, kShaderDescriptorSetGlobalIndex,
                    shader.ubo_handle, shader.bound_ubo_offset,
                    shader.global_ubo_data.ubo_stride);
}

void ShaderHandler::BindInstance(Shader& shader,
                                 MaterialInstanceId instance_id) const {
  shader.bound_instance_index = GetInstanceIndex(shader, instance_id);
  shader.bound_ubo_offset =
      shader.global_ubo_data.ubo_stride +
      shader.instances.list[shader.bound_instance_index].offset;

  glBindBufferRange(GL_UNIFORM_BUFFER, kShaderDescriptorSetInstanceIndex,
                    shader.ubo_handle, shader.bound_ubo_offset,
                    shader.instance_ubo_data.ubo_stride);
}

void ShaderHandler::Reset() { bound_shader_ = nullptr; }

void ShaderHandler::UpdateGlobal(Shader& shader,
                                 const ShaderPacket& packet) const {
  if (shader.global_uniform_data.update_frame == packet.frame_count) {
    return;
  }

  BindGlobal(shader);
  SetUniform(shader, shader.uniform_indices.projection,
             packet.projection_matrix);
  SetUniform(shader, shader.uniform_indices.view, packet.view_matrix);

  shader.global_uniform_data.update_frame = packet.frame_count;
}

void ShaderHandler::UpdateLocal(Shader& shader,
                                const ShaderLocalPacket& packet) {
  SetUniform(shader, shader.uniform_indices.model, packet.position);
}

void ShaderHandler::SetUniform(Shader& shader, const ShaderUniform& uniform,
                               const void* value, bool is_update) const {
  if (uniform.type == ShaderUniformType::Sampler) {
    const TextureMap* texture_map;

    if (uniform.scope == ShaderUniformScope::Global) {
      texture_map =
          shader.global_uniform_data.texture_maps[uniform.data_index] =
              static_cast<const TextureMap*>(value);
    } else {
      texture_map = shader.instances.list[shader.bound_instance_index]
                        .uniform_data.texture_maps[uniform.data_index] =
          static_cast<const TextureMap*>(value);
    }

    glActiveTexture(GL_TEXTURE0 + texture_map->type);
    glBindTexture(GL_TEXTURE_2D, texture_map->texture_handle);

    if (is_update) {
      glUniform1i(uniform.location, texture_map->type);
    }

    return;
  }

  if (uniform.scope == ShaderUniformScope::Local) {
    COMET_ASSERT(
        kUniformCallbacks_.find(uniform.type) != kUniformCallbacks_.end(),
        "Unknown or unsupported shader uniform type: ",
        static_cast<std::underlying_type_t<ShaderUniformType>>(uniform.type),
        "!");

    if (is_update) {
      kUniformCallbacks_.at(uniform.type)(uniform.location, value);
    }

    return;
  }

  if (is_update) {
    glBindBuffer(GL_UNIFORM_BUFFER, shader.ubo_handle);
    glBufferSubData(GL_UNIFORM_BUFFER, shader.bound_ubo_offset + uniform.offset,
                    uniform.size, value);
    glBindBuffer(GL_UNIFORM_BUFFER, kInvalidUniformBufferHandle);
  }
}

void ShaderHandler::SetUniform(ShaderHandle handle,
                               const ShaderUniform& uniform, const void* value,
                               bool is_update) {
  auto* shader{Get(handle)};
  COMET_ASSERT(shader != nullptr, "Tried to retrieve shader pass from handle ",
               handle, ", but instance is null!");
  SetUniform(*shader, uniform, value, is_update);
}

void ShaderHandler::SetUniform(Shader& shader, ShaderUniformLocation index,
                               const void* value, bool is_update) const {
  COMET_ASSERT(
      index < shader.uniforms.size(), "Tried to set uniform at index #", index,
      " for shader pass, but uniform count is ", shader.uniforms.size(), "!");
  SetUniform(shader, shader.uniforms[index], value, is_update);
}

void ShaderHandler::SetUniform(ShaderHandle handle, ShaderUniformLocation index,
                               const void* value, bool is_update) {
  auto* shader{Get(handle)};
  COMET_ASSERT(shader != nullptr, "Tried to retrieve shader pass from handle ",
               handle, ", but instance is null!");
  SetUniform(*shader, index, value, is_update);
}

void ShaderHandler::BindMaterial(Material& material) {
  auto* shader{Get(material.shader_id)};

  MaterialInstance instance{};
  auto instance_texture_count{shader->instance_ubo_data.sampler_count};

  if (instance_texture_count > 0) {
    instance.uniform_data.texture_maps.resize(instance_texture_count);

    // TODO(m4jr0): Put texture maps in generic array.
    StaticArray<TextureMap*, 3> maps{
        &material.diffuse_map, &material.specular_map, &material.normal_map};
    memory::CopyMemory(instance.uniform_data.texture_maps.data(),
                       maps.GetData(), sizeof(TextureMap*) * maps.GetSize());
  }

  instance.offset =
      shader->instance_ubo_data.ubo_stride * shader->instances.list.size();

  // Generate material instance.
  shader->instances.list.push_back(std::move(instance));
  material.instance_id = instance_id_handler_.Generate();
  shader->instances.ids.push_back(material.instance_id);
}

void ShaderHandler::UnbindMaterial(Material& material) {
  COMET_ASSERT(HasMaterial(material),
               "Trying to destroy dead material instance #",
               material.instance_id, "!");

  auto* shader{Get(material.shader_id)};
  auto index{GetInstanceIndex(*shader, material)};
  auto& instances{shader->instances.list};

  auto& instance{instances[index]};
  instance.uniform_data.texture_maps.clear();

  // Delete material instance.
  // Copy last instance in place of the deleted one + Remove last duplicate in
  // the list.
  auto& instance_ids{shader->instances.ids};
  COMET_ASSERT(instance_ids.size() == instances.size(),
               "Size mismatch between material IDs (", instance_ids.size(),
               ") and instances (", instances.size(),
               ")! Something very wrong must have happened!");
  auto max_index{instance_ids.size() - 1};

  if (index != max_index) {
    instance_ids[index] = instance_ids[max_index];
    instances[index] = instances[max_index];
  }

  instance_ids.resize(max_index);
  instances.resize(max_index);
  instance_id_handler_.Destroy(material.instance_id);
  material.instance_id = kInvalidMaterialInstanceId;
}

bool ShaderHandler::HasMaterial(const Material& material) const {
  return instance_id_handler_.IsAlive(material.instance_id);
}

MaterialInstance& ShaderHandler::GetInstance(Shader& shader,
                                             const Material& material) {
  return shader.instances.list[GetInstanceIndex(shader, material)];
}

GLenum ShaderHandler::GetCullMode(CullMode cull_mode) {
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

ShaderUniformSize ShaderHandler::GetUniformSize(ShaderUniformType type) {
  switch (type) {
    case ShaderUniformType::B32:
    case ShaderUniformType::S32:
    case ShaderUniformType::U32:
    case ShaderUniformType::F32:
      return 4;

    case ShaderUniformType::F64:
      return 8;

    case ShaderUniformType::B32Vec2:
    case ShaderUniformType::S32Vec2:
    case ShaderUniformType::U32Vec2:
    case ShaderUniformType::Vec2:
      return 8;

    case ShaderUniformType::B32Vec3:
    case ShaderUniformType::S32Vec3:
    case ShaderUniformType::U32Vec3:
    case ShaderUniformType::Vec3:
      return 12;

    case ShaderUniformType::B32Vec4:
    case ShaderUniformType::S32Vec4:
    case ShaderUniformType::U32Vec4:
    case ShaderUniformType::Vec4:
      return 16;

    case ShaderUniformType::F64Vec2:
      return 16;

    case ShaderUniformType::F64Vec3:
      return 24;

    case ShaderUniformType::F64Vec4:
      return 32;

    case ShaderUniformType::Mat2x2:
      return 8;

    case ShaderUniformType::Mat2x3:
    case ShaderUniformType::Mat3x2:
      return 24;

    case ShaderUniformType::Mat3x3:
      return 36;

    case ShaderUniformType::Mat2x4:
    case ShaderUniformType::Mat4x2:
      return 32;

    case ShaderUniformType::Mat3x4:
    case ShaderUniformType::Mat4x3:
      return 48;

    case ShaderUniformType::Mat4x4:
      return 64;

    case ShaderUniformType::Sampler:
    case ShaderUniformType::Image:
    case ShaderUniformType::Atomic:
      return kInvalidShaderUniformSize;

    default:
      COMET_ASSERT(false, "Unknown or unsupported shader data type: ",
                   static_cast<std::underlying_type_t<ShaderUniformType>>(type),
                   "!");
  }

  return kInvalidShaderUniformSize;
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

void ShaderHandler::Destroy(Shader& shader, bool is_destroying_handler) {
  if (shader.handle != kInvalidShaderHandle) {
    glDeleteProgram(shader.handle);
    shader.handle = kInvalidShaderHandle;
  }

  if (!is_destroying_handler) {
    shaders_.erase(shader.id);

    if (shader.ubo_handle != kInvalidUniformBufferHandle) {
      glDeleteBuffers(1, &shader.ubo_handle);
      shader.ubo_handle = kInvalidUniformBufferHandle;
    }
  }

  shader.is_wireframe = false;
  shader.local_uniform_count = 0;
  shader.cull_mode = GL_INVALID_VALUE;
  shader.id = kInvalidShaderId;
  shader.bound_ubo_offset = 0;
  shader.bound_instance_index = kInvalidMaterialInstanceId;
  shader.global_uniform_data = {};
  shader.global_ubo_data = {};
  shader.instance_ubo_data = {};
  shader.instances = {};
  shader.uniforms.clear();
  shader.uniform_indices.projection = kInvalidShaderUniformIndex;
  shader.uniform_indices.view = kInvalidShaderUniformIndex;
  shader.uniform_indices.ambient_color = kInvalidShaderUniformIndex;
  shader.uniform_indices.view_pos = kInvalidShaderUniformIndex;
  shader.uniform_indices.diffuse_color = kInvalidShaderUniformIndex;
  shader.uniform_indices.diffuse_map = kInvalidShaderUniformIndex;
  shader.uniform_indices.specular_map = kInvalidShaderUniformIndex;
  shader.uniform_indices.normal_map = kInvalidShaderUniformIndex;
  shader.uniform_indices.shininess = kInvalidShaderUniformIndex;
  shader.uniform_indices.model = kInvalidShaderUniformIndex;

  if (shader_module_handler_->IsInitialized()) {
    for (auto& module : shader.modules) {
      shader_module_handler_->Destroy(module->handle);
    }
  }

  shader.modules.clear();
}

u32 ShaderHandler::GetInstanceIndex(const Shader& shader,
                                    const Material& material) const {
  return GetInstanceIndex(shader, material.instance_id);
}

u32 ShaderHandler::GetInstanceIndex(
    const Shader& shader, const MaterialInstanceId instance_id) const {
  auto& instance_ids{shader.instances.ids};
  auto it{std::find(instance_ids.begin(), instance_ids.end(), instance_id)};
  COMET_ASSERT(it != instance_ids.end(),
               "Unable to find bound material instance (ID: ", instance_id,
               ") in shader ", COMET_STRING_ID_LABEL(shader.id), "!");
  return static_cast<u32>(it - instance_ids.begin());
}

void ShaderHandler::HandleUniformsGeneration(
    Shader& shader, const resource::ShaderResource& resource) const {
  shader.global_ubo_data.uniform_block_index =
      glGetUniformBlockIndex(shader.handle, "global_uniform_object");
  shader.instance_ubo_data.uniform_block_index =
      glGetUniformBlockIndex(shader.handle, "local_uniform_object");

  u32 instance_texture_count{0};

  for (const auto& uniform_descr : resource.descr.uniforms) {
    if (uniform_descr.type == ShaderUniformType::Sampler) {
      HandleSamplerGeneration(shader, uniform_descr, instance_texture_count);
      continue;
    }

    AddUniform(shader, uniform_descr, 0);
  }
}

void ShaderHandler::HandleUniformCount(Shader& shader) const {
  shader.global_ubo_data.uniform_count = 0;
  shader.global_ubo_data.sampler_count = 0;
  shader.instance_ubo_data.uniform_count = 0;
  shader.instance_ubo_data.sampler_count = 0;

  for (const auto& uniform : shader.uniforms) {
    if (uniform.scope == ShaderUniformScope::Global) {
      if (uniform.type == ShaderUniformType::Sampler) {
        ++shader.global_ubo_data.sampler_count;
      } else {
        ++shader.global_ubo_data.uniform_count;
      }
    } else if (uniform.scope == ShaderUniformScope::Instance) {
      if (uniform.type == ShaderUniformType::Sampler) {
        ++shader.instance_ubo_data.sampler_count;
      } else {
        ++shader.instance_ubo_data.uniform_count;
      }
    } else if (uniform.scope == ShaderUniformScope::Local) {
      // TODO(m4jr0).
    } else {
      COMET_ASSERT(false, "Unknown or unsupported uniform scope: ",
                   static_cast<std::underlying_type_t<ShaderUniformScope>>(
                       uniform.scope),
                   "!");
    }
  }
}

ShaderUniformLocation GetSamplerLocation(
    const Shader& shader, const ShaderUniformDescr& uniform_descr,
    TextureType texture_type) {
  constexpr usize kBuffLen{32};
  static schar buff[kBuffLen];
  usize cursor;

  if (uniform_descr.scope == ShaderUniformScope::Global) {
    cursor = 15;
    memory::CopyMemory(buff, "globalSamplers[", cursor);
  } else if (uniform_descr.scope == ShaderUniformScope::Instance) {
    cursor = 12;
    memory::CopyMemory(buff, "texSamplers[", cursor);
  } else {
    COMET_ASSERT(false, "Unknown or unsupported sampler scope: ",
                 static_cast<std::underlying_type_t<ShaderUniformScope>>(
                     uniform_descr.scope),
                 "!");
    return kInvalidShaderUniformLocation;
  }

  usize copy_count;
  ConvertToStr(static_cast<s16>(texture_type), buff + cursor, kBuffLen - cursor,
               &copy_count);
  cursor += copy_count;
  buff[cursor++] = ']';

  FillWith(buff, kBuffLen, '\0', cursor);
  return glGetUniformLocation(shader.handle, buff);
}

void ShaderHandler::HandleSamplerGeneration(
    Shader& shader, const ShaderUniformDescr& uniform_descr,
    u32& instance_texture_count) const {
  COMET_ASSERT(uniform_descr.scope != ShaderUniformScope::Local,
               "Samplers cannot be at a local scope!");
  auto location{kInvalidShaderUniformLocation};

  if (uniform_descr.scope == ShaderUniformScope::Global) {
    COMET_ASSERT(shader.global_uniform_data.texture_maps.size() ==
                     kMaxShaderTextureMapCount,
                 "Max texture map count reached: ", kMaxShaderTextureMapCount,
                 " for shader ", COMET_STRING_ID_LABEL(shader.id), "!");

    location = static_cast<ShaderUniformLocation>(
        shader.global_uniform_data.texture_maps.size() - 1);
    shader.global_uniform_data.texture_maps.resize(
        shader.global_uniform_data.texture_maps.size() + 1);
  } else {
    COMET_ASSERT(instance_texture_count <= kMaxShaderTextureMapCount,
                 "Max texture map count reached: ", kMaxShaderTextureMapCount,
                 " for shader ", COMET_STRING_ID_LABEL(shader.id), "!");
    location = static_cast<ShaderUniformLocation>(instance_texture_count++);
  }

  COMET_ASSERT(location != kInvalidShaderUniformLocation,
               "Location found is invalid for shader ",
               COMET_STRING_ID_LABEL(shader.id), "!");
  AddUniform(shader, uniform_descr, location);
}

ShaderUniformIndex ShaderHandler::HandleUniformIndex(
    Shader& shader, const ShaderUniformDescr& uniform_descr) const {
  ShaderUniformIndex* index{nullptr};

  if (AreStringsEqual(uniform_descr.name, uniform_descr.name_len, "projection",
                      10)) {
    index = &shader.uniform_indices.projection;
  } else if (AreStringsEqual(uniform_descr.name, uniform_descr.name_len, "view",
                             4)) {
    index = &shader.uniform_indices.view;
  } else if (AreStringsEqual(uniform_descr.name, uniform_descr.name_len,
                             "ambientColor", 12)) {
    index = &shader.uniform_indices.ambient_color;
  } else if (AreStringsEqual(uniform_descr.name, uniform_descr.name_len,
                             "viewPos", 7)) {
    index = &shader.uniform_indices.view_pos;
  } else if (AreStringsEqual(uniform_descr.name, uniform_descr.name_len,
                             "diffuseColor", 12)) {
    index = &shader.uniform_indices.diffuse_color;
  } else if (AreStringsEqual(uniform_descr.name, uniform_descr.name_len,
                             "diffuseMap", 10)) {
    index = &shader.uniform_indices.diffuse_map;
  } else if (AreStringsEqual(uniform_descr.name, uniform_descr.name_len,
                             "specularMap", 11)) {
    index = &shader.uniform_indices.specular_map;
  } else if (AreStringsEqual(uniform_descr.name, uniform_descr.name_len,
                             "normalMap", 9)) {
    index = &shader.uniform_indices.normal_map;
  } else if (AreStringsEqual(uniform_descr.name, uniform_descr.name_len,
                             "shininess", 9)) {
    index = &shader.uniform_indices.shininess;
  } else if (AreStringsEqual(uniform_descr.name, uniform_descr.name_len,
                             "model", 5)) {
    index = &shader.uniform_indices.model;
  }

  COMET_ASSERT(index != nullptr, "Unable to find uniform location with name \"",
               uniform_descr.name, "\"!");
  COMET_ASSERT(*index == kInvalidShaderUniformIndex,
               "Uniform location with name \"", uniform_descr.name,
               "\" was already bound!");
  *index = static_cast<ShaderUniformIndex>(shader.uniforms.size());
  return *index;
}

void ShaderHandler::HandleBufferGeneration(Shader& shader) const {
  s32 align;
  glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &align);
  shader.global_ubo_data.ubo_stride = memory::AlignSize(
      shader.global_ubo_data.ubo_size, static_cast<memory::Alignment>(align));
  shader.instance_ubo_data.ubo_stride = memory::AlignSize(
      shader.instance_ubo_data.ubo_size, static_cast<memory::Alignment>(align));
  auto buffer_size{static_cast<GLsizeiptr>(shader.global_ubo_data.ubo_stride +
                                           shader.instance_ubo_data.ubo_stride *
                                               kMaxMaterialInstances)};

#ifdef COMET_DEBUG
  s32 max_range;
  glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &max_range);
  COMET_ASSERT(shader.global_ubo_data.ubo_stride < max_range,
               "Shader global UBO data is too big!");
  COMET_ASSERT(shader.instance_ubo_data.ubo_stride < max_range,
               "Shader instance UBO data is too big!");
#endif  // COMET_DEBUG

  glGenBuffers(1, &shader.ubo_handle);
  glBindBuffer(GL_UNIFORM_BUFFER, shader.ubo_handle);
  glBufferData(GL_UNIFORM_BUFFER, buffer_size, nullptr, GL_DYNAMIC_COPY);
}

void ShaderHandler::AddUniform(Shader& shader, const ShaderUniformDescr& descr,
                               usize data_index) const {
  COMET_ASSERT(shader.uniforms.size() + 1 <= kMaxShaderUniformCount,
               "Too many uniforms to be added to shader ",
               COMET_STRING_ID_LABEL(shader.id), "! Max uniform count is ",
               kMaxShaderUniformCount, ".");
  ShaderUniform uniform{};
  uniform.scope = descr.scope;
  uniform.type = descr.type;
  uniform.index = HandleUniformIndex(shader, descr);
  auto is_sampler{uniform.type == ShaderUniformType::Sampler};

  if (is_sampler) {
    uniform.data_index = data_index;
    uniform.location = GetSamplerLocation(
        shader, descr, static_cast<TextureType>(uniform.data_index));
    ;
  } else {
    if (uniform.scope == ShaderUniformScope::Local) {
      uniform.location = glGetUniformLocation(shader.handle, descr.name);
    } else {
      uniform.location = uniform.scope == ShaderUniformScope::Global
                             ? shader.global_ubo_data.uniform_block_index
                             : shader.instance_ubo_data.uniform_block_index;
    }
  }

  auto size{GetUniformSize(uniform.type)};

  if (uniform.scope == ShaderUniformScope::Local) {
    uniform.offset = shader.local_uniform_count++;
    uniform.size = size;
  } else {
    uniform.offset = is_sampler ? 0
                     : uniform.scope == ShaderUniformScope::Global
                         ? shader.global_ubo_data.ubo_size
                         : shader.instance_ubo_data.ubo_size;
    uniform.size = is_sampler
                       ? 0
                       : static_cast<ShaderUniformSize>(memory::AlignSize(
                             size, static_cast<memory::Alignment>(
                                       GetStd140Alignment(uniform.type))));
  }

  shader.uniforms.push_back(uniform);

  if (!is_sampler) {
    if (uniform.scope == ShaderUniformScope::Global) {
      shader.global_ubo_data.ubo_size += uniform.size;
    } else if (uniform.scope == ShaderUniformScope::Instance) {
      shader.instance_ubo_data.ubo_size += uniform.size;
    }
  }
}
}  // namespace gl
}  // namespace rendering
}  // namespace comet