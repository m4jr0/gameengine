// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_render_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/render/driver/vulkan/label/vulkan_texture_label.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/debug/debug_label.h"

namespace comet {
namespace render {
namespace vk {
const schar* GetTextureKeyKindLabel(TextureKeyKind kind) {
  switch (kind) {
    case TextureKeyKind::Resource:
      return "resource";
    case TextureKeyKind::Runtime:
      return "runtime";
    default:
      return kUnknownLabel;
  }
}
}  // namespace vk
}  // namespace render
}  // namespace comet