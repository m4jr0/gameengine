// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "vulkan_proxy.h"

namespace comet {
namespace rendering {
namespace vk {
VulkanRenderProxy GenerateVulkanRenderProxy(VulkanMesh& mesh,
                                            VulkanMaterial& material,
                                            const glm::mat4& transform) {
  VulkanRenderProxy proxy{};
  proxy.mesh = &mesh;
  proxy.material = &material;
  proxy.transform = transform;
  return proxy;
}
}  // namespace vk
}  // namespace rendering
}  // namespace comet
