// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RENDER_DRIVER_OPENGL_TYPE_OPENGL_SHADER_MODULE_H_
#define COMET_RENDER_DRIVER_OPENGL_TYPE_OPENGL_SHADER_MODULE_H_

// External. ///////////////////////////////////////////////////////////////////
#include "glad/glad.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/essentials.h"
#include "comet/render/render_handle.h"
#include "comet/data/resource/shader/shader_module_resource.h"

namespace comet {
namespace render {
namespace gl {
using GlShaderModuleNativeHandle = u32;
constexpr auto kInvalidGlShaderModuleNativeHandle{0};

struct ShaderModule {
  resource::ShaderModuleResourceId id{};
  ShaderModuleHandle handle{};
  usize code_size{0};
  const schar* code{nullptr};
  PipelineBindType bind_type{PipelineBindType::Unknown};
  GlShaderModuleNativeHandle native_handle{kInvalidGlShaderModuleNativeHandle};
  GLenum stage{GL_INVALID_VALUE};
};
}  // namespace gl
}  // namespace render
}  // namespace comet

#endif  // COMET_RENDER_DRIVER_OPENGL_TYPE_OPENGL_SHADER_MODULE_H_