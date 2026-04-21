// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_VULKAN_TYPE_VULKAN_DESCRIPTOR_TYPE_H_
#define COMET_COMET_RENDERING_DRIVER_VULKAN_TYPE_VULKAN_DESCRIPTOR_TYPE_H_

#include "comet/core/essentials.h"

namespace comet {
namespace rendering {
namespace vk {
constexpr auto kDescriptorSetMaxLayoutCount{8};
constexpr auto kDescriptorBindingMaxCount{32};

enum class DescriptorType { Unknown = 0, Static, Dynamic };
}  // namespace vk
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_VULKAN_TYPE_VULKAN_DESCRIPTOR_TYPE_H_
