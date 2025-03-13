// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"

#include "opengl_shader.h"

namespace comet {
namespace rendering {
namespace gl {
ShaderHandle ResolveHandle(const Shader* shader, ShaderBindType bind_type) {
  if (bind_type == ShaderBindType::Compute) {
    return shader->compute_handle;
  } else if (bind_type == ShaderBindType::Graphics) {
    return shader->graphics_handle;
  }

  return kInvalidShaderHandle;
}
}  // namespace gl
}  // namespace rendering
}  // namespace comet