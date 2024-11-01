// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "configuration_value.h"

#include "comet/core/c_string.h"

namespace comet {
namespace conf {
ConfValue GetDefaultValue(const schar* key) {
  return GetDefaultValue(COMET_STRING_ID(key));
}

ConfValue GetDefaultValue(ConfKey key) {
  ConfValue default_value{};

  if (key == kApplicationName) {
    Copy(default_value.str_value, "Application\0", 12);
  } else if (key == kApplicationMajorVersion) {
    default_value.u16_value = 0;
  } else if (key == kApplicationMinorVersion) {
    default_value.u16_value = 1;
  } else if (key == kApplicationPatchVersion) {
    default_value.u16_value = 0;
  } else if (key == kCoreMsPerUpdate) {
    default_value.f64_value = 16.66;
  } else if (key == kCoreForcedFiberWorkerCount) {
    default_value.u8_value = 0;
  } else if (key == kCoreForcedIOWorkerCount) {
    default_value.u8_value = 0;
  } else if (key == kCoreLargeFiberCount) {
    default_value.u16_value = 128;
  } else if (key == kCoreGiganticFiberCount) {
    default_value.u16_value = 32;
  } else if (key == kCoreExternalLibraryFiberCount) {
    default_value.u16_value = 32;
  } else if (key == kCoreJobCounterCount) {
    default_value.u16_value = 2048;
  } else if (key == kCoreJobQueueCount) {
    default_value.u16_value = 256;
  } else if (key == kCoreIsMainThreadWorkerDisabled) {
    default_value.bool_value = false;
  } else if (key == kCoreTaggedHeapCapacity) {
    default_value.u32_value = 2147483648;  // 2 GiB.
  } else if (key == kCoreFiberFrameAllocatorBaseCapacity) {
    default_value.u32_value = 4194304;  // 4 MiB.
  } else if (key == kCoreIOFrameAllocatorBaseCapacity) {
    default_value.u32_value = 4194304;  // 4 MiB.
  } else if (key == kCoreTStringAllocatorCapacity) {
    default_value.u32_value = 134217728;  // 128 MiB.
  } else if (key == kEventMaxQueueSize) {
    default_value.u16_value = 200;
  } else if (key == kRenderingDriver) {
    Copy(default_value.str_value, kRenderingDriverVulkan.data(),
         kRenderingDriverVulkan.size());
  } else if (key == kRenderingWindowWidth) {
    default_value.u16_value = 800;
  } else if (key == kRenderingWindowHeight) {
    default_value.u16_value = 600;
  } else if (key == kRenderingClearColorR) {
    default_value.f32_value = 0.5f;
  } else if (key == kRenderingClearColorG) {
    default_value.f32_value = 0.5f;
  } else if (key == kRenderingClearColorB) {
    default_value.f32_value = 0.5f;
  } else if (key == kRenderingClearColorA) {
    default_value.f32_value = 1.0f;
  } else if (key == kRenderingIsVsync) {
    default_value.bool_value = true;
  } else if (key == kRenderingIsTripleBuffering) {
    default_value.bool_value = true;
  } else if (key == kRenderingFpsCap) {
    default_value.u16_value = 0;
  } else if (key == kRenderingAntiAliasing) {
    Copy(default_value.str_value, kRenderingAntiAliasingTypeNone.data(),
         kRenderingAntiAliasingTypeNone.size());
  } else if (key == kRenderingIsSamplerAnisotropy) {
    default_value.bool_value = true;
  } else if (key == kRenderingIsSampleRateShading) {
    default_value.bool_value = true;
  } else if (key == kRenderingOpenGlMajorVersion) {
    default_value.u16_value = 4;
  } else if (key == kRenderingOpenGlMinorVersion) {
    default_value.u16_value = 6;
  } else if (key == kRenderingVulkanVariantVersion) {
    default_value.u16_value = 0;
  } else if (key == kRenderingVulkanMajorVersion) {
    default_value.u16_value = 1;
  } else if (key == kRenderingVulkanMinorVersion) {
    default_value.u16_value = 2;
  } else if (key == kRenderingVulkanPatchVersion) {
    default_value.u16_value = 0;
  } else if (key == kRenderingVulkanMaxFramesInFlight) {
    default_value.u16_value = 2;
  } else if (key == kResourceRootPath) {
#ifdef COMET_WIDE_TCHAR
    Copy(default_value.wstr_value, COMET_TCHAR("resources\0"), 10);
#else
    Copy(default_value.str_value, COMET_TCHAR("resources\0"), 10);
#endif  // COMET_WIDE_TCHAR
  }

  return default_value;
}
}  // namespace conf
}  // namespace comet