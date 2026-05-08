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

#include "comet/core/frame/frame_string.h"
#include "comet/core/memory/allocator/allocator.h"
#include "comet/rendering/driver/opengl/utils/opengl_shader_utils.h"
#include "comet/resource/resource_manager.h"

namespace comet {
namespace rendering {
namespace gl {
ShaderModuleHandler::ShaderModuleHandler(const ShaderModuleHandlerDescr& descr)
    : Handler{descr}, shader_modules_{&cache_allocator_, 256} {}

ShaderModuleHandle ShaderModuleHandler::Generate(
    resource::ShaderModuleResourceId shader_module_resource_id) {
  COMET_ASSERT(shader_module_resource_id.IsValid(),
               "ShaderModuleHandler::Generate",
               "shader module resource id is invalid");

  if (const auto handle{shader_modules_.TryAcquire(shader_module_resource_id)};
      handle) {
    return handle;
  }

  ShaderModuleHandle generated_handle{ShaderModuleHandle::Invalid()};

  auto* shader_module_resource_handler{
      resource::ResourceManager::Get().GetShaderModules()};

  const auto is_loaded{shader_module_resource_handler->WithTemporaryLoad(
      shader_module_resource_id,
      [this, &generated_handle, shader_module_resource_id](
          const resource::ShaderModuleResource* shader_module_resource) {
        auto* shader_module{GenerateShaderModule(shader_module_resource)};
        COMET_ASSERT(shader_module != nullptr, "ShaderModuleHandler::Generate",
                     "generated shader module is null",
                     "shader_module_resource_id", shader_module_resource_id);

        generated_handle =
            shader_modules_.Create(shader_module->id, shader_module);
        COMET_ASSERT(generated_handle, "ShaderModuleHandler::Generate",
                     "shader module instance creation failed",
                     "shader_module_resource_id", shader_module_resource_id);

        shader_module->handle = generated_handle;
      })};

  return is_loaded ? generated_handle : ShaderModuleHandle::Invalid();
}

void ShaderModuleHandler::Destroy(ShaderModuleHandle handle) {
  auto* shader_module{shader_modules_.Get(handle)};

  if (!shader_modules_.Release(handle)) {
    return;
  }

  DestroyShaderModule(shader_module);
  shader_modules_.Remove(handle);
}

void ShaderModuleHandler::Attach(const Shader* shader,
                                 ShaderModuleHandle handle) const {
  COMET_ASSERT(shader != nullptr, "ShaderModuleHandler::Attach",
               "shader is null");

  const auto* shader_module{Get(handle)};
  COMET_ASSERT(
      shader_module->native_handle != kInvalidGlShaderModuleNativeHandle,
      "ShaderModuleHandler::Attach", "shader module native handle is invalid",
      "shader_module_handle", handle);

  glAttachShader(shader->program_native_handle, shader_module->native_handle);
}

void ShaderModuleHandler::Detach(const Shader* shader,
                                 ShaderModuleHandle handle) const {
  COMET_ASSERT(shader != nullptr, "ShaderModuleHandler::Detach",
               "shader is null");

  const auto* shader_module{Get(handle)};
  COMET_ASSERT(
      shader_module->native_handle != kInvalidGlShaderModuleNativeHandle,
      "ShaderModuleHandler::Detach", "shader module native handle is invalid",
      "shader_module_handle", handle);

  glDetachShader(shader->program_native_handle, shader_module->native_handle);
}

GLenum ShaderModuleHandler::GetStage(ShaderModuleHandle handle) const {
  return Get(handle)->stage;
}

PipelineBindType ShaderModuleHandler::GetBindType(
    ShaderModuleHandle handle) const {
  return Get(handle)->bind_type;
}

ShaderModule* ShaderModuleHandler::Get(ShaderModuleHandle handle) {
  auto* shader_module{shader_modules_.TryGet(handle)};
  COMET_ASSERT(shader_module != nullptr, "ShaderModuleHandler::Get",
               "shader module not found", "shader_module_handle", handle);
  return shader_module;
}

const ShaderModule* ShaderModuleHandler::Get(ShaderModuleHandle handle) const {
  const auto* shader_module{shader_modules_.TryGet(handle)};
  COMET_ASSERT(shader_module != nullptr, "ShaderModuleHandler::Get",
               "shader module not found", "shader_module_handle", handle);
  return shader_module;
}

void ShaderModuleHandler::OnInitialize() {
  allocator_.Initialize();
  shader_modules_.Initialize();
}

void ShaderModuleHandler::OnShutdown() {
  memory::PlatformAllocator tmp_allocator{memory::kEngineMemoryTagRendering};
  auto handles_to_destroy{Array<ShaderModuleHandle>::WithCapacity(
      &tmp_allocator, shader_modules_.GetLiveCount())};

  shader_modules_.ForEachLive(
      [&handles_to_destroy](ShaderModuleHandle handle, const ShaderModule*) {
        handles_to_destroy.PushLast(handle);
      });

  for (const auto handle : handles_to_destroy) {
    const auto ref_count{shader_modules_.GetRefCount(handle)};

    if (ref_count > 0) {
      COMET_LOG_WARNING(
          LoggerType::Rendering, "ShaderModuleHandler::OnShutdown",
          "forcing shader module destruction", "shader_module_handle", handle,
          "ref_count", ref_count, "shader_module_resource_id",
          shader_modules_.Get(handle)->id);
    }

    auto* shader_module{shader_modules_.Drain(handle)};

    if (shader_module == nullptr) {
      continue;
    }

    COMET_LOG_WARNING(LoggerType::Rendering, "ShaderModuleHandler::OnShutdown",
                      "forcing shader module destruction",
                      "shader_module_handle", handle, "ref_count", ref_count,
                      "shader_module_resource_id",
                      shader_modules_.Get(handle)->id);

    DestroyShaderModule(shader_module);
  }

  shader_modules_.Destroy();
  allocator_.Destroy();
}

ShaderModule* ShaderModuleHandler::GenerateShaderModule(
    const resource::ShaderModuleResource* shader_module_resource) {
  COMET_ASSERT(shader_module_resource != nullptr,
               "ShaderModuleHandler::GenerateShaderModule",
               "shader module resource is null");

  auto* code{
      reinterpret_cast<const schar*>(shader_module_resource->data.GetData())};
  const auto code_size{
      static_cast<s32>(shader_module_resource->data.GetSize())};

  COMET_ASSERT(code_size > 0, "ShaderModuleHandler::GenerateShaderModule",
               "shader module resource is empty", "shader_module_resource_id",
               shader_module_resource->id);

  auto* shader_module{allocator_.AllocateOneAndPopulate<ShaderModule>()};
  shader_module->handle = ShaderModuleHandle::Invalid();
  shader_module->id = shader_module_resource->GetId();
  shader_module->code = code;
  shader_module->code_size = static_cast<usize>(code_size);
  shader_module->stage = GetGlStage(shader_module_resource->descr.stage);
  shader_module->bind_type = shader_module->stage == GL_COMPUTE_SHADER
                                 ? PipelineBindType::Compute
                                 : PipelineBindType::Graphics;

  shader_module->native_handle = glCreateShader(shader_module->stage);
  COMET_ASSERT(
      shader_module->native_handle != kInvalidGlShaderModuleNativeHandle,
      "ShaderModuleHandler::GenerateShaderModule",
      "opengl shader module creation failed", "shader_module_resource_id",
      shader_module->id);

  glShaderSource(shader_module->native_handle, 1, &code, &code_size);
  glCompileShader(shader_module->native_handle);

  GLint result{GL_FALSE};
  GLint msg_len{0};

  glGetShaderiv(shader_module->native_handle, GL_COMPILE_STATUS, &result);
  glGetShaderiv(shader_module->native_handle, GL_INFO_LOG_LENGTH, &msg_len);

  if (msg_len > 0) {
    auto* error_message{
        GenerateFrameString<schar>(static_cast<usize>(msg_len + 1))};
    glGetShaderInfoLog(shader_module->native_handle, msg_len, nullptr,
                       error_message);

    if (result == GL_FALSE) {
      COMET_ASSERT(false, "ShaderModuleHandler::GenerateShaderModule",
                   "shader module compilation failed",
                   "shader_module_resource_id", shader_module->id, "info_log",
                   error_message);
    } else {
      COMET_LOG_WARNING(
          LoggerType::Rendering, "ShaderModuleHandler::GenerateShaderModule",
          "shader module compilation warning", "shader_module_resource_id",
          shader_module->id, "info_log", error_message);
    }
  } else if (result == GL_FALSE) {
    COMET_ASSERT(false, "ShaderModuleHandler::GenerateShaderModule",
                 "shader module compilation failed",
                 "shader_module_resource_id", shader_module->id);
  }

  return shader_module;
}

void ShaderModuleHandler::DestroyShaderModule(ShaderModule* shader_module) {
  COMET_ASSERT(shader_module != nullptr,
               "ShaderModuleHandler::DestroyShaderModule",
               "shader module is null");

  if (shader_module->native_handle != kInvalidGlShaderModuleNativeHandle) {
    glDeleteShader(shader_module->native_handle);
    shader_module->native_handle = kInvalidGlShaderModuleNativeHandle;
  }

  shader_module->handle.Invalidate();
  allocator_.Deallocate(shader_module);
}
}  // namespace gl
}  // namespace rendering
}  // namespace comet