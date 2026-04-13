// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "opengl_material_handler.h"
////////////////////////////////////////////////////////////////////////////////

// External. ///////////////////////////////////////////////////////////////////
#include <functional>
#include <type_traits>
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/memory/allocator/allocator.h"
#include "comet/profiler/profiler.h"
#include "comet/resource/resource_manager.h"
#include "comet/resource/texture_resource.h"

namespace comet {
namespace rendering {
namespace gl {
namespace internal {
SamplerId GenerateSamplerId(const resource::TextureMap& texture_map) {
  HashValue hash{0};
  hash = HashCombine(hash, static_cast<HashValue>(texture_map.u_repeat_mode));
  hash = HashCombine(hash, static_cast<HashValue>(texture_map.v_repeat_mode));
  hash = HashCombine(hash, static_cast<HashValue>(texture_map.min_filter_mode));
  hash = HashCombine(hash, static_cast<HashValue>(texture_map.mag_filter_mode));
  return static_cast<SamplerId>(hash);
}
}  // namespace internal

MaterialHandler::MaterialHandler(const MaterialHandlerDescr& descr)
    : Handler{descr}, texture_handler_{descr.texture_handler} {
  COMET_ASSERT(texture_handler_ != nullptr, "Texture handler is null!");
}

void MaterialHandler::Initialize() {
  Handler::Initialize();
  allocator_.Initialize();
  materials_ = Map<MaterialId, Material*>{&allocator_};
  samplers_ = Map<SamplerId, Sampler*>{&allocator_};
}

void MaterialHandler::Shutdown() {
  for (auto& it : materials_) {
    Destroy(it.value, true);
  }

  COMET_ASSERT(samplers_.IsEmpty(), "Sampler cache is not empty on shutdown!");

  materials_.Destroy();
  samplers_.Destroy();
  allocator_.Destroy();

  destroy_callback_ = nullptr;
  destroy_callback_user_data_ = nullptr;

  Handler::Shutdown();
}

void MaterialHandler::SetDestroyCallback(MaterialDestroyCallback callback,
                                         void* user_data) {
  destroy_callback_ = callback;
  destroy_callback_user_data_ = user_data;
}

Material* MaterialHandler::Generate(const MaterialDescr& descr) {
  COMET_PROFILE("MaterialHandler::Generate");

  if (auto* material{TryGet(descr.id)}; material != nullptr) {
    ++material->ref_count;
    return material;
  }

  auto* material{allocator_.AllocateOneAndPopulate<Material>()};
  material->id = descr.id;
  material->shader_id = descr.shader_id;
  material->diffuse_color = descr.diffuse_color;
  material->shininess = descr.shininess;
  material->diffuse_map = descr.diffuse_map;
  material->specular_map = descr.specular_map;
  material->normal_map = descr.normal_map;
  material->ref_count = 1;

  return materials_.Emplace(material->id, material).value;
}

Material* MaterialHandler::Generate(
    const resource::MaterialResource* resource) {
  COMET_ASSERT(resource != nullptr, "Material resource is null!");

  if (auto* material{TryGet(resource->id)}; material != nullptr) {
    ++material->ref_count;
    return material;
  }

  MaterialDescr descr{};
  constexpr auto kLifeSpan{resource::ResourceLifeSpan::Manual};

  descr.id = resource->id;
  descr.shader_id = resource->descr.shader_id;
  descr.diffuse_color = resource->descr.diffuse_color;
  descr.shininess = resource->descr.shininess;
  descr.diffuse_map =
      GenerateTextureMap(&resource->descr.diffuse_map, kLifeSpan);
  descr.specular_map =
      GenerateTextureMap(&resource->descr.specular_map, kLifeSpan);
  descr.normal_map = GenerateTextureMap(&resource->descr.normal_map, kLifeSpan);

  return Generate(descr);
}

Material* MaterialHandler::TryGet(MaterialId material_id) {
  auto material_ptr{materials_.TryGet(material_id)};
  return material_ptr == nullptr ? nullptr : *material_ptr;
}

Material* MaterialHandler::Get(MaterialId material_id) {
  auto* material{TryGet(material_id)};
  COMET_ASSERT(material != nullptr,
               "Requested material does not exist: ", material_id, "!");
  return material;
}

void MaterialHandler::Destroy(MaterialId material_id) {
  Destroy(Get(material_id));
}

void MaterialHandler::Destroy(Material* material) { Destroy(material, false); }

TextureMap MaterialHandler::GenerateTextureMap(
    const resource::TextureMap* map, resource::ResourceLifeSpan life_span) {
  COMET_ASSERT(map != nullptr, "Texture map is null!");

  auto resource_id{map->texture_id != resource::kInvalidResourceId
                       ? map->texture_id
                       : resource::GetDefaultTextureFromType(map->type)};

  auto* resource{resource::ResourceManager::Get().GetTextures()->Load(
      resource_id, life_span)};
  COMET_ASSERT(resource != nullptr, "Texture resource is null!");

  return TextureMap{GetOrGenerateSampler(map),
                    texture_handler_->GetOrGenerate(resource, map->type),
                    resource_id, map->type};
}

void MaterialHandler::Destroy(Material* material, bool is_destroying_handler) {
  COMET_PROFILE("MaterialHandler::Destroy");
  COMET_ASSERT(material != nullptr, "Material is null!");

  if (!is_destroying_handler) {
    COMET_ASSERT(material->ref_count > 0,
                 "Material has a reference count of 0!");

    if (--material->ref_count > 0) {
      return;
    }
  }

  if (destroy_callback_ != nullptr) {
    destroy_callback_(material, destroy_callback_user_data_);
  }

  StaticArray<TextureMap*, 3> texture_maps{
      &material->diffuse_map, &material->specular_map, &material->normal_map};

  auto* texture_resource_handler{
      resource::ResourceManager::Get().GetTextures()};

  for (auto* texture_map : texture_maps) {
    texture_resource_handler->Unload(texture_map->texture_resource_id);
    Destroy(texture_map->sampler);
    *texture_map = {};
  }

  if (!is_destroying_handler) {
    materials_.Remove(material->id);
  }

  allocator_.Deallocate(material);
}

Sampler* MaterialHandler::GenerateSampler(SamplerId sampler_id, GLenum wrap_s,
                                          GLenum wrap_t, GLenum min_filter,
                                          GLenum mag_filter) {
  auto* sampler{allocator_.AllocateOneAndPopulate<Sampler>()};
  sampler->id = sampler_id;
  sampler->ref_count = 1;

  glGenSamplers(1, &sampler->handle);
  COMET_ASSERT(sampler->handle != 0, "Failed to create OpenGL sampler!");

  glSamplerParameteri(sampler->handle, GL_TEXTURE_WRAP_S,
                      static_cast<GLint>(wrap_s));
  glSamplerParameteri(sampler->handle, GL_TEXTURE_WRAP_T,
                      static_cast<GLint>(wrap_t));
  glSamplerParameteri(sampler->handle, GL_TEXTURE_MIN_FILTER,
                      static_cast<GLint>(min_filter));
  glSamplerParameteri(sampler->handle, GL_TEXTURE_MAG_FILTER,
                      static_cast<GLint>(mag_filter));

  return samplers_.Emplace(sampler->id, sampler).value;
}

Sampler* MaterialHandler::GetSampler(SamplerId sampler_id) {
  auto* sampler{TryGetSampler(sampler_id)};
  COMET_ASSERT(sampler != nullptr,
               "Requested sampler does not exist: ", sampler_id, "!");
  return sampler;
}

Sampler* MaterialHandler::TryGetSampler(SamplerId sampler_id) {
  auto* sampler_ptr{samplers_.TryGet(sampler_id)};

  if (sampler_ptr == nullptr) {
    return nullptr;
  }

  return *sampler_ptr;
}

Sampler* MaterialHandler::GetOrGenerateSampler(
    const resource::TextureMap* texture_map) {
  COMET_ASSERT(texture_map != nullptr, "Texture map is null!");

  auto sampler_id{internal::GenerateSamplerId(*texture_map)};
  auto* sampler{TryGetSampler(sampler_id)};

  if (sampler != nullptr) {
    ++sampler->ref_count;
    return sampler;
  }

  auto wrap_s{GetWrapMode(texture_map->u_repeat_mode)};
  auto wrap_t{GetWrapMode(texture_map->v_repeat_mode)};
  auto min_filter{GetFilterMode(texture_map->min_filter_mode)};
  auto mag_filter{GetFilterMode(texture_map->mag_filter_mode)};

  return GenerateSampler(sampler_id, wrap_s, wrap_t, min_filter, mag_filter);
}

void MaterialHandler::Destroy(Sampler* sampler) {
  if (sampler == nullptr) {
    return;
  }

  COMET_ASSERT(sampler->ref_count > 0, "Sampler ref count is 0!");

  if (sampler->ref_count > 1) {
    --sampler->ref_count;
    return;
  }

  if (sampler->handle != 0) {
    glDeleteSamplers(1, &sampler->handle);
    sampler->handle = 0;
  }

  samplers_.Remove(sampler->id);
  allocator_.Deallocate(sampler);
}

GLenum MaterialHandler::GetWrapMode(TextureRepeatMode repeat_mode) {
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
      return GL_REPEAT;
  }

  COMET_ASSERT(false, "Unknown or unsupported repeat mode: ",
               GetTextureRepeatModeLabel(repeat_mode), "!");
  return GL_REPEAT;
}

GLenum MaterialHandler::GetFilterMode(TextureFilterMode filter_mode) {
  switch (filter_mode) {
    case TextureFilterMode::Linear:
      return GL_LINEAR;

    case TextureFilterMode::Nearest:
      return GL_NEAREST;

    case TextureFilterMode::Unknown:
      return GL_LINEAR;
  }

  COMET_ASSERT(false, "Unknown or unsupported filter mode: ",
               GetTextureFilterModeLabel(filter_mode), "!");
  return GL_LINEAR;
}
}  // namespace gl
}  // namespace rendering
}  // namespace comet