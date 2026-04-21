// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_OPENGL_TYPE_OPENGL_SHADER_MODULE_TYPE_H_
#define COMET_COMET_RENDERING_DRIVER_OPENGL_TYPE_OPENGL_SHADER_MODULE_TYPE_H_

// External. ///////////////////////////////////////////////////////////////////
#include "glad/glad.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/essentials.h"
#include "comet/rendering/rendering_handle.h"
#include "comet/resource/shader/shader_module_resource.h"

namespace comet {
namespace rendering {
namespace gl {
using GlShaderModuleNativeHandle = u32;
constexpr auto kInvalidGlShaderModuleNativeHandle{0};

enum class ShaderBindType { Unknown = 0, Graphics, Compute };

struct ShaderModule {
  resource::ShaderModuleResourceId id{};
  ShaderModuleHandle handle{};
  usize code_size{0};
  const schar* code{nullptr};
  ShaderBindType bind_type{ShaderBindType::Unknown};
  GlShaderModuleNativeHandle native_handle{kInvalidGlShaderModuleNativeHandle};
  GLenum stage{GL_INVALID_VALUE};
};
}  // namespace gl
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_OPENGL_TYPE_OPENGL_SHADER_MODULE_TYPE_H_