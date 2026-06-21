// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RENDER_DRIVER_VULKAN_LABEL_VULKAN_RENDER_PASS_LABEL_H_
#define COMET_RENDER_DRIVER_VULKAN_LABEL_VULKAN_RENDER_PASS_LABEL_H_

#include "comet/core/essentials.h"
#include "comet/render/driver/vulkan/type/vulkan_render_pass.h"

namespace comet {
namespace rendering {
namespace vk {
const schar* GetAttachmentTypeLabel(AttachmentType type);
}  // namespace vk
}  // namespace rendering
}  // namespace comet

#endif  // COMET_RENDER_DRIVER_VULKAN_LABEL_VULKAN_RENDER_PASS_LABEL_H_