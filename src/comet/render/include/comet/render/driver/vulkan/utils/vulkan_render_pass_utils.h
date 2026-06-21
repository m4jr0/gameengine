// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RENDER_DRIVER_VULKAN_UTILS_VULKAN_RENDER_PASS_UTILS_H_
#define COMET_RENDER_DRIVER_VULKAN_UTILS_VULKAN_RENDER_PASS_UTILS_H_

// External. ///////////////////////////////////////////////////////////////////
#include "vulkan/vulkan.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/essentials.h"
#include "comet/render/driver/vulkan/type/vulkan_render_pass.h"

namespace comet {
namespace rendering {
namespace vk {
bool IsMultisampled(RenderPassOptionFlags flags);
bool IsSwapchainTarget(RenderPassOptionFlags flags);
}  // namespace vk
}  // namespace rendering
}  // namespace comet

#endif  // COMET_RENDER_DRIVER_VULKAN_UTILS_VULKAN_RENDER_PASS_UTILS_H_
