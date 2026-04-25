// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_UTILS_CULLING_UTILS_H_
#define COMET_COMET_RENDERING_UTILS_CULLING_UTILS_H_

#include "comet/core/essentials.h"
#include "comet/core/type/array.h"
#include "comet/math/vector.h"
#include "comet/rendering/type/camera.h"

namespace comet {
namespace rendering {
void ComputeFrustumCorners(const RenderCameraData& camera_data,
                           f32 near_distance, f32 far_distance,
                           StaticArray<math::Vec3, 8>& out);
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_UTILS_CULLING_UTILS_H_
