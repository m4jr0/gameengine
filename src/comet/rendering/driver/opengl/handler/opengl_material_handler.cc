// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"

#include "opengl_material_handler.h"

#include "comet/core/type/array.h"
#include "comet/profiler/profiler.h"
#include "comet/resource/texture_resource.h"

namespace comet {
namespace rendering {
namespace gl {
MaterialHandler::MaterialHandler(const MaterialHandlerDescr& descr)
    : Handler{descr}, texture_handler_{descr.texture_handler} {
  COMET_ASSERT(texture_handler_ != nullptr, "Texture handler is null!");
}

void MaterialHandler::Initialize() {
  Handler::Initialize();
  allocator_.Initialize();
  materials_ = Map<MaterialId, Material*>{&allocator_};
}

void MaterialHandler::Shutdown() {
  materials_.Clear();

  for (auto& it : materials_) {
    Destroy(it.value, true);
  }

  materials_.Clear();
  allocator_.Destroy();
  Handler::Shutdown();
}

Material* MaterialHandler::Generate(const MaterialDescr& descr) {
  COMET_PROFILE("MaterialHandler::Generate");
  auto* material{allocator_.AllocateOneAndPopulate<Material>()};
  material->id = descr.id;
  material->shader_id = descr.shader_id;
  material->diffuse_map = descr.diffuse_map;
  material->specular_map = descr.specular_map;
  material->normal_map = descr.normal_map;
  return materials_.Emplace(material->id, material).value;
}

Material* MaterialHandler::Generate(
    const resource::MaterialResource* resource) {
  MaterialDescr descr{};
  descr.id = resource->id;
  descr.diffuse_map = GenerateTextureMap(&resource->descr.diffuse_map);
  descr.specular_map = GenerateTextureMap(&resource->descr.specular_map);
  descr.normal_map = GenerateTextureMap(&resource->descr.normal_map);

  TString shader_path{};
  shader_path.Reserve(resource::kMaxShaderNameLen);
  COMET_DISALLOW_STR_ALLOC(shader_path);
  shader_path += COMET_TCHAR("shaders/opengl/");
  shader_path += GetTmpTChar(resource->descr.shader_name);
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
  auto** material{materials_.TryGet(material_id)};

  if (material == nullptr) {
    return nullptr;
  }

  return *material;
}

Material* MaterialHandler::GetOrGenerate(const MaterialDescr& descr) {
  auto* material{TryGet(descr.id)};

  if (material != nullptr) {
    return material;
  }

  return Generate(descr);
}

Material* MaterialHandler::GetOrGenerate(
    const resource::MaterialResource* resource) {
  auto* material{TryGet(resource->id)};

  if (material != nullptr) {
    return material;
  }

  return Generate(resource);
}

void MaterialHandler::Destroy(MaterialId material_id) {
  return Destroy(Get(material_id));
}

void MaterialHandler::Destroy(Material* material) {
  return Destroy(material, false);
}

TextureMap MaterialHandler::GenerateTextureMap(
    const resource::TextureMap* map) {
  TextureMap texture_map{};
  texture_map.type = GetTextureType(map->type);

  const auto resource_id{map->texture_id != resource::kInvalidResourceId
                             ? map->texture_id
                             : resource::GetDefaultTextureFromType(map->type)};

  const auto* resource{
      resource::ResourceManager::Get().Load<resource::TextureResource>(
          resource_id)};

  texture_map.texture_handle =
      texture_handler_->GetOrGenerate(resource)->handle;

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                  GetRepeatMode(map->u_repeat_mode));
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
                  GetRepeatMode(map->v_repeat_mode));

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                  GetFilterMode(map->min_filter_mode));
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                  GetFilterMode(map->mag_filter_mode));

  return texture_map;
}

void MaterialHandler::Destroy(Material* material, bool is_destroying_handler) {
  COMET_PROFILE("MaterialHandler::Destroy");

  if (!is_destroying_handler) {
    StaticArray<TextureMap*, 3> texture_maps = {
        &material->diffuse_map, &material->specular_map, &material->normal_map};

    for (auto* texture_map : texture_maps) {
      *texture_map = {};
    }

    materials_.Remove(material->id);
    allocator_.Deallocate(material);
  }
}

TextureType MaterialHandler::GetTextureType(
    rendering::TextureType texture_type) {
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
}  // namespace gl
}  // namespace rendering
}  // namespace comet
