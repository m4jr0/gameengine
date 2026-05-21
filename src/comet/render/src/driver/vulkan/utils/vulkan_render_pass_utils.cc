// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/render/driver/vulkan/utils/vulkan_render_pass_utils.h"
////////////////////////////////////////////////////////////////////////////////

namespace comet {
namespace rendering {
namespace vk {
bool IsMultisampled(RenderPassOptionFlags flags) {
  return (flags & kRenderPassOptionFlagBitsMultisampled) != 0;
}

bool IsSwapchainTarget(RenderPassOptionFlags flags) {
  return (flags & kRenderPassOptionFlagBitsSwapchainTarget) != 0;
}
}  // namespace vk
}  // namespace rendering
}  // namespace comet
