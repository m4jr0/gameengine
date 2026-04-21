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

#include "comet/core/memory/allocator/allocator.h"
#include "comet/core/type_trait.h"
#include "comet/profiler/profiler.h"
#include "comet/rendering/driver/opengl/type/opengl_sampler_type.h"
#include "comet/rendering/driver/opengl/utils/opengl_texture_map_utils.h"
#include "comet/rendering/driver/opengl/utils/opengl_texture_utils.h"
#include "comet/rendering/label/rendering_texture_label.h"
#include "comet/resource/material/material_resource.h"
#include "comet/resource/resource_manager.h"
#include "comet/resource/texture/texture_resource.h"

namespace comet {
namespace rendering {
namespace gl {
MaterialHandler::MaterialHandler(const MaterialHandlerDescr& descr)
    : Handler{descr},
      materials_{&cache_allocator_, 256},
      texture_handler_{descr.texture_handler},
      sampler_handler_{descr.sampler_handler} {
  COMET_ASSERT(texture_handler_ != nullptr, "MaterialHandler::MaterialHandler",
               "texture handler is null");
  COMET_ASSERT(sampler_handler_ != nullptr, "MaterialHandler::MaterialHandler",
               "sampler handler is null");
}

void MaterialHandler::SetDestroyCallback(MaterialDestroyCallback callback,
                                         void* user_data) {
  destroy_callback_ = callback;
  destroy_callback_user_data_ = user_data;
}

MaterialHandle MaterialHandler::GetOrGenerate(const MaterialDescr& descr) {
  if (const auto handle{materials_.TryAcquire(descr.id)}; handle) {
    return handle;
  }

  auto* material{allocator_.AllocateOneAndPopulate<Material>()};
  material->handle = MaterialHandle::Invalid();
  material->id = descr.id;
  material->shader_resource_id = descr.shader_resource_id;
  material->diffuse_color = descr.diffuse_color;
  material->shininess = descr.shininess;
  material->diffuse_map = descr.diffuse_map;
  material->specular_map = descr.specular_map;
  material->normal_map = descr.normal_map;

  const auto handle{materials_.Create(material->id, material)};
  COMET_ASSERT(handle, "MaterialHandler::GetOrGenerate",
               "failed to create material instance", "material_id", descr.id);

  material->handle = handle;
  return handle;
}

MaterialHandle MaterialHandler::GetOrGenerate(
    resource::MaterialResourceId material_resource_id) {
  COMET_ASSERT(material_resource_id.IsValid(), "MaterialHandler::GetOrGenerate",
               "material resource id is invalid");

  if (const auto handle{materials_.TryAcquire(material_resource_id)}; handle) {
    return handle;
  }

  MaterialHandle generated_handle{};
  auto* material_resource_handler{
      resource::ResourceManager::Get().GetMaterials()};

  const auto is_loaded{material_resource_handler->WithTemporaryLoad(
      material_resource_id,
      [this,
       &generated_handle](const resource::MaterialResource* material_resource) {
        MaterialDescr descr{};
        descr.id = material_resource->GetId();
        descr.shader_resource_id = material_resource->descr.shader_resource_id;
        descr.diffuse_color = material_resource->descr.diffuse_color;
        descr.shininess = material_resource->descr.shininess;
        descr.diffuse_map =
            GenerateTextureMap(&material_resource->descr.diffuse_map);
        descr.specular_map =
            GenerateTextureMap(&material_resource->descr.specular_map);
        descr.normal_map =
            GenerateTextureMap(&material_resource->descr.normal_map);

        generated_handle = GetOrGenerate(descr);
      })};

  return is_loaded ? generated_handle : MaterialHandle{};
}

void MaterialHandler::Destroy(MaterialHandle handle) {
  auto* material{materials_.Get(handle)};

  if (!materials_.Release(handle)) {
    return;
  }

  COMET_ASSERT(material->handle == handle, "MaterialHandler::Destroy",
               "material handle mismatch", "handle", handle, "material_handle",
               material->handle);
  DestroyMaterial(material);
  materials_.Remove(handle);
}

const Material* MaterialHandler::Get(MaterialHandle handle) const {
  const auto* material{materials_.TryGet(handle)};
  COMET_ASSERT(material != nullptr, "MaterialHandler::Get",
               "material does not exist", "handle", handle);
  return material;
}

void MaterialHandler::OnInitialize() {
  allocator_.Initialize();
  materials_.Initialize();
}

void MaterialHandler::OnShutdown() {
  destroy_callback_ = nullptr;
  destroy_callback_user_data_ = nullptr;

  memory::PlatformAllocator tmp_allocator{memory::kEngineMemoryTagRendering};

  Array<MaterialHandle> material_handles_to_destroy{&tmp_allocator};

  materials_.ForEachLive(
      [&material_handles_to_destroy](MaterialHandle handle, const Material*) {
        material_handles_to_destroy.PushBack(handle);
      });

  for (const auto handle : material_handles_to_destroy) {
    const auto ref_count{materials_.GetRefCount(handle)};

    if (ref_count > 1) {
      COMET_LOG_WARNING(LoggerType::Rendering, "MaterialHandler::OnShutdown",
                        "forcing material destruction", "handle", handle,
                        "ref_count", ref_count);
    }

    auto* material{materials_.Drain(handle)};

    if (material == nullptr) {
      continue;
    }

    COMET_ASSERT(material->handle == handle, "MaterialHandler::OnShutdown",
                 "material handle mismatch", "handle", handle,
                 "material_handle", material->handle);

    DestroyMaterial(material);
  }

  materials_.Destroy();
  allocator_.Destroy();
}

TextureMap MaterialHandler::GenerateTextureMap(
    const resource::TextureMapResource* map) {
  COMET_ASSERT(map != nullptr, "MaterialHandler::GenerateTextureMap",
               "texture map is null");

  const auto texture_resource_id{
      map->texture_resource_id.IsValid()
          ? map->texture_resource_id
          : resource::GetDefaultTextureFromType(map->type)};

  const auto sampler_handle{GetOrGenerateSampler(map)};
  const auto texture_handle{
      texture_handler_->GetOrGenerate(texture_resource_id, map->type)};

  COMET_ASSERT(texture_handle, "MaterialHandler::GenerateTextureMap",
               "texture could not be loaded", "texture_resource_id",
               texture_resource_id, "texture_type",
               GetTextureTypeLabel(map->type), "texture_type_value",
               ToUnderlying(map->type));

  return BuildTextureMap(sampler_handle, texture_handle, texture_resource_id,
                         map->type);
}

void MaterialHandler::DestroyMaterial(Material* material) {
  COMET_PROFILE("MaterialHandler::DestroyMaterial");
  COMET_ASSERT(material != nullptr, "MaterialHandler::DestroyMaterial",
               "material is null");

  if (destroy_callback_ != nullptr) {
    destroy_callback_(material, destroy_callback_user_data_);
  }

  StaticArray<TextureMap*, 3> texture_maps{
      &material->diffuse_map, &material->specular_map, &material->normal_map};

  for (auto* texture_map : texture_maps) {
    if (texture_map->sampler_handle) {
      sampler_handler_->Destroy(texture_map->sampler_handle);
    }

    if (texture_map->texture_handle) {
      texture_handler_->Destroy(texture_map->texture_handle);
    }

    *texture_map = {};
  }

  material->handle.Invalidate();
  allocator_.Deallocate(material);
}

SamplerHandle MaterialHandler::GetOrGenerateSampler(
    const resource::TextureMapResource* map) {
  COMET_ASSERT(map != nullptr, "MaterialHandler::GetOrGenerateSampler",
               "texture map is null");

  SamplerDescr descr{};
  descr.wrap_s = GetGlWrapMode(map->u_repeat_mode);
  descr.wrap_t = GetGlWrapMode(map->v_repeat_mode);
  descr.wrap_r = GetGlWrapMode(map->w_repeat_mode);
  descr.min_filter = GetGlFilterMode(map->min_filter_mode);
  descr.mag_filter = GetGlFilterMode(map->mag_filter_mode);

  return sampler_handler_->GetOrGenerate(descr);
}

Material* MaterialHandler::Get(MaterialHandle handle) {
  auto* material{materials_.TryGet(handle)};
  COMET_ASSERT(material != nullptr, "MaterialHandler::Get",
               "material does not exist", "handle", handle);
  return material;
}
}  // namespace gl
}  // namespace rendering
}  // namespace comet