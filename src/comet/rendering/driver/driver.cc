// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "driver.h"

#include "comet/core/engine.h"

namespace comet {
namespace rendering {
DriverType GetDriverTypeFromTypeName(const char* name) {
  if (std::strcmp(name, "opengl") == 0) {
    return DriverType::OpenGl;
  } else if (std::strcmp(name, "vulkan") == 0) {
    return DriverType::Vulkan;
  } else if (std::strcmp(name, "direct3d12") == 0) {
    return DriverType::Direct3d12;
  }

  return DriverType::Unknown;
}

DriverType GetDriverTypeFromTypeName(const std::string& name) {
  return GetDriverTypeFromTypeName(name.c_str());
}
}  // namespace rendering
}  // namespace comet
