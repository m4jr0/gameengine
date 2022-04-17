// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "configuration_manager.h"

namespace comet {
namespace core {
std::unordered_map<std::string, std::any> ConfigurationManager::values_;

void ConfigurationManager::Initialize() {
  Set<double>("engine_ms_per_update", 16.66 / 1000);
  Set<unsigned int>("rendering_window_width", 800);
  Set<unsigned int>("rendering_window_height", 600);
  Set<unsigned int>("rendering_opengl_major_version", 4);
  Set<unsigned int>("rendering_opengl_minor_version", 6);
  Set<std::string>("rendering_window_name", "Comet Game Engine");
  Set<std::string>("rendering_driver", "vulkan");
}
}  // namespace core
}  // namespace comet
