// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/data/render/label/light_label.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/debug_label.h"

namespace comet {
namespace rendering {
const schar* GetLightTypeLabel(LightType type) {
  switch (type) {
    case LightType::Unknown:
      return "unknown";
    case LightType::Directional:
      return "directional";
    case LightType::Spot:
      return "spot";
    case LightType::Point:
      return "point";
    default:
      return kUnknownLabel;
  }
}

const schar* GetShadowTypeLabel(ShadowType type) {
  switch (type) {
    case ShadowType::None:
      return "none";
    case ShadowType::DirectionalOrtho:
      return "directional_ortho";
    case ShadowType::SpotPerspective:
      return "spot_perspective";
    case ShadowType::PointCubemap:
      return "point_cubemap";
    default:
      return kUnknownLabel;
  }
}
}  // namespace rendering
}  // namespace comet