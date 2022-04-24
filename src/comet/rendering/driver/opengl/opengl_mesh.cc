// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "opengl_mesh.h"

#include "glad/glad.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include "comet/core/engine.h"

namespace comet {
namespace rendering {
namespace gl {
MeshProxy& GenerateMeshProxy(entity::EntityId entity_id, entity::Mesh mesh,
                             const entity::Texture textures[],
                             uindex texture_count) {
  MeshProxy proxy{};

  proxy.vertices = mesh->vertices;
  proxy.indices = mesh->indices;
  proxy.texture_tuples = mesh->textures;
  proxy.texture_ids.resize(texture_count);

  for (uindex i{0}; i < texture_count; ++i) {
    proxy.texture_ids[i] = LoadTexture(textures[i]);
  }

  glGenVertexArrays(1, &proxy.vao);
  glGenBuffers(1, &proxy.vbo);
  glGenBuffers(1, &proxy.ebo);

  glBindVertexArray(proxy.vao);
  glBindBuffer(GL_ARRAY_BUFFER, proxy.vbo);

  glBufferData(GL_ARRAY_BUFFER, proxy.vertices.size() * sizeof(entity::Vertex),
               &proxy.vertices[0], GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, proxy.ebo);

  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
               proxy.indices.size() * sizeof(entity::Index), &proxy.indices[0],
               GL_STATIC_DRAW);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(entity::Vertex),
                        reinterpret_cast<void*>(0));

  glEnableVertexAttribArray(1);

  glVertexAttribPointer(
      1, 3, GL_FLOAT, GL_FALSE, sizeof(entity::Vertex),
      reinterpret_cast<void*>(offsetof(entity::Vertex, normal)));

  glEnableVertexAttribArray(2);

  glVertexAttribPointer(
      2, 3, GL_FLOAT, GL_FALSE, sizeof(entity::Vertex),
      reinterpret_cast<void*>(offsetof(entity::Vertex, tangent)));

  glEnableVertexAttribArray(3);

  glVertexAttribPointer(
      3, 3, GL_FLOAT, GL_FALSE, sizeof(entity::Vertex),
      reinterpret_cast<void*>(offsetof(entity::Vertex, bitangent)));

  glEnableVertexAttribArray(4);

  glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, sizeof(entity::Vertex),
                        reinterpret_cast<void*>(offsetof(entity::Vertex, uv)));

  glBindVertexArray(0);

  proxies.emplace(entity_id, std::move(proxy));
  return proxies[entity_id];
}

std::string GetTextureLabel(resource::texture::TextureType texture_type,
                            uindex texture_subindex) {
  switch (texture_type) {
    case resource::texture::TextureType::Diffuse:
      return "texture_diffuse" + texture_subindex;

    case resource::texture::TextureType::Specular:
      return "texture_specular" + texture_subindex;

    case resource::texture::TextureType::Ambient:
      return "texture_ambient" + texture_subindex;

    case resource::texture::TextureType::Height:
      return "texture_height" + texture_subindex;

    default:
      COMET_LOG_RENDERING_ERROR(
          "Unknown texture type: ",
          static_cast<std::underlying_type_t<resource::texture::TextureType>>(
              texture_type));
      return "";
  }
}

void DrawMeshProxy(MeshProxy proxy) {
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
      case resource::texture::TextureType::Diffuse:
        texture_subindex = ++diffuse_index;
        break;

      case resource::texture::TextureType::Specular:
        texture_subindex = ++specular_index;
        break;

      case resource::texture::TextureType::Ambient:
        texture_subindex = ++normal_index;
        break;

      case resource::texture::TextureType::Height:
        texture_subindex = ++height_index;
        break;

      default:
        COMET_LOG_RENDERING_ERROR(
            "Unknown texture type: ",
            static_cast<std::underlying_type_t<resource::texture::TextureType>>(
                texture_type));
        continue;
    }

    shader_program.SetInt(
        GetTextureLabel(texture_type, texture_subindex).c_str(), texture_index);

    glBindTexture(GL_TEXTURE_2D, proxy.texture_ids[texture_index]);
  }

  glBindVertexArray(proxy.vao);
  glDrawElements(GL_TRIANGLES, proxy.indices.size(), GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);

  glActiveTexture(GL_TEXTURE0);
}

bool IsMeshProxy(entity::EntityId entity_id) {
  const auto it{proxies.find(entity_id)};
  return it != proxies.cend();
}

void ClearMeshProxies() {
  if (loaded_texture_ids.size() > 0) {
    glDeleteTextures(loaded_texture_ids.size(), loaded_texture_ids.data());
  }
}

void InitializeShader() { shader_program.Initialize(); }

u32 LoadTexture(entity::Texture texture) {
  for (uindex i{0}; i < loaded_texture_resource_ids.size(); ++i) {
    if (texture->id == loaded_texture_resource_ids[i]) {
      // Texture is already loaded.
      return loaded_texture_ids[i];
    }
  }

  GLenum internal_format;
  GLenum format;

  switch (texture->descr.format) {
    case resource::texture::TextureFormat::Rgba8:
      internal_format = GL_RGBA8;
      format = GL_RGBA;
      break;

    case resource::texture::TextureFormat::Rgb8:
      internal_format = GL_RGB8;
      format = GL_RGB;
      break;

    default:
      internal_format = GL_RGBA8;
      format = GL_RGBA;

      COMET_LOG_RENDERING_ERROR(
          "Unsupported texture format: ",
          static_cast<std::underlying_type_t<resource::texture::TextureFormat>>(
              texture->descr.format));
  }

  u32 texture_id{0};
  glGenTextures(1, &texture_id);
  glBindTexture(GL_TEXTURE_2D, texture_id);

  glTexImage2D(GL_TEXTURE_2D, 0, internal_format, texture->descr.resolution[0],
               texture->descr.resolution[1], 0, format, GL_UNSIGNED_BYTE,
               texture->data.data());

  glGenerateMipmap(GL_TEXTURE_2D);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                  GL_LINEAR_MIPMAP_LINEAR);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  loaded_texture_ids.emplace_back(texture_id);
  return texture_id;
}

void DrawMeshProxies() {
  auto nearest_point_{0.1f};
  auto farthest_point_{100.0f};
  auto fov_{45.0f};

  auto width{Engine::Get().GetRenderingManager().GetWindow()->GetWidth()};
  auto height{Engine::Get().GetRenderingManager().GetWindow()->GetHeight()};

  auto projection_matrix{glm::perspective(
      glm::radians(fov_), static_cast<f32>(width) / static_cast<f32>(height),
      nearest_point_, farthest_point_)};

  auto position{glm::vec3(0, 18, 15)};
  auto direction{glm::vec3(0, 0, -15)};
  auto orientation{glm::vec3(0, 1, 0)};

  auto view_matrix{glm::lookAt(position, direction, orientation)};

  auto model_matrix{
      glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f))};

  const auto mvp{projection_matrix * view_matrix * model_matrix};

  shader_program.SetMatrix4("mvp", mvp);
  shader_program.Use();

  for (auto& mesh : proxies) {
    DrawMeshProxy(mesh.second);
  }
}
}  // namespace gl
}  // namespace rendering
}  // namespace comet
