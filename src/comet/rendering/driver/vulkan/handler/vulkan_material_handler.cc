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
#include "comet/resource/shader_resource.h"
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
  auto& device{context_->GetDevice()};

  for (auto& it : samplers_) {
    auto& sampler{it.value};

    if (sampler->handle != VK_NULL_HANDLE) {
      vkDestroySampler(device, sampler->handle, VK_NULL_HANDLE);
    }
  }

  samplers_.Destroy();

  for (auto& it : materials_) {
    Destroy(it.value, true);
  }

  materials_.Destroy();
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
  material->ref_count = 1;
  return materials_.Emplace(material->id, material).value;
}

Material* MaterialHandler::Generate(
    const resource::MaterialResource* resource) {
  MaterialDescr descr{};
  constexpr auto kLifeSpan{resource::ResourceLifeSpan::Manual};
  descr.id = resource->id;
  descr.diffuse_map =
      GenerateTextureMap(&resource->descr.diffuse_map, kLifeSpan);
  descr.specular_map =
      GenerateTextureMap(&resource->descr.specular_map, kLifeSpan);
  descr.normal_map = GenerateTextureMap(&resource->descr.normal_map, kLifeSpan);

  TString shader_path{};
  shader_path.Reserve(resource::kMaxShaderNameLen);
  COMET_DISALLOW_STR_ALLOC(shader_path);
  shader_path += COMET_TCHAR("shaders/vulkan/");
  shader_path += GetTmpTChar(resource->descr.shader_name);
  shader_path += COMET_TCHAR(".vk.cshader");
  descr.shader_id =
      resource::GenerateResourceIdFromPath<resource::ShaderResource>(
          shader_path);
  return Generate(descr);
}

Material* MaterialHandler::Get(MaterialId material_id) {
  auto* material{TryGet(material_id)};
  COMET_ASSERT(material != nullptr,
               "Requested material does not exist: ", material_id, "!");
  return material;
}

Material* MaterialHandler::TryGet(MaterialId material_id) {
  auto material_ptr{materials_.TryGet(material_id)};

  if (material_ptr == nullptr) {
    return nullptr;
  }

  auto* material{*material_ptr};
  ++material->ref_count;
  return material;
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
    const resource::TextureMap* map, resource::ResourceLifeSpan life_span) {
  const auto resource_id{map->texture_id != resource::kInvalidResourceId
                             ? map->texture_id
                             : resource::GetDefaultTextureFromType(map->type)};

  return TextureMap{GetOrGenerateSampler(map),
                    texture_handler_->GetOrGenerate(
                        resource::ResourceManager::Get().GetTextures()->Load(
                            resource_id, life_span)),
                    resource_id, map->type};
}

void MaterialHandler::Destroy(Material* material, bool is_destroying_handler) {
  COMET_PROFILE("MaterialHandler::Destroy");

  if (!is_destroying_handler) {
    COMET_ASSERT(material->ref_count > 0,
                 "Material has a reference count of 0!");

    if (--material->ref_count > 0) {
      return;
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

    materials_.Remove(material->id);
    allocator_.Deallocate(material);
  }
}

Sampler* MaterialHandler::GenerateSampler(SamplerId sampler_id,
                                          const VkSamplerCreateInfo& info) {
  auto* sampler{allocator_.AllocateOneAndPopulate<Sampler>()};
  sampler->id = sampler_id;
  sampler->ref_count = 0;

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
  const auto sampler_info{init::GenerateSamplerCreateInfo(
      VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT, texture_map,
      context_->IsSamplerAnisotropy(),
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