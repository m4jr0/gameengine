// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "vulkan_mesh.h"

namespace comet {
namespace rendering {
namespace vk {
VertexInputDescription VulkanVertex::GetVertexDescr() {
  VertexInputDescription descr{};

  VkVertexInputBindingDescription main_binding{};
  main_binding.binding = 0;
  main_binding.stride = sizeof(VulkanVertex);
  main_binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

  descr.bindings.push_back(main_binding);

  VkVertexInputAttributeDescription position_attr{};
  position_attr.binding = 0;
  position_attr.location = 0;
  position_attr.format = VK_FORMAT_R32G32B32_SFLOAT;
  position_attr.offset = offsetof(VulkanVertex, position);

  VkVertexInputAttributeDescription normal_attr{};
  normal_attr.binding = 0;
  normal_attr.location = 1;
  normal_attr.format = VK_FORMAT_R32G32B32_SFLOAT;
  normal_attr.offset = offsetof(VulkanVertex, normal);

  VkVertexInputAttributeDescription color_attr{};
  color_attr.binding = 0;
  color_attr.location = 2;
  color_attr.format = VK_FORMAT_R32G32B32_SFLOAT;
  color_attr.offset = offsetof(VulkanVertex, color);

  VkVertexInputAttributeDescription uv_attr{};
  uv_attr.binding = 0;
  uv_attr.location = 3;
  uv_attr.format = VK_FORMAT_R32G32_SFLOAT;
  uv_attr.offset = offsetof(VulkanVertex, uv);

  descr.attributes.push_back(position_attr);
  descr.attributes.push_back(normal_attr);
  descr.attributes.push_back(color_attr);
  descr.attributes.push_back(uv_attr);

  return descr;
}

bool VulkanVertex::operator==(const VulkanVertex& other) const {
  return position == other.position && normal == other.normal &&
         color == other.color && uv == other.uv;
}

VulkanMeshId GenerateMeshId(const resource::MeshResource* resource) {
  return static_cast<u64>(resource->internal_id) |
         static_cast<u64>(resource->resource_id) << 32;
}

VulkanMesh GenerateMesh(const resource::MeshResource* resource) {
  VulkanMesh mesh{};
  mesh.id = GenerateMeshId(resource);

  const auto vertex_count{resource->vertices.size()};
  mesh.vertices.resize(resource->vertices.size());

  for (uindex i{0}; i < vertex_count; ++i) {
    auto& vertex{mesh.vertices[i]};
    auto& vertex_res{resource->vertices[i]};

    vertex.position = vertex_res.position;
    vertex.normal = vertex_res.normal;

    vertex.uv.x = vertex_res.uv.x;
    vertex.uv.y = vertex_res.uv.y;

    vertex.color = {1.0f, 1.0f, 1.0f};
  }

  const auto index_count{resource->indices.size()};
  mesh.indices.resize(resource->indices.size());

  for (uindex i{0}; i < index_count; ++i) {
    mesh.indices[i] = resource->indices[i];
  }

  return mesh;
}
}  // namespace vk
}  // namespace rendering
}  // namespace comet

namespace std {
size_t hash<comet::rendering::vk::VulkanVertex>::operator()(
    comet::rendering::vk::VulkanVertex const& vertex) const {
  return comet::utils::hash::HashCombine(
      hash<glm::vec3>()(vertex.position),
      comet::utils::hash::HashCombine(
          hash<glm::vec3>()(vertex.normal),
          comet::utils::hash::HashCombine(hash<glm::vec3>()(vertex.color),
                                          hash<glm::vec2>()(vertex.uv))));
}
}  // namespace std
