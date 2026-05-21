// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RENDER_DRIVER_VULKAN_LABEL_VULKAN_DESCRIPTOR_LABEL_H_
#define COMET_RENDER_DRIVER_VULKAN_LABEL_VULKAN_DESCRIPTOR_LABEL_H_

#include "comet/core/essentials.h"
#include "comet/rendering/driver/vulkan/type/vulkan_descriptor.h"

namespace comet {
namespace rendering {
namespace vk {
const schar* GetDescriptorTypeLabel(DescriptorType type);
}  // namespace vk
}  // namespace rendering
}  // namespace comet

#endif  // COMET_RENDER_DRIVER_VULKAN_LABEL_VULKAN_DESCRIPTOR_LABEL_H_