// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "opengl_shader_module_label.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/debug_label.h"

namespace comet {
namespace rendering {
namespace gl {
const schar* GetShaderBindTypeLabel(ShaderBindType type) {
  switch (type) {
    case ShaderBindType::Unknown:
      return "unknown";
    case ShaderBindType::Graphics:
      return "graphics";
    case ShaderBindType::Compute:
      return "compute";
    default:
      return kUnknownLabel;
  }
}
}  // namespace gl
}  // namespace rendering
}  // namespace comet