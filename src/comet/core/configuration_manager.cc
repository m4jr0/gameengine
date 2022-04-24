// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "configuration_manager.h"

namespace comet {
namespace conf {
void ConfigurationManager::Initialize() {
  Set<std::string>("application_name", "Comet Editor");
  Set<u8>("application_major_version", 0);
  Set<u8>("application_minor_version", 1);
  Set<u8>("application_patch_version", 0);
  Set<std::string>("engine_name", "Comet Game Engine");
  Set<u8>("engine_major_version", 0);
  Set<u8>("engine_minor_version", 1);
  Set<u8>("engine_patch_version", 0);
  Set<f64>("engine_ms_per_update", 16.66 / 1000);
  Set<u16>("rendering_window_width", 800);
  Set<u16>("rendering_window_height", 600);
  Set<u8>("rendering_opengl_major_version", 4);
  Set<u8>("rendering_opengl_minor_version", 6);
  Set<u8>("rendering_vulkan_variant_version", 0);
  Set<u8>("rendering_vulkan_major_version", 1);
  Set<u8>("rendering_vulkan_minor_version", 2);
  Set<u8>("rendering_vulkan_patch_version", 0);
  Set<bool>("rendering_vulkan_is_specific_transfer_queue_requested", true);
  Set<u8>("rendering_vulkan_max_frames_in_flight", 2);
  Set<std::string>("rendering_driver", "vulkan");
  Set<std::string>("resource_root_path", "resources");
}

void ConfigurationManager::Destroy() {}
}  // namespace conf
}  // namespace comet
