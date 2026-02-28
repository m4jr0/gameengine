// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_RENDERING_UTILS_H_
#define COMET_COMET_RENDERING_RENDERING_UTILS_H_

#include "comet/core/essentials.h"
#include "comet/math/matrix.h"
#include "comet/math/vector.h"

namespace comet {
namespace rendering {
math::Mat4 LookAt(const math::Vec3& eye, const math::Vec3& center,
                  const math::Vec3& up);
math::Mat4 GenerateProjectionMatrix(f32 vertical_fov, f32 ratio, f32 z_near,
                                    f32 z_far);
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_RENDERING_UTILS_H_
