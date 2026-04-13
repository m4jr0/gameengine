// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "opengl_shader_module_handler.h"
////////////////////////////////////////////////////////////////////////////////

// External. ///////////////////////////////////////////////////////////////////
#include <type_traits>
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/generator.h"
#include "comet/core/memory/allocator/allocator.h"
#include "comet/resource/resource_manager.h"

namespace comet {
namespace rendering {
namespace gl {
ShaderModuleHandler::ShaderModuleHandler(const ShaderModuleHandlerDescr& descr)
    : Handler{descr} {}

void ShaderModuleHandler::Initialize() {
  Handler::Initialize();
  allocator_.Initialize();
  shader_modules_ = Map<ShaderModuleId, ShaderModule*>{&allocator_};
}

void ShaderModuleHandler::Shutdown() {
  for (auto& it : shader_modules_) {
    Destroy(it.value, true);
  }

  shader_modules_.Destroy();
  allocator_.Destroy();
  Handler::Shutdown();
}

const ShaderModule* ShaderModuleHandler::Generate(
    CTStringView shader_module_path) {
  auto* shader_module_resource{
      resource::ResourceManager::Get().GetShaderModules()->Load(
          shader_module_path)};

  COMET_ASSERT(shader_module_resource != nullptr,
               "Shader module resource is null!");

  auto* shader_module{CompileShader(shader_module_resource)};
  return shader_modules_.Emplace(shader_module->id, shader_module).value;
}

const ShaderModule* ShaderModuleHandler::Get(
    ShaderModuleId shader_module_id) const {
  auto* shader_module{TryGet(shader_module_id)};
  COMET_ASSERT(shader_module != nullptr,
               "Requested shader module does not exist: ",
               COMET_STRING_ID_LABEL(shader_module_id), "!");
  return shader_module;
}

const ShaderModule* ShaderModuleHandler::TryGet(
    ShaderModuleId shader_module_id) const {
  auto shader_module_ptr{shader_modules_.TryGet(shader_module_id)};

  if (shader_module_ptr == nullptr) {
    return nullptr;
  }

  auto* shader_module{*shader_module_ptr};
  ++shader_module->ref_count;
  return shader_module;
}

const ShaderModule* ShaderModuleHandler::GetOrGenerate(CTStringView path) {
  auto* shader_module{TryGet(COMET_STRING_ID(path))};

  if (shader_module != nullptr) {
    return shader_module;
  }

  return Generate(path);
}

void ShaderModuleHandler::Destroy(ShaderModuleId shader_module_id) {
  Destroy(Get(shader_module_id), false);
}

void ShaderModuleHandler::Destroy(ShaderModule* shader_module) {
  Destroy(shader_module, false);
}

void ShaderModuleHandler::Attach(const Shader* shader,
                                 ShaderModuleId shader_module_id) {
  Attach(shader, Get(shader_module_id));
}

void ShaderModuleHandler::Attach(const Shader* shader,
                                 ShaderModule* shader_module) {
  COMET_ASSERT(shader != nullptr, "Shader is null!");
  COMET_ASSERT(shader_module != nullptr, "Shader module is null!");
  COMET_ASSERT(shader_module->handle != kInvalidShaderModuleHandle,
               "Shader module handle is invalid!");

  glAttachShader(ResolveHandle(shader, shader_module->bind_type),
                 shader_module->handle);
  ++shader_module->ref_count;
}

void ShaderModuleHandler::Detach(const Shader* shader,
                                 ShaderModuleId shader_module_id) {
  Detach(shader, Get(shader_module_id));
}

void ShaderModuleHandler::Detach(const Shader* shader,
                                 ShaderModule* shader_module) {
  COMET_ASSERT(shader != nullptr, "Shader is null!");
  COMET_ASSERT(shader_module != nullptr, "Shader module is null!");
  COMET_ASSERT(shader_module->ref_count > 0,
               "Tried to detach shader module, but ref count is already 0!");

  glDetachShader(ResolveHandle(shader, shader_module->bind_type),
                 shader_module->handle);
  --shader_module->ref_count;
}

GLenum ShaderModuleHandler::GetOpenGlType(ShaderModuleType module_type) {
  switch (module_type) {
    case ShaderModuleType::Compute:
      return GL_COMPUTE_SHADER;

    case ShaderModuleType::Vertex:
      return GL_VERTEX_SHADER;

    case ShaderModuleType::Fragment:
      return GL_FRAGMENT_SHADER;

    default:
      COMET_ASSERT(
          false, "Unknown shader module type: ",
          static_cast<std::underlying_type_t<ShaderModuleType>>(module_type),
          "!");
      return GL_INVALID_VALUE;
  }
}

ShaderModule* ShaderModuleHandler::Get(ShaderModuleId shader_module_id) {
  auto* shader_module{TryGet(shader_module_id)};
  COMET_ASSERT(shader_module != nullptr,
               "Requested shader module does not exist: ",
               COMET_STRING_ID_LABEL(shader_module_id), "!");
  return shader_module;
}

ShaderModule* ShaderModuleHandler::TryGet(ShaderModuleId shader_module_id) {
  auto* shader_module_ptr{shader_modules_.TryGet(shader_module_id)};

  if (shader_module_ptr == nullptr) {
    return nullptr;
  }

  return *shader_module_ptr;
}

void ShaderModuleHandler::Destroy(ShaderModule* shader_module,
                                  bool is_destroying_handler) {
  COMET_ASSERT(shader_module != nullptr, "Shader module is null!");

  if (!is_destroying_handler) {
    COMET_ASSERT(shader_module->ref_count > 0,
                 "Shader module has a reference count of 0!");

    if (--shader_module->ref_count > 0) {
      return;
    }
  }

  if (shader_module->handle != kInvalidShaderModuleHandle) {
    glDeleteShader(shader_module->handle);
    shader_module->handle = kInvalidShaderModuleHandle;
  }

  if (!is_destroying_handler) {
    shader_modules_.Remove(shader_module->id);
  }

  allocator_.Deallocate(shader_module);
}

ShaderModule* ShaderModuleHandler::CompileShader(
    const resource::ShaderModuleResource* resource) {
  COMET_ASSERT(resource != nullptr, "Shader module resource is null!");

  auto* code{reinterpret_cast<const schar*>(resource->data.GetData())};
  auto code_size{static_cast<s32>(resource->data.GetSize())};

  COMET_ASSERT(code_size > 0, "Shader module resource #", resource->id,
               " is empty!");

  auto* shader_module{allocator_.AllocateOneAndPopulate<ShaderModule>()};
  shader_module->id = resource->id;
  shader_module->code = code;
  shader_module->code_size = static_cast<usize>(code_size);
  shader_module->type = GetOpenGlType(resource->descr.shader_type);
  shader_module->bind_type = shader_module->type == GL_COMPUTE_SHADER
                                 ? ShaderBindType::Compute
                                 : ShaderBindType::Graphics;
  shader_module->ref_count = 1;

  shader_module->handle = glCreateShader(shader_module->type);
  COMET_ASSERT(shader_module->handle != kInvalidShaderModuleHandle,
               "Failed to create OpenGL shader module!");

  glShaderSource(shader_module->handle, 1, &code, &code_size);
  glCompileShader(shader_module->handle);

  GLint result{GL_FALSE};
  GLint msg_len{0};

  glGetShaderiv(shader_module->handle, GL_COMPILE_STATUS, &result);
  glGetShaderiv(shader_module->handle, GL_INFO_LOG_LENGTH, &msg_len);

  if (msg_len > 0) {
    auto* error_message{
        GenerateForOneFrame<schar>(static_cast<usize>(msg_len + 1))};
    glGetShaderInfoLog(shader_module->handle, msg_len, nullptr, error_message);
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