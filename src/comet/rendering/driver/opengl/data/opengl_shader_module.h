// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_OPENGL_DATA_OPENGL_SHADER_MODULE_H_
#define COMET_COMET_RENDERING_DRIVER_OPENGL_DATA_OPENGL_SHADER_MODULE_H_

#include "glad/glad.h"

#include "comet/core/essentials.h"

namespace comet {
namespace rendering {
namespace gl {
using ShaderModuleHandle = u32;
constexpr auto kInvalidShaderModuleHandle{0};

enum class ShaderBindType { Unknown = 0, Graphics, Compute };

struct ShaderModule {
  u16 ref_count{0};
  ShaderBindType bind_type{ShaderBindType::Unknown};
  ShaderModuleHandle handle{kInvalidShaderModuleHandle};
  GLenum type{GL_INVALID_VALUE};
};
}  // namespace gl
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_OPENGL_DATA_OPENGL_SHADER_MODULE_H_
