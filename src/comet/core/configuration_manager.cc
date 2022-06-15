// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "configuration_manager.h"

#include "comet/rendering/rendering_common.h"

namespace comet {
namespace conf {
void ConfigurationManager::Initialize() {
  // TODO(m4jr0): Implement parser and read from configuration file properly.

  // Application. //////////////////////////////////////////////////////////////
  Set<std::string>("application_name", "Comet Editor");
  Set<u8>("application_major_version", 0);
  Set<u8>("application_minor_version", 1);
  Set<u8>("application_patch_version", 0);

  // Core. /////////////////////////////////////////////////////////////////////
  Set<f64>("core_ms_per_update", 16.66);

  // Entity. ///////////////////////////////////////////////////////////////////
  // *

  // Event. ////////////////////////////////////////////////////////////////////
  // *

  // Input. ////////////////////////////////////////////////////////////////////
  // *

  // Physics. //////////////////////////////////////////////////////////////////
  // *

  // Rendering. ////////////////////////////////////////////////////////////////
  // Common.
  Set<std::string>("rendering_driver", "vulkan");
  Set<rendering::WindowSize>("rendering_window_width", 800);
  Set<rendering::WindowSize>("rendering_window_height", 600);
  Set<f32>("rendering_clear_color_r", 0.5f);
  Set<f32>("rendering_clear_color_g", 0.5f);
  Set<f32>("rendering_clear_color_b", 0.5f);
  Set<f32>("rendering_clear_color_a", 1.0f);
  Set<bool>("rendering_is_vsync", true);
  Set<bool>("rendering_is_sampler_anisotropy", true);
  Set<bool>("rendering_is_sample_rate_shading", true);

  // OpenGL.
  Set<u8>("rendering_opengl_major_version", 4);
  Set<u8>("rendering_opengl_minor_version", 6);

  // Vulkan.
  Set<u8>("rendering_vulkan_variant_version", 0);
  Set<u8>("rendering_vulkan_major_version", 1);
  Set<u8>("rendering_vulkan_minor_version", 2);
  Set<u8>("rendering_vulkan_patch_version", 0);
  Set<u8>("rendering_vulkan_max_frames_in_flight", 2);

  // Resource. /////////////////////////////////////////////////////////////////
  Set<std::string>("resource_root_path", "resources");

  // Time. /////////////////////////////////////////////////////////////////////
  // *
}

void ConfigurationManager::Destroy() {}
}  // namespace conf
}  // namespace comet
