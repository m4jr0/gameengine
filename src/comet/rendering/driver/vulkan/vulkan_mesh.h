// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_VULKAN_VULKAN_MESH_H_
#define COMET_COMET_RENDERING_DRIVER_VULKAN_VULKAN_MESH_H_

#include "comet_precompile.h"

#include "glm/glm.hpp"
#include "glm/gtx/hash.hpp"
#include "vulkan/vulkan.h"

#include "comet/rendering/driver/vulkan/vulkan_common_types.h"
#include "comet/resource/model_resource.h"
#include "comet/utils/hash.h"

namespace comet {
namespace rendering {
namespace vk {
using VulkanMeshId = u64;
constexpr auto kInvalidVulkanMeshId{static_cast<VulkanMeshId>(-1)};

struct VertexInputDescription {
  std::vector<VkVertexInputBindingDescription> bindings;
  std::vector<VkVertexInputAttributeDescription> attributes;
  VkPipelineVertexInputStateCreateFlags flags = 0;
};

struct VulkanVertex {
  glm::vec3 position;
  glm::vec3 normal;
  glm::vec3 color;
  glm::vec2 uv;
  static VertexInputDescription GetVertexDescr();

  bool operator==(const VulkanVertex& other) const;
};

using VulkanIndex = u32;

struct VulkanMesh {
  std::vector<VulkanVertex> vertices;
  std::vector<VulkanIndex> indices;
  AllocatedBuffer vertex_buffer;
  VulkanMeshId id{kInvalidVulkanMeshId};
};

VulkanMeshId GenerateMeshId(const resource::MeshResource* resource);
VulkanMesh GenerateMesh(const resource::MeshResource* resource);
}  // namespace vk
}  // namespace rendering
}  // namespace comet

namespace std {
template <>
struct hash<comet::rendering::vk::VulkanVertex> {
  size_t operator()(comet::rendering::vk::VulkanVertex const& vertex) const;
};
}  // namespace std

#endif  // COMET_COMET_RENDERING_DRIVER_VULKAN_VULKAN_MESH_H_
