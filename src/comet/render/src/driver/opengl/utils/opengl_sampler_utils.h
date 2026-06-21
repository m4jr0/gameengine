// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RENDER_DRIVER_OPENGL_UTILS_OPENGL_SAMPLER_UTILS_H_
#define COMET_RENDER_DRIVER_OPENGL_UTILS_OPENGL_SAMPLER_UTILS_H_

#include "comet/core/essentials.h"
#include "comet/render/driver/opengl/type/opengl_sampler.h"

namespace comet {
namespace render {
namespace gl {
SamplerKey GenerateSamplerKey(const SamplerDescr& descr);
}  // namespace gl
}  // namespace render
}  // namespace comet

#endif  // COMET_RENDER_DRIVER_OPENGL_UTILS_OPENGL_SAMPLER_UTILS_H_