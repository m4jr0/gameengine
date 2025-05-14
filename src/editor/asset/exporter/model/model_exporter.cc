// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "comet_pch.h"

#include "model_exporter.h"

#include "assimp/postprocess.h"
#include "comet/core/c_string.h"
#include "comet/core/concurrency/job/job_utils.h"
#include "comet/core/concurrency/job/scheduler.h"
#include "comet/core/file_system/file_system.h"
#include "comet/core/logger.h"
#include "comet/core/memory/memory_utils.h"
#include "comet/core/type/array.h"
#include "comet/rendering/rendering_common.h"
#include "comet/resource/resource_manager.h"
#include "comet/resource/texture_resource.h"
#include "editor/asset/asset_utils.h"
#include "editor/asset/exporter/model/utils/model_exporter_utils.h"

#ifdef COMET_FIBER_DEBUG_LABEL
#include "comet/core/concurrency/fiber/fiber.h"
#endif  // COMET_FIBER_DEBUG_LABEL

namespace comet {
namespace editor {
namespace asset {
bool ModelExporter::IsCompatible(CTStringView extension) const {
  return extension == COMET_TCHAR("obj") || extension == COMET_TCHAR("fbx") ||
         extension == COMET_TCHAR("dae") || extension == COMET_TCHAR("gltf");
}

void ModelExporter::PopulateFiles(ResourceFilesContext& context) const {
  auto& asset_descr{context.asset_descr};

  SceneContext scene_context{};
  scene_context.resource_files = &context.files;
  scene_context.exporter = this;
  scene_context.asset_abs_path = asset_descr.asset_abs_path.GetCTStr();
  scene_context.asset_path = asset_descr.asset_path.GetCTStr();
  scene_context.allocator = context.allocator;

  auto& scheduler{job::Scheduler::Get()};

  auto io_job_descr{scene_context.GenerateSceneLoadingJobDescr()};
  scheduler.KickAndWait(io_job_descr);
  scheduler.DestroyCounter(io_job_descr.counter);

  if (scene_context.scene == nullptr) {
    return;
  }

  auto* counter{scheduler.GenerateCounter()};

  // N material resources, and 1 model.
  scene_context.resource_files->Reserve(
      static_cast<usize>(scene_context.scene->mNumMaterials + 1));

  scheduler.Kick(scene_context.GenerateModelProcessingJobDescr(counter));
  scheduler.Kick(scene_context.GenerateMaterialsProcessingJobDescr(counter));
  scheduler.Wait(counter);
  scheduler.DestroyCounter(counter);
}

void ModelExporter::LoadMaterialTextures(CTStringView resource_path,
                                         resource::MaterialResource& material,
                                         aiMaterial* raw_material,
                                         aiTextureType raw_texture_type) const {
  const auto texture_count{raw_material->GetTextureCount(raw_texture_type)};

  if (texture_count == 0) {
    return;
  }

  auto texture_type{GetTextureType(raw_texture_type)};

  if (texture_count > 1) {
    COMET_LOG_GLOBAL_WARNING("Texture count for type \"",
                             rendering::GetTextureTypeLabel(texture_type),
                             "\" is greater than 1. This is not supported. "
                             "Ignoring excess textures.");
  }

  resource::TextureMap* map{nullptr};

  switch (texture_type) {
    case rendering::TextureType::Diffuse:
      map = &material.descr.diffuse_map;
      break;
    case rendering::TextureType::Specular:
      map = &material.descr.specular_map;
      break;
      // TODO(m4jr0): Check behavior. Apparently, Assimp might process normal
      // textures as height ones. See
      // https://github.com/assimp/assimp/issues/430
    case rendering::TextureType::Normal:
      map = &material.descr.normal_map;
      break;
    default:
      COMET_LOG_GLOBAL_WARNING("Unsupported texture type: ",
                               rendering::GetTextureTypeLabel(texture_type),
                               ". Ignoring.");
      break;
  }

  if (map == nullptr) {
    return;
  }

  aiString raw_texture_path;
  raw_material->GetTexture(raw_texture_type, 0, &raw_texture_path);

  if (!raw_material->GetTexture(raw_texture_type, 0, &raw_texture_path) ==
      AI_SUCCESS) {
    COMET_LOG_GLOBAL_ERROR("Could not load ", GetTextureTypeLabel(texture_type),
                           " texture from material ",
                           raw_material->GetName().C_Str(), " at path ",
                           resource_path);
    return;
  }

  auto path{resource_path / GetTmpTChar(raw_texture_path.C_Str())};
  Clean(path);
  map->texture_id =
      resource::GenerateResourceIdFromPath<resource::TextureResource>(path);

  map->type = texture_type;
  aiTextureMapMode raw_texture_repeat_mode;

  if (raw_material->Get(AI_MATKEY_MAPPINGMODE_U(raw_texture_type, 0),
                        raw_texture_repeat_mode) != AI_SUCCESS) {
    COMET_LOG_GLOBAL_DEBUG(
        "Could not get texture repeat mode for U. Setting it to repeat mode.");
    raw_texture_repeat_mode = aiTextureMapMode_Wrap;
  }

  map->u_repeat_mode = GetTextureRepeatMode(raw_texture_repeat_mode);

  if (raw_material->Get(AI_MATKEY_MAPPINGMODE_V(raw_texture_type, 0),
                        raw_texture_repeat_mode) != AI_SUCCESS) {
    COMET_LOG_GLOBAL_DEBUG(
        "Could not get texture repeat mode for V. Setting it to repeat "
        "mode.");
    raw_texture_repeat_mode = aiTextureMapMode_Wrap;
  }

  map->v_repeat_mode = GetTextureRepeatMode(raw_texture_repeat_mode);

  // TODO(m4jr0): Handle filter modes better.
  map->min_filter_mode = rendering::TextureFilterMode::Linear;
  map->mag_filter_mode = rendering::TextureFilterMode::Linear;
}

void ModelExporter::LoadDefaultTextures(
    resource::MaterialResource& material) const {
  if (material.descr.diffuse_map.type == rendering::TextureType::Unknown) {
    auto& map{material.descr.diffuse_map};
    map.texture_id = resource::kInvalidResourceId;
    map.type = rendering::TextureType::Diffuse;
    map.u_repeat_mode = rendering::TextureRepeatMode::Repeat;
    map.v_repeat_mode = rendering::TextureRepeatMode::Repeat;
    map.min_filter_mode = rendering::TextureFilterMode::Linear;
    map.mag_filter_mode = rendering::TextureFilterMode::Linear;
  }

  if (material.descr.specular_map.type == rendering::TextureType::Unknown) {
    auto& map{material.descr.specular_map};
    map.texture_id = resource::kInvalidResourceId;
    map.type = rendering::TextureType::Specular;
    map.u_repeat_mode = rendering::TextureRepeatMode::Repeat;
    map.v_repeat_mode = rendering::TextureRepeatMode::Repeat;
    map.min_filter_mode = rendering::TextureFilterMode::Linear;
    map.mag_filter_mode = rendering::TextureFilterMode::Linear;
  }

  if (material.descr.normal_map.type == rendering::TextureType::Unknown) {
    auto& map{material.descr.normal_map};
    map.texture_id = resource::kInvalidResourceId;
    map.type = rendering::TextureType::Normal;
    map.u_repeat_mode = rendering::TextureRepeatMode::Repeat;
    map.v_repeat_mode = rendering::TextureRepeatMode::Repeat;
    map.min_filter_mode = rendering::TextureFilterMode::Linear;
    map.mag_filter_mode = rendering::TextureFilterMode::Linear;
  }
}

void ModelExporter::OnSceneLoading(job::IOJobParamsHandle params_handle) {
  auto* scene_context{reinterpret_cast<SceneContext*>(params_handle)};
#ifdef COMET_WIDE_TCHAR
  auto length{GetLength(scene_context->asset_abs_path)};
  auto* scene_path{scene_context->allocator->AllocateMany<schar>(length + 1)};
  Copy(scene_path, scene_context->asset_abs_path, length);
  scene_path[length] = COMET_TCHAR('\0');
#else
  auto* scene_path{scene_context->asset_abs_path};
#endif  // COMET_WIDE_TCHAR

  COMET_LOG_GLOBAL_DEBUG("Loading scene at ", scene_path, "...");
  const auto* scene{scene_context->assimp_importer.ReadFile(
      scene_path,
      aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace)};

  if (scene == nullptr || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE ||
      scene->mRootNode == nullptr) {
    const auto* assimp_error{scene_context->assimp_importer.GetErrorString()};

    if (IsEmpty(assimp_error)) {
      if (scene == nullptr) {
        assimp_error = "Scene is null";
      } else if (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) {
        assimp_error = "Scene is incomplete";
      } else if (scene->mRootNode) {
        assimp_error = "Scene has no root node";
      } else {
        assimp_error = "Unknown error";
      }
    }

    COMET_LOG_RESOURCE_ERROR("Assimp error at path: ",
                             scene_context->asset_abs_path, ": ", assimp_error);
  }

  scene_context->scene = scene;
  COMET_LOG_GLOBAL_DEBUG("Scene loaded at ", scene_path);

#ifdef COMET_WIDE_TCHAR
  scene_context->allocator->Deallocate(scene_path);
  scene_path = nullptr;
#endif  // COMET_WIDE_TCHAR
}

void ModelExporter::OnModelProcessing(job::JobParamsHandle params_handle) {
  auto* scene_context{reinterpret_cast<SceneContext*>(params_handle)};
  auto* exporter{scene_context->exporter};
  auto* scene{scene_context->scene};
  COMET_LOG_GLOBAL_DEBUG("Processing model at: ", scene_context->asset_abs_path,
                         "...");

  if (scene->HasAnimations()) {
    auto resources{LoadSkeletalModel(scene_context->allocator, scene,
                                     scene_context->asset_path)};

    scene_context->AddResourceFile(
        resource::ResourceManager::Get().GetResourceFile(
            resources.skeleton, exporter->compression_mode_));

    scene_context->AddResourceFile(
        resource::ResourceManager::Get().GetResourceFile(
            resources.model, exporter->compression_mode_));

    for (const auto& clip : resources.animation_clips) {
      scene_context->AddResourceFile(
          resource::ResourceManager::Get().GetResourceFile(
              clip, exporter->compression_mode_));
    }

  } else {
    auto resources{LoadStaticModel(scene_context->allocator, scene,
                                   scene_context->asset_path)};

    scene_context->AddResourceFile(
        resource::ResourceManager::Get().GetResourceFile(
            LoadStaticModel(scene_context->allocator, scene,
                            scene_context->asset_path)
                .model,
            exporter->compression_mode_));
  }

  COMET_LOG_GLOBAL_DEBUG("Model processed at: ", scene_context->asset_abs_path);
}

void ModelExporter::OnMaterialsProcessing(job::JobParamsHandle params_handle) {
  auto* scene_context{reinterpret_cast<SceneContext*>(params_handle)};
  auto* exporter{scene_context->exporter};
  COMET_LOG_GLOBAL_DEBUG(
      "Processing model materials at: ", scene_context->asset_abs_path, "...");
  exporter->LoadMaterials(scene_context);
  COMET_LOG_GLOBAL_DEBUG("Model materials processed at: ",
                         scene_context->asset_abs_path);
}

void ModelExporter::LoadMaterials(SceneContext* scene_context) const {
  auto* scene{scene_context->scene};
  auto directory_path{GetDirectoryPath(scene_context->asset_abs_path)};
  const auto resource_path{GetRelativePath(directory_path, root_asset_path_)};
  constexpr StaticArray<aiTextureType, 4> exported_texture_types{
      aiTextureType_DIFFUSE, aiTextureType_SPECULAR, aiTextureType_HEIGHT,
      aiTextureType_AMBIENT};

  for (usize i{0}; i < scene->mNumMaterials; ++i) {
    const auto raw_material{scene->mMaterials[i]};
    const auto& raw_material_name{raw_material->GetName()};
    const auto* material_name{raw_material_name.C_Str()};

    if (IsEmpty(material_name)) {
      COMET_LOG_GLOBAL_ERROR("Empty material found. Ignoring.");
      continue;
    }

    COMET_LOG_GLOBAL_DEBUG("Material \"", material_name, "\" found.");

    resource::MaterialResource material{};
    constexpr auto kDefaultShaderName{"default_shader"};
    memory::CopyMemory(material.descr.shader_name, kDefaultShaderName,
                       GetLength(kDefaultShaderName));

    if (aiGetMaterialFloat(raw_material, AI_MATKEY_SHININESS,
                           &material.descr.shininess) != AI_SUCCESS) {
      COMET_LOG_GLOBAL_WARNING(
          "Could not retrieve shininess property from material. Setting it to ",
          kDefaultMaterialShininess_);
      material.descr.shininess = kDefaultMaterialShininess_;
    }

    aiColor3D color;

    if (raw_material->Get(AI_MATKEY_COLOR_DIFFUSE, color) != AI_SUCCESS) {
      COMET_LOG_GLOBAL_WARNING(
          "Could not retrieve diffuse color property from material. Setting it "
          "to (",
          kDefaultColor_.r, ", ", kDefaultColor_.g, ", ", kDefaultColor_.b,
          ").");

      color = kDefaultColor_;
    }

    material.descr.diffuse_color = math::Vec4(color.r, color.g, color.b, 1.0f);

    for (auto raw_texture_type : exported_texture_types) {
      LoadMaterialTextures(resource_path, material, raw_material,
                           raw_texture_type);
    }

    LoadDefaultTextures(material);
    material.id = resource::GenerateMaterialId(material_name);
    material.type_id = resource::MaterialResource::kResourceTypeId;

    scene_context->AddResourceFile(
        resource::ResourceManager::Get().GetResourceFile(material,
                                                         compression_mode_));
  }
}

rendering::TextureType ModelExporter::GetTextureType(
    aiTextureType raw_texture_type) {
  switch (raw_texture_type) {
    case aiTextureType_DIFFUSE:
      return rendering::TextureType::Diffuse;

    case aiTextureType_SPECULAR:
      return rendering::TextureType::Specular;

    case aiTextureType_HEIGHT:
      return rendering::TextureType::Normal;

    case aiTextureType_AMBIENT:
      return rendering::TextureType::Ambient;

    case aiTextureType_BASE_COLOR:
      return rendering::TextureType::Color;

    default:
      return rendering::TextureType::Unknown;
  }
}

rendering::TextureRepeatMode ModelExporter::GetTextureRepeatMode(
    aiTextureMapMode raw_texture_repeat_mode) {
  switch (raw_texture_repeat_mode) {
    case aiTextureMapMode_Wrap:
      return rendering::TextureRepeatMode::Repeat;
    case aiTextureMapMode_Mirror:
      return rendering::TextureRepeatMode::MirroredRepeat;
    case aiTextureMapMode_Clamp:
      return rendering::TextureRepeatMode::ClampToEdge;
    case aiTextureMapMode_Decal:
      return rendering::TextureRepeatMode::ClampToBorder;
    default:
      return rendering::TextureRepeatMode::Unknown;
  }
}

job::IOJobDescr ModelExporter::SceneContext::GenerateSceneLoadingJobDescr() {
  return job::GenerateIOJobDescr(OnSceneLoading, this,
                                 job::Scheduler::Get().GenerateCounter());
}

job::JobDescr ModelExporter::SceneContext::GenerateModelProcessingJobDescr(
    job::Counter* counter) {
#ifdef COMET_FIBER_DEBUG_LABEL
  schar debug_label[fiber::Fiber::kDebugLabelMaxLen_ + 1]{'\0'};
#endif  // COMET_FIBER_DEBUG_LABEL

  return job::GenerateJobDescr(
      job::JobPriority::Normal, ModelExporter::OnModelProcessing, this,
      job::JobStackSize::Normal, counter,
      COMET_ASSET_HANDLE_FIBER_DEBUG_LABEL(asset_path, debug_label,
                                           fiber::Fiber::kDebugLabelMaxLen_));
}

job::JobDescr ModelExporter::SceneContext::GenerateMaterialsProcessingJobDescr(
    job::Counter* counter) {
#ifdef COMET_FIBER_DEBUG_LABEL
  schar buffer[fiber::Fiber::kDebugLabelMaxLen_ + 1];
#endif  // COMET_FIBER_DEBUG_LABEL

  return job::GenerateJobDescr(
      job::JobPriority::Normal, ModelExporter::OnMaterialsProcessing, this,
      job::JobStackSize::Normal, counter,
      COMET_ASSET_HANDLE_FIBER_DEBUG_LABEL(asset_path, buffer,
                                           fiber::Fiber::kDebugLabelMaxLen_));
}

void ModelExporter::SceneContext::AddResourceFile(
    const resource::ResourceFile& file) {
  fiber::FiberLockGuard lock{resource_mutex};
  resource_files->PushBack(file);
}
}  // namespace asset
}  // namespace editor
}  // namespace comet
