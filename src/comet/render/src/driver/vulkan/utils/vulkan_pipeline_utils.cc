// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_render_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/render/driver/vulkan/utils/vulkan_view_utils.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/type_trait.h"
#include "comet/data/render/label/pipeline_label.h"

namespace comet {
namespace render {
namespace vk {
VkPipelineBindPoint GetVkPipelineBindPoint(PipelineBindType bind_type) {
  switch (bind_type) {
    case PipelineBindType::Graphics:
      return VK_PIPELINE_BIND_POINT_GRAPHICS;
    case PipelineBindType::Compute:
      return VK_PIPELINE_BIND_POINT_COMPUTE;
    default:
      COMET_ASSERT(false, "vulkan_pipeline_utils::GetVkPipelineBindPoint",
                   "pipeline bind point is unsupported", "bind_type",
                   GetPipelineBindTypeLabel(bind_type), "bind_type_value",
                   ToUnderlying(bind_type));
      return VK_PIPELINE_BIND_POINT_MAX_ENUM;
  }
}
}  // namespace vk
}  // namespace render
}  // namespace comet
