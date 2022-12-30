// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "opengl_mesh.h"

#include "glad/glad.h"
#include "glm/gtc/matrix_transform.hpp"

#include "comet/core/engine.h"
#include "comet/rendering/rendering_common.h"

namespace comet {
namespace rendering {
namespace gl {
ShaderProgram& GetShaderProgram() {
  static std::unique_ptr<ShaderProgram> shader_program{nullptr};

  if (shader_program == nullptr) {
    shader_program = std::make_unique<ShaderProgram>(
        "shaders/opengl/default.gl.vert", "shaders/opengl/default.gl.frag");
  }

  return *shader_program;
}

RenderProxy& GenerateRenderProxy(entity::EntityId entity_id,
                                 const resource::MeshResource& mesh_resource,
                                 const glm::mat4& transform,
                                 const resource::MaterialResource& material) {
  RenderProxy proxy{};

  // TODO(m4jr0): Check if copies are necessary here.
  proxy.vertices = mesh_resource.vertices;
  proxy.indices = mesh_resource.indices;
  proxy.texture_tuples = material.texture_tuples;
  const auto texture_count{material.texture_tuples.size()};
  proxy.texture_ids.resize(texture_count);
  proxy.transform = transform;
  auto& resource_manager{Engine::Get().GetResourceManager()};

  for (uindex i{0}; i < texture_count; ++i) {
    proxy.texture_ids[i] = LoadTexture(
        resource_manager.LoadFromResourceId<resource::TextureResource>(
            material.texture_tuples[i].texture_id));
  }

  glGenVertexArrays(1, &proxy.vao);
  glGenBuffers(1, &proxy.vbo);
  glGenBuffers(1, &proxy.ebo);

  glBindVertexArray(proxy.vao);
  glBindBuffer(GL_ARRAY_BUFFER, proxy.vbo);

  glBufferData(GL_ARRAY_BUFFER,
               proxy.vertices.size() * sizeof(rendering::Vertex),
               &proxy.vertices[0], GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, proxy.ebo);

  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
               proxy.indices.size() * sizeof(rendering::Index),
               &proxy.indices[0], GL_STATIC_DRAW);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(rendering::Vertex),
                        reinterpret_cast<void*>(0));

  glEnableVertexAttribArray(1);

  glVertexAttribPointer(
      1, 3, GL_FLOAT, GL_FALSE, sizeof(rendering::Vertex),
      reinterpret_cast<void*>(offsetof(rendering::Vertex, normal)));

  glEnableVertexAttribArray(2);

  glVertexAttribPointer(
      2, 3, GL_FLOAT, GL_FALSE, sizeof(rendering::Vertex),
      reinterpret_cast<void*>(offsetof(rendering::Vertex, tangent)));

  glEnableVertexAttribArray(3);

  glVertexAttribPointer(
      3, 3, GL_FLOAT, GL_FALSE, sizeof(rendering::Vertex),
      reinterpret_cast<void*>(offsetof(rendering::Vertex, bitangent)));

  glEnableVertexAttribArray(4);

  glVertexAttribPointer(
      4, 2, GL_FLOAT, GL_FALSE, sizeof(rendering::Vertex),
      reinterpret_cast<void*>(offsetof(rendering::Vertex, uv)));

  glBindVertexArray(0);

  const auto result{proxies.emplace(entity_id, std::move(proxy))};
  COMET_ASSERT(result.second, "Could not add render proxy!");
  return result.first->second;
}

std::string GetTextureLabel(rendering::TextureType texture_type,
                            uindex texture_subindex) {
  switch (texture_type) {
    case rendering::TextureType::Diffuse:
      return "texture_diffuse" + texture_subindex;

    case rendering::TextureType::Specular:
      return "texture_specular" + texture_subindex;

    case rendering::TextureType::Ambient:
      return "texture_ambient" + texture_subindex;

    case rendering::TextureType::Height:
      return "texture_height" + texture_subindex;

    default:
      COMET_LOG_RENDERING_ERROR(
          "Unknown texture type: ",
          static_cast<std::underlying_type_t<rendering::TextureType>>(
              texture_type));
      return "";
  }
}

void DrawRenderProxy(RenderProxy proxy) {
  auto nearest_point{0.1f};
  auto farthest_point{100.0f};
  auto fov{45.0f};

  auto width{Engine::Get().GetRenderingManager().GetWindow()->GetWidth()};
  auto height{Engine::Get().GetRenderingManager().GetWindow()->GetHeight()};

  auto projection_matrix{glm::perspective(
      glm::radians(fov), static_cast<f32>(width) / static_cast<f32>(height),
      nearest_point, farthest_point)};

  auto position{glm::vec3(0, 18, 15)};
  auto direction{glm::vec3(0, 0, -15)};
  auto orientation{glm::vec3(0, 1, 0)};

  auto view_matrix{glm::lookAt(position, direction, orientation)};

  const auto mvp{projection_matrix * view_matrix * proxy.transform};
  auto& shader_program{GetShaderProgram()};

  shader_program.SetMatrix4("mvp", mvp);
  shader_program.Use();

  uindex diffuse_index{0};
  uindex specular_index{0};
  uindex normal_index{0};
  uindex height_index{0};
  const auto texture_count{proxy.texture_tuples.size()};

  for (uindex texture_index{0}; texture_index < texture_count;
       ++texture_index) {
    glActiveTexture(GL_TEXTURE0 + texture_index);
    uindex texture_subindex;
    const auto texture_type{proxy.texture_tuples[texture_index].type};

    switch (texture_type) {
      case rendering::TextureType::Diffuse:
        texture_subindex = ++diffuse_index;
        break;

      case rendering::TextureType::Specular:
        texture_subindex = ++specular_index;
        break;

      case rendering::TextureType::Ambient:
        texture_subindex = ++normal_index;
        break;

      case rendering::TextureType::Height:
        texture_subindex = ++height_index;
        break;

      default:
        COMET_LOG_RENDERING_ERROR(
            "Unknown texture type: ",
            static_cast<std::underlying_type_t<rendering::TextureType>>(
                texture_type));
        continue;
    }

    GetShaderProgram().SetInt(
        GetTextureLabel(texture_type, texture_subindex).c_str(), texture_index);

    glBindTexture(GL_TEXTURE_2D, proxy.texture_ids[texture_index]);
  }

  glBindVertexArray(proxy.vao);
  glDrawElements(GL_TRIANGLES, proxy.indices.size(), GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);

  glActiveTexture(GL_TEXTURE0);
}

bool IsRenderProxy(entity::EntityId entity_id) {
  const auto it{proxies.find(entity_id)};
  return it != proxies.cend();
}

RenderProxy* TryGetRenderProxy(entity::EntityId entity_id) {
  const auto it{proxies.find(entity_id)};
  if (it == proxies.cend()) {
    return nullptr;
  }

  return &it->second;
}

void ClearRenderProxies() {
  if (loaded_texture_ids.size() > 0) {
    glDeleteTextures(loaded_texture_ids.size(), loaded_texture_ids.data());
  }
}

void InitializeShader() { GetShaderProgram().Initialize(); }

u32 LoadTexture(const resource::TextureResource* resource) {
  for (uindex i{0}; i < loaded_texture_resource_ids.size(); ++i) {
    if (resource->id == loaded_texture_resource_ids[i]) {
      // Texture is already loaded.
      return loaded_texture_ids[i];
    }
  }

  GLenum internal_format;
  GLenum format;

  switch (resource->descr.format) {
    case rendering::TextureFormat::Rgba8:
      internal_format = GL_RGBA8;
      format = GL_RGBA;
      break;

    case rendering::TextureFormat::Rgb8:
      internal_format = GL_RGB8;
      format = GL_RGB;
      break;

    default:
      internal_format = GL_RGBA8;
      format = GL_RGBA;

      COMET_LOG_RENDERING_ERROR(
          "Unsupported texture format: ",
          static_cast<std::underlying_type_t<rendering::TextureFormat>>(
              resource->descr.format));
  }

  u32 texture_id{0};
  glGenTextures(1, &texture_id);
  glBindTexture(GL_TEXTURE_2D, texture_id);

  glTexImage2D(GL_TEXTURE_2D, 0, internal_format, resource->descr.resolution[0],
               resource->descr.resolution[1], 0, format, GL_UNSIGNED_BYTE,
               resource->data.data());

  glGenerateMipmap(GL_TEXTURE_2D);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                  GL_LINEAR_MIPMAP_LINEAR);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  loaded_texture_ids.emplace_back(texture_id);
  return texture_id;
}

void DrawRenderProxies() {
  for (auto& mesh : proxies) {
    DrawRenderProxy(mesh.second);
  }
}
}  // namespace gl
}  // namespace rendering
}  // namespace comet
