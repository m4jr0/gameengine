// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "opengl_mesh.h"

#include "GL/glext.h"
#include "glad/glad.h"
#include "glm/gtc/matrix_transform.hpp"

#include "comet/core/engine.h"
#include "comet/rendering/rendering_common.h"
#include "comet/utils/hash.h"
#include "comet/utils/string.h"

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
                                 const resource::MaterialResource& material,
                                 bool is_sampler_anisotropy) {
  RenderProxy proxy{};
  // TODO(m4jr0): Check if copies are necessary here.
  proxy.vertices = &mesh_resource.vertices;
  proxy.indices = &mesh_resource.indices;
  proxy.transform = transform;

  proxy.diffuse_map =
      GenerateTextureMap(material.descr.diffuse_map, is_sampler_anisotropy);
  proxy.specular_map =
      GenerateTextureMap(material.descr.specular_map, is_sampler_anisotropy);
  proxy.normal_map =
      GenerateTextureMap(material.descr.normal_map, is_sampler_anisotropy);

  glGenVertexArrays(1, &proxy.vao);
  glGenBuffers(1, &proxy.vbo);
  glGenBuffers(1, &proxy.ebo);

  glBindVertexArray(proxy.vao);
  glBindBuffer(GL_ARRAY_BUFFER, proxy.vbo);

  glBufferData(GL_ARRAY_BUFFER,
               proxy.vertices->size() * sizeof(rendering::Vertex),
               proxy.vertices->data(), GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, proxy.ebo);

  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
               proxy.indices->size() * sizeof(rendering::Index),
               proxy.indices->data(), GL_STATIC_DRAW);

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

  const auto result{proxies.emplace(entity_id, proxy)};
  COMET_ASSERT(result.second, "Could not add render proxy!");
  return result.first->second;
}

void HandleTextureLabel(rendering::TextureType texture_type,
                        u32 texture_sub_index, schar* texture_label) {
  u8 offset{0};

  switch (texture_type) {
    case rendering::TextureType::Diffuse:
      offset = 15;
      std::memcpy(texture_label, "texture_diffuse", offset);
      break;

    case rendering::TextureType::Specular:
      offset = 16;
      std::memcpy(texture_label, "texture_specular", offset);
      break;

    case rendering::TextureType::Ambient:
      offset = 15;
      std::memcpy(texture_label, "texture_ambient", offset);
      break;

    case rendering::TextureType::Height:
      offset = 14;
      std::memcpy(texture_label, "texture_height", offset);
      break;

    case rendering::TextureType::Unknown:
      offset = 14;
      std::memcpy(texture_label, "texture_unkown", offset);
      break;

    default:
      COMET_LOG_RENDERING_ERROR(
          "Unknown or unsupported texture type: ",
          static_cast<std::underlying_type_t<rendering::TextureType>>(
              texture_type));
  }

  auto* texture_subindex_str{texture_label + offset};
  // Null character is appended.
  std::sprintf(texture_subindex_str, "%u", texture_sub_index);
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

  // TODO(m4jr0): Support other texture types.
  GetShaderProgram().SetInt(proxy.diffuse_map->texture_label, 0);
  glBindTexture(GL_TEXTURE_2D, proxy.diffuse_map->texture_handle);

  glBindVertexArray(proxy.vao);
  glDrawElements(GL_TRIANGLES, proxy.indices->size(), GL_UNSIGNED_INT, 0);
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

TextureMapId GenerateTextureMapId(const resource::TextureMap& texture_map) {
  u32 repeat_mode = static_cast<u32>(texture_map.u_repeat_mode) |
                    (static_cast<u32>(texture_map.v_repeat_mode) << 8) |
                    (static_cast<u32>(texture_map.w_repeat_mode) << 16);
  u16 filter_mode = static_cast<u32>(texture_map.min_filter_mode) |
                    (static_cast<u32>(texture_map.min_filter_mode) << 8);

  auto texture_id{comet::utils::hash::HashCombine(
      texture_map.texture_id, static_cast<u32>(texture_map.type))};
  texture_id = comet::utils::hash::HashCombine(repeat_mode, texture_id);
  texture_id = comet::utils::hash::HashCombine(filter_mode, texture_id);
  texture_id =
      comet::utils::hash::HashCombine(texture_map.texture_id, texture_id);
  return static_cast<TextureMapId>(texture_id);
}

const TextureMap* GenerateTextureMap(
    const resource::TextureMap& resource_texture_map,
    bool is_sampler_anisotropy) {
  auto texture_map_id{GenerateTextureMapId(resource_texture_map)};
  const auto& it{texture_maps.find(texture_map_id)};

  if (it != texture_maps.cend()) {
    return &it->second;
  }

  TextureMap texture_map{};
  texture_map.id = texture_map_id;
  texture_map.u_repeat_mode = GetRepeatMode(resource_texture_map.u_repeat_mode);
  texture_map.v_repeat_mode = GetRepeatMode(resource_texture_map.v_repeat_mode);
  texture_map.min_filter_mode =
      GetFilterMode(resource_texture_map.min_filter_mode);
  texture_map.mag_filter_mode =
      GetFilterMode(resource_texture_map.mag_filter_mode);
  HandleTextureLabel(resource_texture_map.type, 0, texture_map.texture_label);

  const auto resource_id{resource_texture_map.texture_id !=
                                 resource::kInvalidResourceId
                             ? resource_texture_map.texture_id
                             : resource::kDefaultResourceId};

  const auto* resource{
      Engine::Get().GetResourceManager().Load<resource::TextureResource>(
          resource_id)};

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

    case rendering::TextureFormat::Unknown:
      internal_format = GL_RGB8;
      format = GL_RGB8;
      break;  // Default behavior.

    default:
      internal_format = GL_RGB8;
      format = GL_RGB8;

      COMET_ASSERT(
          false, "Unknown or unsupported texture format: ",
          static_cast<std::underlying_type_t<rendering::TextureFormat>>(
              resource->descr.format));
  }

  glGenTextures(1, &texture_map.texture_handle);
  glBindTexture(GL_TEXTURE_2D, texture_map.texture_handle);

  glTexImage2D(GL_TEXTURE_2D, 0, internal_format, resource->descr.resolution[0],
               resource->descr.resolution[1], 0, format, GL_UNSIGNED_BYTE,
               resource->data.data());

  glGenerateMipmap(GL_TEXTURE_2D);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, texture_map.u_repeat_mode);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, texture_map.v_repeat_mode);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                  texture_map.min_filter_mode);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                  texture_map.mag_filter_mode);

  if (is_sampler_anisotropy) {
#ifdef GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT
    GLfloat max_anisotropy;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &max_anisotropy);
    glSamplerParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT,
                        max_anisotropy);
#else
    COMET_LOG_RENDERING_ERROR(
        "Sampler anisotropy is enabled, but current driver does not seem to "
        "support it. Ignoring feature.");
#endif  // GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT
  }

  const auto insert_pair{
      texture_maps.emplace(texture_map.id, std::move(texture_map))};
  COMET_ASSERT(insert_pair.second, "Could not insert texture!");
  return &insert_pair.first->second;
}

void DrawRenderProxies() {
  for (auto& mesh : proxies) {
    DrawRenderProxy(mesh.second);
  }
}

FilterMode GetFilterMode(rendering::TextureFilterMode filter_mode) {
  switch (filter_mode) {
    case rendering::TextureFilterMode::Linear:
      return GL_LINEAR;
    case rendering::TextureFilterMode::Nearest:
      return GL_NEAREST;
    case rendering::TextureFilterMode::Unknown:
      return GL_LINEAR;  // Default behavior.
  }

  COMET_ASSERT(false, "Unknown or unsupported filter mode: ",
               rendering::GetTextureFilterModeLabel(filter_mode), "!");

  return GL_INVALID_ENUM;
}

RepeatMode GetRepeatMode(rendering::TextureRepeatMode repeat_mode) {
  switch (repeat_mode) {
    case rendering::TextureRepeatMode::Repeat:
      return GL_REPEAT;
    case rendering::TextureRepeatMode::MirroredRepeat:
      return GL_MIRRORED_REPEAT;
    case rendering::TextureRepeatMode::ClampToEdge:
      return GL_CLAMP_TO_EDGE;
    case rendering::TextureRepeatMode::ClampToBorder:
      return GL_CLAMP_TO_BORDER;
    case rendering::TextureRepeatMode::Unknown:
      return GL_REPEAT;  // Default behavior.
  }

  COMET_ASSERT(false, "Unknown or unsupported repeat mode: ",
               rendering::GetTextureRepeatModeLabel(repeat_mode), "!");

  return GL_INVALID_ENUM;
}
}  // namespace gl
}  // namespace rendering
}  // namespace comet
