// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_data_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "data/light/light_utils.h"
////////////////////////////////////////////////////////////////////////////////

namespace comet {
namespace light {
ShadowType ResolveShadowType(const LightProperties& props,
                             const LightShadow& shadow) {
  if (!shadow.is_enabled) {
    return ShadowType::None;
  }

  switch (props.type) {
    case LightType::Directional:
      return ShadowType::DirectionalOrtho;
    case LightType::Spot:
      return ShadowType::SpotPerspective;
    case LightType::Point:
      return ShadowType::PointCubemap;
    default:
      return ShadowType::None;
  }
}

bool IsShadowSupportedForLight(const LightProperties& props,
                               const LightShadow& shadow) {
  switch (ResolveShadowType(props, shadow)) {
    case ShadowType::None:
      return true;
    case ShadowType::DirectionalOrtho:
    case ShadowType::SpotPerspective:
      return true;
    case ShadowType::PointCubemap:
      return false;
    default:
      return false;
  }
}
}  // namespace light
}  // namespace comet
