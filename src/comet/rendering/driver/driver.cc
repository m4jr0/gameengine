// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "driver.h"

#include "comet/core/conf/configuration_value.h"
#include "comet/core/engine.h"

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

  return DriverType::Unknown;
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

Driver::Driver(const DriverDescr& descr)
    : is_vsync_{descr.is_vsync},
      is_triple_buffering_{descr.is_triple_buffering},
      window_width_{descr.window_width},
      window_height_{descr.window_height},
      rendering_view_descrs_{descr.rendering_view_descrs},
      anti_aliasing_type_{descr.anti_aliasing_type},
      is_sampler_anisotropy_{anti_aliasing_type_ != AntiAliasingType::None &&
                             descr.is_sampler_anisotropy},
      is_sample_rate_shading_{anti_aliasing_type_ != AntiAliasingType::None &&
                              descr.is_sample_rate_shading},
      app_name_{descr.app_name},
      app_major_version_{descr.app_major_version},
      app_minor_version_{descr.app_minor_version},
      app_patch_version_{descr.app_patch_version} {
  std::memcpy(clear_color_, descr.clear_color, sizeof(descr.clear_color));
}

Driver ::~Driver() {
  COMET_ASSERT(!is_initialized_,
               "Destructor called for driver, but it is still initialized!");
}

void Driver ::Initialize() {
  COMET_ASSERT(!is_initialized_,
               "Tried to initialize driver, but it is already done!");
  is_initialized_ = true;
}

void Driver ::Shutdown() {
  COMET_ASSERT(is_initialized_,
               "Tried to shutdown driver, but it is not initialized!");
  is_initialized_ = false;
}

bool Driver::IsInitialized() const noexcept { return is_initialized_; }
}  // namespace rendering
}  // namespace comet
