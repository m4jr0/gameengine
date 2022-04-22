// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "configuration_manager.h"

namespace comet {
namespace core {
std::unordered_map<std::string, std::any> ConfigurationManager::values_;

void ConfigurationManager::Initialize() {
  Set<std::string>("application_name", "Comet Editor");
  Set<unsigned int>("application_major_version", 0);
  Set<unsigned int>("application_minor_version", 1);
  Set<unsigned int>("application_patch_version", 0);
  Set<std::string>("engine_name", "Comet Game Engine");
  Set<unsigned int>("engine_major_version", 0);
  Set<unsigned int>("engine_minor_version", 1);
  Set<unsigned int>("engine_patch_version", 0);
  Set<double>("engine_ms_per_update", 16.66 / 1000);
  Set<unsigned int>("rendering_window_width", 800);
  Set<unsigned int>("rendering_window_height", 600);
  Set<unsigned int>("rendering_opengl_major_version", 4);
  Set<unsigned int>("rendering_opengl_minor_version", 6);
  Set<unsigned int>("rendering_vulkan_variant_version", 0);
  Set<unsigned int>("rendering_vulkan_major_version", 1);
  Set<unsigned int>("rendering_vulkan_minor_version", 2);
  Set<unsigned int>("rendering_vulkan_patch_version", 0);
  Set<bool>("rendering_vulkan_is_specific_transfer_queue_requested", true);
  Set<unsigned int>("rendering_vulkan_max_frames_in_flight", 2);
  Set<std::string>("rendering_driver", "vulkan");
}
}  // namespace core
}  // namespace comet
