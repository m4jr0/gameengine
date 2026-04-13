// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "light_common.h"
////////////////////////////////////////////////////////////////////////////////

namespace comet {
namespace rendering {
const schar* GetLightTypeLabel(LightType light_type) {
  switch (light_type) {
    using enum LightType;

    case Unknown:
      return "unknown";
    case Directional:
      return "directional";
    case Spot:
      return "spot";
    case Point:
      return "point";
  }

  return "???";
}

const schar* GetShadowTypeLabel(ShadowType stack_size) {
  switch (stack_size) {
    using enum ShadowType;

    case None:
      return "none";
    case DirectionalOrtho:
      return "directional_ortho";
    case SpotPerspective:
      return "spot_perspective";
    case PointCubemap:
      return "point_cubemap";
  }

  return "???";
}

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
}  // namespace rendering
}  // namespace comet
