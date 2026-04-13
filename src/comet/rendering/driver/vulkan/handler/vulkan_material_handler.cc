// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "vulkan_material_handler.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/file_system/file_system.h"
#include "comet/core/memory/allocator/allocator.h"
#include "comet/core/type/array.h"
#include "comet/profiler/profiler.h"
#include "comet/rendering/driver/vulkan/data/vulkan_material.h"
#include "comet/rendering/driver/vulkan/utils/vulkan_initializer_utils.h"
#include "comet/rendering/driver/vulkan/utils/vulkan_material_utils.h"
#include "comet/rendering/driver/vulkan/vulkan_context.h"
#include "comet/rendering/driver/vulkan/vulkan_debug.h"
#include "comet/resource/material_resource.h"
#include "comet/resource/resource.h"
#include "comet/resource/resource_manager.h"
#include "comet/resource/texture_resource.h"

namespace comet {
namespace rendering {
namespace vk {
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
  return Destroy(Get(material_id));
}

void MaterialHandler::Destroy(Material* material) {
  return Destroy(material, false);
}

TextureMap MaterialHandler::GenerateTextureMap(
    const resource::TextureMap* map, resource::ResourceLifeSpan life_span) {
  COMET_ASSERT(map != nullptr, "Texture map is null!");

  auto resource_id{map->texture_id != resource::kInvalidResourceId
                       ? map->texture_id
                       : resource::GetDefaultTextureFromType(map->type)};

  const auto* resource{resource::ResourceManager::Get().GetTextures()->Load(
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

  StaticArray<TextureMap*, 3> texture_maps = {
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

Sampler* MaterialHandler::GenerateSampler(SamplerId sampler_id,
                                          const VkSamplerCreateInfo& info) {
  auto* sampler{allocator_.AllocateOneAndPopulate<Sampler>()};
  sampler->id = sampler_id;
  sampler->ref_count = 1;

  COMET_CHECK_VK(vkCreateSampler(context_->GetDevice(), &info, VK_NULL_HANDLE,
                                 &sampler->handle),
                 "Failed to create texture sampler!");

  return samplers_.Emplace(sampler->id, sampler).value;
}

Sampler* MaterialHandler::GetSampler(SamplerId sampler_id) {
  auto* sampler{TryGetSampler(sampler_id)};
  COMET_ASSERT(sampler != nullptr,
               "Requested sampler does not exist: ", sampler_id, "!");
  return sampler;
}

Sampler* MaterialHandler::TryGetSampler(SamplerId sampler_id) {
  auto** sampler{samplers_.TryGet(sampler_id)};

  if (sampler == nullptr) {
    return nullptr;
  }

  return *sampler;
}

Sampler* MaterialHandler::GetOrGenerateSampler(
    const resource::TextureMap* texture_map) {
  auto sampler_info{init::GenerateSamplerCreateInfo(
      *texture_map, context_->IsSamplerAnisotropy(),
      context_->GetDevice().GetProperties().limits.maxSamplerAnisotropy)};

  auto sampler_id{std::hash<VkSamplerCreateInfo>()(sampler_info)};
  auto* sampler{TryGetSampler(sampler_id)};

  if (sampler != nullptr) {
    ++sampler->ref_count;
    return sampler;
  }

  return GenerateSampler(sampler_id, sampler_info);
}

void MaterialHandler::Destroy(Sampler* sampler) {
  if (sampler == nullptr) {
    return;
  }

  if (sampler->ref_count > 1) {
    --sampler->ref_count;
    return;
  }

  if (sampler->handle != VK_NULL_HANDLE) {
    vkDestroySampler(context_->GetDevice(), sampler->handle, VK_NULL_HANDLE);
  }

  samplers_.Remove(sampler->id);
  allocator_.Deallocate(sampler);
}
}  // namespace vk
}  // namespace rendering
}  // namespace comet