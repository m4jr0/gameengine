// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/render/driver/vulkan/label/vulkan_descriptor_label.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/debug_label.h"

namespace comet {
namespace rendering {
namespace vk {
const schar* GetDescriptorTypeLabel(DescriptorType type) {
  switch (type) {
    case DescriptorType::Unknown:
      return "unknown";
    case DescriptorType::Static:
      return "static";
    case DescriptorType::Dynamic:
      return "dynamic";
    default:
      return kUnknownLabel;
  }
}
}  // namespace vk
}  // namespace rendering
}  // namespace comet