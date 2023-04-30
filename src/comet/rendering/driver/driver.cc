// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "driver.h"

#include "comet/core/conf/configuration_value.h"

namespace comet {
namespace rendering {
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
      app_patch_version_{descr.app_patch_version},
      camera_manager_{descr.camera_manager},
      configuration_manager_{descr.configuration_manager},
#ifdef COMET_DEBUG
      debugger_displayer_manager_{descr.debugger_displayer_manager},
#endif  // COMET_DEBUG
      entity_manager_{descr.entity_manager},
      event_manager_{descr.event_manager},
      resource_manager_{descr.resource_manager} {
  std::memcpy(clear_color_, descr.clear_color, sizeof(descr.clear_color));
  COMET_ASSERT(camera_manager_ != nullptr, "Camera manager is null!");
  COMET_ASSERT(configuration_manager_ != nullptr,
               "Configuration manager is null!");
#ifdef COMET_DEBUG
  COMET_ASSERT(debugger_displayer_manager_ != nullptr,
               "Debugger displayer manager is null!");
#endif  // COMET_DEBUG
  COMET_ASSERT(entity_manager_ != nullptr, "Entity manager is null!");
  COMET_ASSERT(event_manager_ != nullptr, "Event manager is null!");
  COMET_ASSERT(resource_manager_ != nullptr, "Resource manager is null!");
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
