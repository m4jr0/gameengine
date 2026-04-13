// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "opengl_common_utils.h"
////////////////////////////////////////////////////////////////////////////////

namespace comet {
namespace rendering {
namespace gl {
ShaderStageFlags ResolveStageFlags(ShaderStageFlags flags) { return flags; }

bool IsGraphicsStage(ShaderStageFlags flags) {
  return (flags &
          (kShaderStageFlagBitsVertex | kShaderStageFlagBitsFragment)) != 0;
}

bool IsComputeStage(ShaderStageFlags flags) {
  return (flags & kShaderStageFlagBitsCompute) != 0;
}
}  // namespace gl
}  // namespace rendering
}  // namespace comet