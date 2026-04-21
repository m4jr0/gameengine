// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "vulkan_pipeline_label.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/debug_label.h"

namespace comet {
namespace rendering {
namespace vk {
const schar* GetPipelineBindTypeLabel(PipelineBindType type) {
  switch (type) {
    case PipelineBindType::Unknown:
      return "unknown";
    case PipelineBindType::Graphics:
      return "graphics";
    case PipelineBindType::Compute:
      return "compute";
    default:
      return kUnknownLabel;
  }
}
}  // namespace vk
}  // namespace rendering
}  // namespace comet