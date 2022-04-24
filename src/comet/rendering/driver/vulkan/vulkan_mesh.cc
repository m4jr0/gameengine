// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "vulkan_mesh.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

namespace comet {
namespace rendering {
namespace vk {
VertexInputDescription Vertex::GetVertexDescription() {
  VertexInputDescription description;

  VkVertexInputBindingDescription main_binding{};
  main_binding.binding = 0;
  main_binding.stride = sizeof(Vertex);
  main_binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

  description.bindings.push_back(main_binding);

  VkVertexInputAttributeDescription position_attr{};
  position_attr.binding = 0;
  position_attr.location = 0;
  position_attr.format = VK_FORMAT_R32G32B32_SFLOAT;
  position_attr.offset = offsetof(Vertex, position);

  VkVertexInputAttributeDescription normal_attr{};
  normal_attr.binding = 0;
  normal_attr.location = 1;
  normal_attr.format = VK_FORMAT_R32G32B32_SFLOAT;
  normal_attr.offset = offsetof(Vertex, normal);

  VkVertexInputAttributeDescription color_attr{};
  color_attr.binding = 0;
  color_attr.location = 2;
  color_attr.format = VK_FORMAT_R32G32B32_SFLOAT;
  color_attr.offset = offsetof(Vertex, color);

  VkVertexInputAttributeDescription uv_attr{};
  uv_attr.binding = 0;
  uv_attr.location = 3;
  uv_attr.format = VK_FORMAT_R32G32_SFLOAT;
  uv_attr.offset = offsetof(Vertex, uv);

  description.attributes.push_back(position_attr);
  description.attributes.push_back(normal_attr);
  description.attributes.push_back(color_attr);
  description.attributes.push_back(uv_attr);

  return description;
}

bool Vertex::operator==(const Vertex& other) const {
  return position == other.position && normal == other.normal &&
         color == other.color && uv == other.uv;
}

bool Mesh::LoadFromObj(const std::string& path) {
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;

  std::string warning_msg;
  std::string error_msg;

  tinyobj::LoadObj(&attrib, &shapes, &materials, &warning_msg, &error_msg,
                   path.c_str(), nullptr);

  if (!warning_msg.empty()) {
    COMET_LOG_RENDERING_WARNING("tinyobjloader: ", warning_msg);
  }

  if (!error_msg.empty()) {
    COMET_LOG_RENDERING_ERROR("tinyobjloader: ", error_msg);
    return false;
  }
  std::unordered_map<Vertex, u32> unique_vertices{};

  for (const auto& shape : shapes) {
    for (const auto& uindex : shape.mesh.indices) {
      Vertex vertex{};

      vertex.position = {
          static_cast<u32>(attrib.vertices[3 * uindex.vertex_index]),
          static_cast<u32>(attrib.vertices[3 * uindex.vertex_index + 1]),
          static_cast<u32>(attrib.vertices[3 * uindex.vertex_index + 2])};

      vertex.normal = {
          static_cast<u32>(attrib.normals[3 * uindex.normal_index]),
          static_cast<u32>(attrib.normals[3 * uindex.normal_index + 1]),
          static_cast<u32>(attrib.normals[3 * uindex.normal_index + 2])};

      vertex.uv = {
          static_cast<u32>(attrib.texcoords[3 * uindex.texcoord_index]),
          static_cast<u32>(1 -  // Set Y coordinate according to Vulkan.
                           attrib.texcoords[3 * uindex.texcoord_index + 1])};

      vertex.color = {1.0f, 1.0f, 1.0f};

      if (unique_vertices.count(vertex) == 0) {
        unique_vertices[vertex] = static_cast<u32>(vertices.size());
        vertices.push_back(vertex);
      }

      indices.push_back(unique_vertices[vertex]);
    }
  }

  return true;
}
}  // namespace vk
}  // namespace rendering
}  // namespace comet
