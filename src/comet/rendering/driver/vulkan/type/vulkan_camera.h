// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_VULKAN_TYPE_VULKAN_CAMERA_H_
#define COMET_COMET_RENDERING_DRIVER_VULKAN_TYPE_VULKAN_CAMERA_H_

// External. ///////////////////////////////////////////////////////////////////
#include "vulkan/vulkan.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/essentials.h"
#include "comet/math/matrix.h"
#include "comet/math/vector.h"

namespace comet {
namespace rendering {
namespace vk {
struct GpuCameraData {
  math::Mat4 projection{1.0f};
  math::Mat4 view{1.0f};
  math::Vec4 view_position{0.0f};
};
}  // namespace vk
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_VULKAN_TYPE_VULKAN_CAMERA_H_
