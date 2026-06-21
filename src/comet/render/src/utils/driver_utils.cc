// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_render_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/render/utils/driver_utils.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/render/common.h"
#include "comet/runtime/conf/conf_manager.h"
#include "comet/runtime/conf/config_defaults.h"
#include "comet/runtime/conf/config_keys.h"
#include "comet/runtime/conf/config_value.h"

namespace comet {
namespace render {
DriverType GetDriverTypeFromStr(std::string_view str) {
  if (str == conf::kRenderingDriverOpengl) {
    return DriverType::OpenGl;
  } else if (str == conf::kRenderingDriverVulkan) {
    return DriverType::Vulkan;
  } else if (str == conf::kRenderingDriverDirect3d12) {
    return DriverType::Direct3d12;
  }
#ifdef COMET_DEBUG
  else if (str == conf::kRenderingDriverEmpty) {
    return DriverType::Empty;
  }
#endif  // COMET_DEBUG

  return DriverType::Unknown;
}

DriverType GetDriverType() {
  return GetDriverTypeFromStr(COMET_CONF_STR(conf::kRenderingDriver));
}

bool IsMultithreading(DriverType type) {
#ifdef COMET_ENABLE_RENDERDOC_COMPATIBILITY
  return false;
#else
  switch (type) {
    case DriverType::OpenGl:
      return false;

    case DriverType::Vulkan:
    case DriverType::Direct3d12:
      return true;

#ifdef COMET_DEBUG
    case DriverType::Empty:
      return true;
#endif  // COMET_DEBUG

    case DriverType::Unknown:
    default:
      return false;
  }
#endif  // COMET_ENABLE_RENDERDOC_COMPATIBILITY
}

AntiAliasingType GetAntiAliasingTypeFromStr(std::string_view str) {
  if (str == conf::kRenderingAntiAliasingTypeNone) {
    return AntiAliasingType::None;
  } else if (str == conf::kRenderingAntiAliasingTypeMsaaX64) {
    return AntiAliasingType::MsaaX64;
  } else if (str == conf::kRenderingAntiAliasingTypeMsaaX32) {
    return AntiAliasingType::MsaaX32;
  } else if (str == conf::kRenderingAntiAliasingTypeMsaaX16) {
    return AntiAliasingType::MsaaX16;
  } else if (str == conf::kRenderingAntiAliasingTypeMsaaX8) {
    return AntiAliasingType::MsaaX8;
  } else if (str == conf::kRenderingAntiAliasingTypeMsaaX4) {
    return AntiAliasingType::MsaaX4;
  } else if (str == conf::kRenderingAntiAliasingTypeMsaaX2) {
    return AntiAliasingType::MsaaX2;
  } else if (str == conf::kRenderingAntiAliasingTypeMsaa) {
    return AntiAliasingType::Msaa;
  }

  return AntiAliasingType::None;
}

u8 GetMsaaSampleCount(AntiAliasingType anti_aliasing_type) {
  switch (anti_aliasing_type) {
    case AntiAliasingType::Msaa:
    case AntiAliasingType::MsaaX64:
    case AntiAliasingType::MsaaX32:
    case AntiAliasingType::MsaaX16:
    case AntiAliasingType::MsaaX8:
      return 8;

    case AntiAliasingType::MsaaX4:
      return 4;

    case AntiAliasingType::MsaaX2:
      return 2;

    case AntiAliasingType::None:
    default:
      return 1;
  }
}
}  // namespace render
}  // namespace comet