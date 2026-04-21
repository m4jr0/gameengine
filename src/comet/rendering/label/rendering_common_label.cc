// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "rendering_common_label.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/debug_label.h"

namespace comet {
namespace rendering {
const schar* GetDriverTypeLabel(DriverType type) {
  switch (type) {
    case DriverType::Unknown:
      return "unknown";
    case DriverType::Empty:
      return "empty";
    case DriverType::OpenGl:
      return "opengl";
    case DriverType::Vulkan:
      return "vulkan";
    case DriverType::Direct3d12:
      return "direct3d12";
    default:
      return kUnknownLabel;
  }
}

const schar* GetAntiAliasingTypeLabel(AntiAliasingType type) {
  switch (type) {
    case AntiAliasingType::None:
      return "none";
    case AntiAliasingType::Msaa:
      return "msaa";
    case AntiAliasingType::MsaaX2:
      return "msaa_x2";
    case AntiAliasingType::MsaaX4:
      return "msaa_x4";
    case AntiAliasingType::MsaaX8:
      return "msaa_x8";
    case AntiAliasingType::MsaaX16:
      return "msaa_x16";
    case AntiAliasingType::MsaaX32:
      return "msaa_x32";
    case AntiAliasingType::MsaaX64:
      return "msaa_x64";
    default:
      return kUnknownLabel;
  }
}
}  // namespace rendering
}  // namespace comet