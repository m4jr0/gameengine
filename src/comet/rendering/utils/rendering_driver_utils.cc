// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "rendering_driver_utils.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/conf/configuration_manager.h"
#include "comet/core/conf/configuration_value.h"
#include "comet/rendering/type/rendering_common_type.h"

namespace comet {
namespace rendering {
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

bool IsMultithreading([[maybe_unused]] DriverType type) {
#ifdef COMET_ENABLE_RENDERDOC_COMPATIBILITY
#ifndef COMET_ALLOW_DISABLED_MAIN_THREAD_WORKER
  COMET_ASSERT(false, "rendering_driver_utils::IsMultithreading",
               "main thread worker must be allowed when renderdoc "
               "compatibility is enabled");
#endif  // !COMET_ALLOW_DISABLED_MAIN_THREAD_WORKER
  return false;
#else
  switch (type) {
    case DriverType::OpenGl:
      return false;
    case DriverType::Vulkan:
      return true;
    case DriverType::Direct3d12:
      return true;
#ifdef COMET_DEBUG
    case DriverType::Empty:
      return true;
#endif  // COMET_DEBUG
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
}  // namespace rendering
}  // namespace comet