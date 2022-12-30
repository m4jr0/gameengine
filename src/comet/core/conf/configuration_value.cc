// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "configuration_value.h"

namespace comet {
namespace conf {
ConfValue GetDefaultValue(const std::string& key) {
  return GetDefaultValue(key.c_str());
}

ConfValue GetDefaultValue(const char* key) {
  return GetDefaultValue(COMET_STRING_ID(key));
}

ConfValue GetDefaultValue(ConfKey key) {
  ConfValue default_value{};

  if (key == kApplicationName) {
    std::memcpy(&default_value.str, "Comet Editor", 12);
  } else if (key == kApplicationMajorVersion) {
    default_value.ushort = 0;
  } else if (key == kApplicationMinorVersion) {
    default_value.ushort = 1;
  } else if (key == kApplicationPatchVersion) {
    default_value.ushort = 0;
  } else if (key == kCoreMsPerUpdate) {
    default_value.flong = 16.66;
  } else if (key == kRenderingDriver) {
    std::memcpy(&default_value.str, "vulkan", 6);
  } else if (key == kRenderingWindowWidth) {
    default_value.ushort = 800;
  } else if (key == kRenderingWindowHeight) {
    default_value.ushort = 600;
  } else if (key == kRenderingClearColorR) {
    default_value.f = 0.5f;
  } else if (key == kRenderingClearColorG) {
    default_value.f = 0.5f;
  } else if (key == kRenderingClearColorB) {
    default_value.f = 0.5f;
  } else if (key == kRenderingClearColorA) {
    default_value.f = 1.0f;
  } else if (key == kRenderingIsVsync) {
    default_value.flag = true;
  } else if (key == kRenderingIsSamplerAnisotropy) {
    default_value.flag = true;
  } else if (key == kRenderingIsSampleRateShading) {
    default_value.flag = true;
  } else if (key == kRenderingOpenGlMajorVersion) {
    default_value.ushort = 4;
  } else if (key == kRenderingOpenGlMinorVersion) {
    default_value.ushort = 6;
  } else if (key == kRenderingVulkanVariantVersion) {
    default_value.ushort = 0;
  } else if (key == kRenderingVulkanMajorVersion) {
    default_value.ushort = 1;
  } else if (key == kRenderingVulkanMinorVersion) {
    default_value.ushort = 2;
  } else if (key == kRenderingVulkanPatchVersion) {
    default_value.ushort = 0;
  } else if (key == kRenderingVulkanMaxFramesInFlight) {
    default_value.ushort = 2;
  } else if (key == kResourceRootPath) {
    std::memcpy(&default_value.str, "resources", 9);
  }

  return default_value;
}
}  // namespace conf
}  // namespace comet