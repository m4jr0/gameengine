// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_render_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/render/driver/vulkan/label/vulkan_render_pass_label.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/debug/debug_label.h"

namespace comet {
namespace render {
namespace vk {
const schar* GetAttachmentTypeLabel(AttachmentType type) {
  switch (type) {
    case AttachmentType::Unknown:
      return "unknown";
    case AttachmentType::Color:
      return "color";
    case AttachmentType::Depth:
      return "depth";
    case AttachmentType::Stencil:
      return "stencil";
    case AttachmentType::Resolve:
      return "resolve";
    default:
      return kUnknownLabel;
  }
}
}  // namespace vk
}  // namespace render
}  // namespace comet