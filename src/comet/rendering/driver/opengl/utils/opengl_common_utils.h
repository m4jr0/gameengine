// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_OPENGL_UTILS_OPENGL_SHADER_UTILS_H_
#define COMET_COMET_RENDERING_DRIVER_OPENGL_UTILS_OPENGL_SHADER_UTILS_H_

#include "comet/core/essentials.h"
#include "comet/rendering/rendering_common.h"

namespace comet {
namespace rendering {
namespace gl {
ShaderStageFlags ResolveStageFlags(ShaderStageFlags flags);
bool IsGraphicsStage(ShaderStageFlags flags);
bool IsComputeStage(ShaderStageFlags flags);
}  // namespace gl
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_OPENGL_UTILS_OPENGL_SHADER_UTILS_H_