// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_VULKAN_TYPE_VULKAN_RENDER_PROXY_H_
#define COMET_COMET_RENDERING_DRIVER_VULKAN_TYPE_VULKAN_RENDER_PROXY_H_

#include "comet/core/essentials.h"
#include "comet/rendering/type/render_proxy.h"
#include "vulkan/vulkan.h"

namespace comet {
namespace rendering {
namespace vk {
struct GpuIndirectRenderProxy {
  VkDrawIndexedIndirectCommand command{};
  RenderProxyId proxy_id{kInvalidRenderProxyId};
  BatchId batch_id{kInvalidBatchId};
};
}  // namespace vk
}  // namespace rendering
}  // namespace comet

#endif  // !COMET_COMET_RENDERING_DRIVER_VULKAN_TYPE_VULKAN_RENDER_PROXY_H_