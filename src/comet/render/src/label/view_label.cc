// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/render/label/view_label.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/debug_label.h"

namespace comet {
namespace rendering {
const schar* GetRenderingViewTypeLabel(RenderingViewType type) {
  switch (type) {
    case RenderingViewType::Unknown:
      return "unknown";
    case RenderingViewType::World:
      return "world";
    case RenderingViewType::Shadow:
      return "shadow";
    case RenderingViewType::Skybox:
      return "skybox";
#ifdef COMET_DEBUG_VIEW
    case RenderingViewType::Debug:
      return "debug";
#endif  // COMET_DEBUG_VIEW
#ifdef COMET_IMGUI
    case RenderingViewType::ImGui:
      return "imgui";
#endif  // COMET_IMGUI
    default:
      return kUnknownLabel;
  }
}

const schar* GetRenderingViewMatrixSourceLabel(RenderingViewMatrixSource src) {
  switch (src) {
    case RenderingViewMatrixSource::Unknown:
      return "unknown";
    case RenderingViewMatrixSource::SceneCamera:
      return "scene_camera";
    case RenderingViewMatrixSource::UiCamera:
      return "ui_camera";
    default:
      return kUnknownLabel;
  }
}
}  // namespace rendering
}  // namespace comet