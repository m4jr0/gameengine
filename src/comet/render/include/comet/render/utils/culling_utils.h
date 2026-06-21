// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RENDER_UTILS_CULLING_UTILS_H_
#define COMET_RENDER_UTILS_CULLING_UTILS_H_

#include "comet/core/essentials.h"
#include "comet/core/container/array.h"
#include "comet/core/math/vector.h"
#include "comet/runtime/camera/camera.h"

namespace comet {
namespace render {
void ComputeFrustumCorners(const CameraViewData& camera_data,
                           f32 near_distance, f32 far_distance,
                           StaticArray<math::Vec3, 8>& out);
}  // namespace render
}  // namespace comet

#endif  // COMET_RENDER_UTILS_CULLING_UTILS_H_
