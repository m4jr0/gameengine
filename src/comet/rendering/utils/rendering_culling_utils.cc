// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "rendering_culling_utils.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/math/geometry.h"

namespace comet {
namespace rendering {
void ComputeFrustumCorners(const RenderCameraData& camera_data,
                           f32 near_distance, f32 far_distance,
                           StaticArray<math::Vec3, 8>& out) {
  const auto& position{camera_data.view_position};
  const auto& front{camera_data.front};
  const auto& up{camera_data.up};
  const auto& right{camera_data.right};
  const auto fov{camera_data.fov_y_radians};
  const auto aspect{camera_data.aspect_ratio};

  const auto near_center{position + front * near_distance};
  const auto far_center{position + front * far_distance};

  const auto near_half_height{math::Tan(fov * .5f) * near_distance};
  const auto near_half_width{near_half_height * aspect};

  const auto far_half_height{math::Tan(fov * .5f) * far_distance};
  const auto far_half_width{far_half_height * aspect};

  out[0] = near_center + up * near_half_height - right * near_half_width;
  out[1] = near_center + up * near_half_height + right * near_half_width;
  out[2] = near_center - up * near_half_height - right * near_half_width;
  out[3] = near_center - up * near_half_height + right * near_half_width;

  out[4] = far_center + up * far_half_height - right * far_half_width;
  out[5] = far_center + up * far_half_height + right * far_half_width;
  out[6] = far_center - up * far_half_height - right * far_half_width;
  out[7] = far_center - up * far_half_height + right * far_half_width;
}
}  // namespace rendering
}  // namespace comet
