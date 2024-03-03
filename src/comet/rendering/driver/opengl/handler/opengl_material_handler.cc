// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "opengl_material_handler.h"

#include "comet/rendering/driver/opengl/data/opengl_frame.h"
#include "comet/resource/texture_resource.h"

namespace comet {
namespace rendering {
namespace gl {
MaterialHandler::MaterialHandler(const MaterialHandlerDescr& descr)
    : Handler{descr},
      texture_handler_{descr.texture_handler},
      shader_handler_{descr.shader_handler} {
  COMET_ASSERT(texture_handler_ != nullptr, "Texture handler is null!");
  COMET_ASSERT(shader_handler_ != nullptr, "Shader handler is null!");
}

void MaterialHandler::Shutdown() {
  materials_.clear();

  if (shader_handler_->IsInitialized()) {
    for (auto& it : materials_) {
      Destroy(it.second, true);
    }
  }

  materials_.clear();
  Handler::Shutdown();
}

Material* MaterialHandler::Generate(const MaterialDescr& descr) {
  return GenerateInternal(descr);
}

Material* MaterialHandler::Generate(
    const resource::MaterialResource& resource) {
  MaterialDescr descr{};
  descr.id = resource.id;
  descr.diffuse_map = GenerateTextureMap(resource.descr.diffuse_map);
  descr.specular_map = GenerateTextureMap(resource.descr.specular_map);
  descr.normal_map = GenerateTextureMap(resource.descr.normal_map);

  TString shader_path{};
  shader_path.Reserve(resource::kMaxShaderNameLen);
  COMET_DISALLOW_STR_ALLOC(shader_path);
  shader_path += COMET_TCHAR("shaders/opengl/");
  shader_path += GetTmpTChar(resource.descr.shader_name);
  shader_path += COMET_TCHAR(".gl.cshader");
  descr.shader_id = resource::GenerateResourceIdFromPath(shader_path);

  return Generate(descr);
}

Material* MaterialHandler::Get(MaterialId material_id) {
  auto* material{TryGet(material_id)};
  COMET_ASSERT(material != nullptr,
               "Requested material does not exist: ", material_id, "!");
  return material;
}

Material* MaterialHandler::TryGet(MaterialId material_id) {
  auto it{materials_.find(material_id)};

  if (it == materials_.end()) {
    return nullptr;
  }

  return &it->second;
}

Material* MaterialHandler::GetOrGenerate(const MaterialDescr& descr) {
  auto* material{TryGet(descr.id)};

  if (material != nullptr) {
    return material;
  }

  return Generate(descr);
}

Material* MaterialHandler::GetOrGenerate(
    const resource::MaterialResource& resource) {
  auto* material{TryGet(resource.id)};

  if (material != nullptr) {
    return material;
  }

  return Generate(resource);
}

void MaterialHandler::Destroy(MaterialId material_id) {
  return Destroy(*Get(material_id));
}

void MaterialHandler::Destroy(Material& material) {
  return Destroy(material, false);
}

void MaterialHandler::UpdateInstance(Material& material, ShaderId shader_id,
                                     FrameIndex frame_count) {
  auto* shader_ptr{shader_handler_->Get(shader_id)};
  COMET_ASSERT(shader_ptr != nullptr, "Material's shader cannot be null!");
  auto& shader{*shader_ptr};
  shader_handler_->BindInstance(shader, material.instance_id);
  auto is_udpate{material.instance_update_frame != frame_count};

  shader_handler_->SetUniform(shader, shader.uniform_indices.diffuse_map,
                              &material.diffuse_map, is_udpate);
  shader_handler_->SetUniform(shader, shader.uniform_indices.specular_map,
                              &material.specular_map, is_udpate);
  shader_handler_->SetUniform(shader, shader.uniform_indices.normal_map,
                              &material.normal_map, is_udpate);
  shader_handler_->SetUniform(shader, shader.uniform_indices.diffuse_color,
                              &material.diffuse_color, is_udpate);
  shader_handler_->SetUniform(shader, shader.uniform_indices.shininess,
                              &material.shininess, is_udpate);

  material.instance_update_frame = frame_count;
}

FilterMode MaterialHandler::GetFilterMode(TextureFilterMode filter_mode) {
  switch (filter_mode) {
    case TextureFilterMode::Linear:
      return GL_LINEAR;
    case TextureFilterMode::Nearest:
      return GL_NEAREST;
    case TextureFilterMode::Unknown:
      return GL_LINEAR;  // Default behavior.
  }

  COMET_ASSERT(false, "Unknown or unsupported filter mode: ",
               GetTextureFilterModeLabel(filter_mode), "!");

  return GL_INVALID_ENUM;
}

RepeatMode MaterialHandler::GetRepeatMode(TextureRepeatMode repeat_mode) {
  switch (repeat_mode) {
    case TextureRepeatMode::Repeat:
      return GL_REPEAT;
    case TextureRepeatMode::MirroredRepeat:
      return GL_MIRRORED_REPEAT;
    case TextureRepeatMode::ClampToEdge:
      return GL_CLAMP_TO_EDGE;
    case TextureRepeatMode::ClampToBorder:
      return GL_CLAMP_TO_BORDER;
    case TextureRepeatMode::Unknown:
      return GL_REPEAT;  // Default behavior.
  }

  COMET_ASSERT(false, "Unknown or unsupported repeat mode: ",
               GetTextureRepeatModeLabel(repeat_mode), "!");

  return GL_INVALID_ENUM;
}

TextureType MaterialHandler::GetTextureType(
    rendering::TextureType texture_type) {
  u8 offset{0};

  switch (texture_type) {
    case rendering::TextureType::Diffuse:
      return TextureType::Diffuse;

    case rendering::TextureType::Specular:
      return TextureType::Specular;

    case rendering::TextureType::Normal:
      return TextureType::Normal;

    default:
      COMET_LOG_RENDERING_ERROR(
          "Unknown or unsupported texture type: ",
          static_cast<std::underlying_type_t<TextureType>>(texture_type), "!");
      return TextureType::Invalid;
  }
}

void MaterialHandler::Destroy(Material& material, bool is_destroying_handler) {
  if (shader_handler_->IsInitialized() &&
      shader_handler_->HasMaterial(material)) {
    shader_handler_->UnbindMaterial(material);
  }

  material.shininess = .0f;
  material.instance_update_frame = kInvalidFrameIndex;
  material.id = kInvalidMaterialId;
  material.shader_id = kInvalidShaderId;
  material.diffuse_color = {kColorWhite, 1.0f};

  if (!is_destroying_handler) {
    std::array<TextureMap*, 3> texture_maps = {
        &material.diffuse_map, &material.specular_map, &material.normal_map};

    for (auto* texture_map : texture_maps) {
      *texture_map = {};
    }
    materials_.erase(material.id);
  }
}

Material* MaterialHandler::GenerateInternal(const MaterialDescr& descr) {
  Material material;
  material.id = descr.id;
  material.shader_id = descr.shader_id;
  material.diffuse_map = descr.diffuse_map;
  material.specular_map = descr.specular_map;
  material.normal_map = descr.normal_map;

  auto insert_pair{materials_.emplace(material.id, material)};
  COMET_ASSERT(insert_pair.second, "Could not insert material: ",
               COMET_STRING_ID_LABEL(material.id), "!");
  shader_handler_->BindMaterial(insert_pair.first->second);
  return &insert_pair.first->second;
}

TextureMap MaterialHandler::GenerateTextureMap(
    const resource::TextureMap& map) {
  TextureMap texture_map{};
  texture_map.type = GetTextureType(map.type);

  const auto resource_id{map.texture_id != resource::kInvalidResourceId
                             ? map.texture_id
                             : resource::GetDefaultTextureFromType(map.type)};

  const auto* resource{
      resource::ResourceManager::Get().Load<resource::TextureResource>(
          resource_id)};

  texture_map.texture_handle =
      texture_handler_->GetOrGenerate(resource)->handle;

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                  GetRepeatMode(map.u_repeat_mode));
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
                  GetRepeatMode(map.v_repeat_mode));

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                  GetFilterMode(map.min_filter_mode));
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                  GetFilterMode(map.mag_filter_mode));

  return texture_map;
}
}  // namespace gl
}  // namespace rendering
}  // namespace comet
