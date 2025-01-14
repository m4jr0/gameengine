// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "comet_pch.h"

#include "opengl_shader_module_handler.h"

#include "comet/core/generator.h"
#include "comet/resource/resource_manager.h"

namespace comet {
namespace rendering {
namespace gl {
ShaderModuleHandler::ShaderModuleHandler(const ShaderModuleHandlerDescr& descr)
    : Handler{descr} {}

void ShaderModuleHandler::Shutdown() {
  for (auto& it : shader_modules_) {
    Destroy(it.second, true);
  }

  shader_modules_.clear();
  Handler::Shutdown();
}

const ShaderModule* ShaderModuleHandler::Generate(
    CTStringView shader_module_path) {
  const auto* resource{
      resource::ResourceManager::Get().Load<resource::ShaderModuleResource>(
          shader_module_path)};

  auto shader_module{CompileShader(resource)};

#ifdef COMET_DEBUG
  const auto shader_module_id{shader_module.handle};
#endif  // COMET_DEBUG

  auto insert_pair{
      shader_modules_.emplace(shader_module.handle, shader_module)};
  COMET_ASSERT(insert_pair.second, "Could not insert shader module: ",
               COMET_STRING_ID_LABEL(shader_module_id), "!");
  return &insert_pair.first->second;
}

const ShaderModule* ShaderModuleHandler::Get(
    ShaderModuleHandle shader_module_id) const {
  auto* shader_module{TryGet(shader_module_id)};
  COMET_ASSERT(shader_module != nullptr,
               "Requested shader module does not exist: ",
               COMET_STRING_ID_LABEL(shader_module_id), "!");
  return shader_module;
}

const ShaderModule* ShaderModuleHandler::TryGet(
    ShaderModuleHandle shader_module_id) const {
  const auto it{shader_modules_.find(shader_module_id)};

  if (it == shader_modules_.end()) {
    return nullptr;
  }

  return &it->second;
}

const ShaderModule* ShaderModuleHandler::GetOrGenerate(CTStringView path) {
  const auto* shader_module{TryGet(COMET_STRING_ID(path))};

  if (shader_module != nullptr) {
    return shader_module;
  }

  return Generate(path);
}

void ShaderModuleHandler::Destroy(ShaderModuleHandle shader_module_id) {
  Destroy(*Get(shader_module_id), false);
}

void ShaderModuleHandler::Destroy(ShaderModule& shader_module) {
  Destroy(shader_module, false);
}

void ShaderModuleHandler::Attach(const Shader& shader,
                                 ShaderModuleHandle shader_module_id) {
  Attach(shader, *Get(shader_module_id));
}

void ShaderModuleHandler::Attach(const Shader& shader,
                                 ShaderModule& shader_module) {
  glAttachShader(shader.handle, shader_module.handle);
  ++shader_module.ref_count;
}

void ShaderModuleHandler::Detach(const Shader& shader,
                                 ShaderModuleHandle shader_module_id) {
  Detach(shader, *Get(shader_module_id));
}

void ShaderModuleHandler::Detach(const Shader& shader,
                                 ShaderModule& shader_module) {
  COMET_ASSERT(shader_module.ref_count > 0,
               "Tried to detach shader module from shader, but reference count "
               "is already 0!");
  glDetachShader(shader.handle, shader_module.handle);
  --shader_module.ref_count;
}

GLenum ShaderModuleHandler::GetOpenGlType(ShaderModuleType module_type) {
  switch (module_type) {
    case ShaderModuleType::Vertex:
      return GL_VERTEX_SHADER;
    case ShaderModuleType::Fragment:
      return GL_FRAGMENT_SHADER;
    default:
      COMET_ASSERT(
          false, "Unknown shader module type: ",
          static_cast<std::underlying_type_t<ShaderModuleType>>(module_type),
          "!");
  }

  return GL_INVALID_VALUE;
}

ShaderModule* ShaderModuleHandler::Get(ShaderModuleHandle shader_module_id) {
  auto* shader_module{TryGet(shader_module_id)};
  COMET_ASSERT(shader_module != nullptr,
               "Requested shader module does not exist: ",
               COMET_STRING_ID_LABEL(shader_module_id), "!");
  return shader_module;
}

ShaderModule* ShaderModuleHandler::TryGet(ShaderModuleHandle shader_module_id) {
  auto it{shader_modules_.find(shader_module_id)};

  if (it == shader_modules_.end()) {
    return nullptr;
  }

  return &it->second;
}

void ShaderModuleHandler::Destroy(ShaderModule& shader_module,
                                  bool is_destroying_handler) {
  COMET_ASSERT(shader_module.ref_count == 0,
               "Tried to destroy shader module, but it is still used!");

  if (shader_module.handle != kInvalidShaderModuleHandle) {
    glDeleteShader(shader_module.handle);
    shader_module.handle = kInvalidShaderModuleHandle;
  }

  if (!is_destroying_handler) {
    shader_modules_.erase(shader_module.handle);
  }

  shader_module.type = GL_INVALID_VALUE;
}

ShaderModule ShaderModuleHandler::CompileShader(
    const resource::ShaderModuleResource* resource) const {
  const auto* code{reinterpret_cast<const schar*>(resource->data.GetData())};
  const auto code_size{static_cast<s32>(resource->data.GetSize())};

  ShaderModule shader_module{};
  shader_module.type = GetOpenGlType(resource->descr.shader_type);
  shader_module.ref_count = 0;

  // Compile shader.
  shader_module.handle = glCreateShader(shader_module.type);
  glShaderSource(shader_module.handle, 1, &code, &code_size);
  glCompileShader(shader_module.handle);

  auto result{GL_FALSE};
  GLsizei msg_len{0};

  // Check shader.
  glGetShaderiv(shader_module.handle, GL_COMPILE_STATUS, &result);
  glGetShaderiv(shader_module.handle, GL_INFO_LOG_LENGTH, &msg_len);

  if (msg_len > 0) {
    auto* error_message{
        GenerateForOneFrame<schar>(static_cast<usize>(msg_len))};
    glGetShaderInfoLog(shader_module.handle, msg_len, nullptr, error_message);
    COMET_ASSERT(false, "Error while compiling shader module: ", error_message);
  }

  if (result == GL_FALSE) {
    COMET_LOG_RENDERING_ERROR("Unknown error while compiling shader module!");
  }

  return shader_module;
}
}  // namespace gl
}  // namespace rendering
}  // namespace comet
