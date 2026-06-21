// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RENDER_DRIVER_VULKAN_UTILS_VULKAN_PIPELINE_UTILS_H_
#define COMET_RENDER_DRIVER_VULKAN_UTILS_VULKAN_PIPELINE_UTILS_H_

// External. ///////////////////////////////////////////////////////////////////
#include "vulkan/vulkan.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/essentials.h"
#include "comet/render/type/pipeline.h"

namespace comet {
namespace rendering {
namespace vk {
VkPipelineBindPoint GetVkPipelineBindPoint(PipelineBindType bind_type);
}  // namespace vk
}  // namespace rendering
}  // namespace comet

#endif  // COMET_RENDER_DRIVER_VULKAN_UTILS_VULKAN_PIPELINE_UTILS_H_
