// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_VULKAN_VULKAN_MESH_H_
#define COMET_COMET_RENDERING_DRIVER_VULKAN_VULKAN_MESH_H_

#include "comet_precompile.h"

#include "glm/glm.hpp"
#include "glm/gtx/hash.hpp"
#include "vulkan/vulkan.h"

#include "comet/rendering/driver/vulkan/vulkan_types.h"

namespace comet {
namespace rendering {
namespace vk {
struct VertexInputDescription {
  std::vector<VkVertexInputBindingDescription> bindings;
  std::vector<VkVertexInputAttributeDescription> attributes;
  VkPipelineVertexInputStateCreateFlags flags = 0;
};

struct Vertex {
  glm::vec3 position;
  glm::vec3 normal;
  glm::vec3 color;
  glm::vec2 uv;
  static VertexInputDescription GetVertexDescription();

  bool operator==(const Vertex& other) const;
};

struct Mesh {
  std::vector<Vertex> vertices;
  std::vector<u32> indices;
  AllocatedBuffer vertex_buffer;
  bool LoadFromObj(const std::string& path);
};
}  // namespace vk
}  // namespace rendering
}  // namespace comet

namespace std {
template <>
struct hash<comet::rendering::vk::Vertex> {
  size_t operator()(comet::rendering::vk::Vertex const& vertex) const {
    return ((hash<glm::vec3>()(vertex.position) ^
             (hash<glm::vec3>()(vertex.color) << 1)) >>
            1) ^
           (hash<glm::vec2>()(vertex.uv) << 1);
  }
};
}  // namespace std

#endif  // COMET_COMET_RENDERING_DRIVER_VULKAN_VULKAN_MESH_H_
