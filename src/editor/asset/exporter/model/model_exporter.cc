// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "model_exporter.h"
////////////////////////////////////////////////////////////////////////////////

// External. ///////////////////////////////////////////////////////////////////
#include "assimp/postprocess.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/c_string.h"
#include "comet/core/concurrency/job/job_utils.h"
#include "comet/core/concurrency/job/scheduler.h"
#include "comet/core/file_system/file_system.h"
#include "comet/core/logger/logging.h"
#include "comet/core/type/array.h"
#include "comet/rendering/label/rendering_texture_label.h"
#include "comet/rendering/type/rendering_texture_type.h"
#include "comet/resource/resource_manager.h"
#include "comet/resource/shader/shader_resource.h"
#include "comet/resource/texture/texture_resource.h"
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
         extension == COMET_TCHAR("dae") || extension == COMET_TCHAR("gltf") ||
         extension == COMET_TCHAR("glb");
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

  const auto io_job_descr{scene_context.GenerateSceneLoadingJobDescr()};
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
  COMET_ASSERT(raw_material != nullptr, "ModelExporter::LoadMaterialTextures",
               "raw material is null");
  const auto texture_count{raw_material->GetTextureCount(raw_texture_type)};

  if (texture_count == 0) {
    return;
  }

  const auto texture_type{GetTextureType(raw_texture_type)};

  if (texture_count > 1) {
    COMET_LOG_WARNING(
        LoggerType::External, "ModelExporter::LoadMaterialTextures",
        "texture count is greater than one, ignoring excess textures",
        "texture_type", rendering::GetTextureTypeLabel(texture_type),
        "texture_count", texture_count);
  }

  resource::TextureMapResource* map{nullptr};

  switch (texture_type) {
    case rendering::TextureType::Diffuse:
      map = &material.descr.diffuse_map;
      break;

    case rendering::TextureType::Specular:
      map = &material.descr.specular_map;
      break;

    case rendering::TextureType::Normal:
      map = &material.descr.normal_map;
      break;

    default:
      COMET_LOG_WARNING(LoggerType::External,
                        "ModelExporter::LoadMaterialTextures",
                        "texture type is unsupported", "texture_type",
                        rendering::GetTextureTypeLabel(texture_type));
      return;
  }

  aiString raw_texture_path{};

  if (raw_material->GetTexture(raw_texture_type, 0, &raw_texture_path) !=
      AI_SUCCESS) {
    COMET_LOG_ERROR(LoggerType::External, "ModelExporter::LoadMaterialTextures",
                    "material texture load failed", "texture_type",
                    GetTextureTypeLabel(texture_type), "material",
                    raw_material->GetName().C_Str(), "resource_path",
                    resource_path);
    return;
  }

  auto path{resource_path / GetTmpTChar(raw_texture_path.C_Str())};
  Clean(path);

  COMET_LOG_DEBUG(LoggerType::External, "ModelExporter::LoadMaterialTextures",
                  "material texture resolved", "material",
                  raw_material->GetName().C_Str(), "texture_type",
                  GetTextureTypeLabel(texture_type), "assimp_slot",
                  static_cast<int>(raw_texture_type), "path", path);

  map->texture_resource_id =
      resource::GenerateResourceIdFromPath<resource::TextureResource>(path);
  map->type = texture_type;

  aiTextureMapMode raw_u_repeat_mode;
  aiTextureMapMode raw_v_repeat_mode;

  if (raw_material->Get(AI_MATKEY_MAPPINGMODE_U(raw_texture_type, 0),
                        raw_u_repeat_mode) != AI_SUCCESS) {
    COMET_LOG_DEBUG(LoggerType::External, "ModelExporter::LoadMaterialTextures",
                    "texture u repeat mode is missing, using wrap",
                    "texture_type", GetTextureTypeLabel(texture_type),
                    "material", raw_material->GetName().C_Str());
    raw_u_repeat_mode = aiTextureMapMode_Wrap;
  }

  if (raw_material->Get(AI_MATKEY_MAPPINGMODE_V(raw_texture_type, 0),
                        raw_v_repeat_mode) != AI_SUCCESS) {
    COMET_LOG_DEBUG(LoggerType::External, "ModelExporter::LoadMaterialTextures",
                    "texture v repeat mode is missing, using wrap",
                    "texture_type", GetTextureTypeLabel(texture_type),
                    "material", raw_material->GetName().C_Str());
    raw_v_repeat_mode = aiTextureMapMode_Wrap;
  }

  map->u_repeat_mode = GetTextureRepeatMode(raw_u_repeat_mode);
  map->v_repeat_mode = GetTextureRepeatMode(raw_v_repeat_mode);
  map->w_repeat_mode =
      rendering::TextureRepeatMode::Repeat;  // No AI_MATKEY_MAPPINGMODE_W.

  map->min_filter_mode = rendering::TextureFilterMode::Linear;
  map->mag_filter_mode = rendering::TextureFilterMode::Linear;
}

void ModelExporter::OnSceneLoading(job::IOJobParamsHandle params_handle) {
  auto* scene_context{reinterpret_cast<SceneContext*>(params_handle)};
  COMET_ASSERT(scene_context != nullptr, "ModelExporter::OnSceneLoading",
               "scene context is null");

#ifdef COMET_WIDE_TCHAR
  const auto length{GetLength(scene_context->asset_abs_path)};
  auto* scene_path{scene_context->allocator->AllocateMany<schar>(length + 1)};
  Copy(scene_path, scene_context->asset_abs_path, length);
  scene_path[length] = COMET_TCHAR('\0');
#else
  auto* scene_path{scene_context->asset_abs_path};
#endif  // COMET_WIDE_TCHAR

  COMET_LOG_DEBUG(LoggerType::External, "ModelExporter::OnSceneLoading",
                  "loading scene", "asset_path", scene_path);

  const auto* scene{scene_context->assimp_importer.ReadFile(
      scene_path, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices |
                      aiProcess_ImproveCacheLocality |
                      aiProcess_LimitBoneWeights | aiProcess_FindInvalidData |
                      aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace |
                      aiProcess_FlipUVs)};

  if (scene == nullptr || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE ||
      scene->mRootNode == nullptr) {
    const auto* assimp_error{scene_context->assimp_importer.GetErrorString()};

    if (IsEmpty(assimp_error)) {
      if (scene == nullptr) {
        assimp_error = "Scene is null";
      } else if (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) {
        assimp_error = "Scene is incomplete";
      } else if (scene->mRootNode == nullptr) {
        assimp_error = "Scene has no root node";
      } else {
        assimp_error = "Unknown error";
      }
    }

    COMET_LOG_ERROR(LoggerType::External, "ModelExporter::OnSceneLoading",
                    "scene loading failed", "asset_path",
                    scene_context->asset_abs_path, "error", assimp_error);
  }

  scene_context->scene = scene;

  COMET_LOG_DEBUG(LoggerType::External, "ModelExporter::OnSceneLoading",
                  "scene loaded", "asset_path", scene_path);

#ifdef COMET_WIDE_TCHAR
  scene_context->allocator->Deallocate(scene_path);
  scene_path = nullptr;
#endif  // COMET_WIDE_TCHAR
}

void ModelExporter::OnModelProcessing(job::JobParamsHandle params_handle) {
  auto* scene_context{reinterpret_cast<SceneContext*>(params_handle)};
  COMET_ASSERT(scene_context != nullptr, "ModelExporter::OnModelProcessing",
               "scene context is null");

  auto* exporter{scene_context->exporter};
  COMET_ASSERT(exporter != nullptr, "ModelExporter::OnModelProcessing",
               "exporter is null");

  auto* scene{scene_context->scene};
  COMET_ASSERT(scene != nullptr, "ModelExporter::OnModelProcessing",
               "scene is null");

  COMET_LOG_DEBUG(LoggerType::External, "ModelExporter::OnModelProcessing",
                  "processing model", "asset_path",
                  scene_context->asset_abs_path);

  if (scene->HasAnimations()) {
    const auto resources{LoadSkeletalModel(scene_context->allocator, scene,
                                           scene_context->asset_path)};

    scene_context->AddResourceFile(
        resource::ResourceManager::Get().GetSkeletons()->Pack(
            resources.skeleton, exporter->compression_mode_));

    scene_context->AddResourceFile(
        resource::ResourceManager::Get().GetSkeletalModels()->Pack(
            resources.model, exporter->compression_mode_));

    for (const auto& clip : resources.animation_clips) {
      scene_context->AddResourceFile(
          resource::ResourceManager::Get().GetAnimationClips()->Pack(
              clip, exporter->compression_mode_));
    }

  } else {
    const auto resources{LoadStaticModel(scene_context->allocator, scene,
                                         scene_context->asset_path)};

    scene_context->AddResourceFile(
        resource::ResourceManager::Get().GetStaticModels()->Pack(
            resources.model, exporter->compression_mode_));
  }

  COMET_LOG_DEBUG(LoggerType::External, "ModelExporter::OnModelProcessing",
                  "model processed", "asset_path",
                  scene_context->asset_abs_path);
}

void ModelExporter::OnMaterialsProcessing(job::JobParamsHandle params_handle) {
  auto* scene_context{reinterpret_cast<SceneContext*>(params_handle)};
  COMET_ASSERT(scene_context != nullptr, "ModelExporter::OnMaterialsProcessing",
               "scene context is null");

  auto* exporter{scene_context->exporter};
  COMET_ASSERT(exporter != nullptr, "ModelExporter::OnMaterialsProcessing",
               "exporter is null");

  COMET_LOG_DEBUG(LoggerType::External, "ModelExporter::OnMaterialsProcessing",
                  "processing model materials", "asset_path",
                  scene_context->asset_abs_path);

  exporter->LoadMaterials(scene_context);

  COMET_LOG_DEBUG(LoggerType::External, "ModelExporter::OnMaterialsProcessing",
                  "model materials processed", "asset_path",
                  scene_context->asset_abs_path);
}

void ModelExporter::LoadMaterials(SceneContext* scene_context) const {
  auto* scene{scene_context->scene};
  const auto directory_path{GetDirectoryPath(scene_context->asset_abs_path)};
  const auto resource_path{GetRelativePath(directory_path, root_asset_path_)};

  for (usize i{0}; i < scene->mNumMaterials; ++i) {
    auto* raw_material{scene->mMaterials[i]};

    resource::MaterialResource material{};
    material.descr.shader_resource_id = resource::ShaderResourceId::Invalid();

    InitializeDefaultTextureMap(material.descr.diffuse_map,
                                rendering::TextureType::Diffuse);
    InitializeDefaultTextureMap(material.descr.specular_map,
                                rendering::TextureType::Specular);
    InitializeDefaultTextureMap(material.descr.normal_map,
                                rendering::TextureType::Normal);

    if (aiGetMaterialFloat(raw_material, AI_MATKEY_SHININESS,
                           &material.descr.shininess) != AI_SUCCESS) {
      COMET_LOG_WARNING(LoggerType::External, "ModelExporter::LoadMaterials",
                        "material shininess is missing, using default",
                        "material", raw_material->GetName().C_Str(),
                        "default_shininess", kDefaultMaterialShininess_);
      material.descr.shininess = kDefaultMaterialShininess_;
    }

    aiColor3D color{};

    if (raw_material->Get(AI_MATKEY_COLOR_DIFFUSE, color) != AI_SUCCESS) {
      COMET_LOG_WARNING(LoggerType::External, "ModelExporter::LoadMaterials",
                        "material diffuse color is missing, using default",
                        "material", raw_material->GetName().C_Str(),
                        "default_r", kDefaultColor_.r, "default_g",
                        kDefaultColor_.g, "default_b", kDefaultColor_.b);
      color = kDefaultColor_;
    }

    material.descr.diffuse_color = math::Vec4{color.r, color.g, color.b, 1.0f};

    LoadMaterialTextures(resource_path, material, raw_material,
                         aiTextureType_BASE_COLOR);
    if (!material.descr.diffuse_map.texture_resource_id) {
      LoadMaterialTextures(resource_path, material, raw_material,
                           aiTextureType_DIFFUSE);
    }

    LoadMaterialTextures(resource_path, material, raw_material,
                         aiTextureType_SPECULAR);
    LoadMaterialTextures(resource_path, material, raw_material,
                         aiTextureType_NORMALS);

    if (!material.descr.normal_map.texture_resource_id) {
      LoadMaterialTextures(resource_path, material, raw_material,
                           aiTextureType_HEIGHT);
    }

    const auto material_resource_id{resource::GenerateQualifiedMaterialId(
        scene_context->asset_path, raw_material->GetName().C_Str(),
        static_cast<u32>(i))};

    material.id = material_resource_id.GetValue();
    material.type_id = resource::MaterialResource::kResourceTypeId;

    scene_context->AddResourceFile(
        resource::ResourceManager::Get().GetMaterials()->Pack(
            material, compression_mode_));
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
  COMET_ASSERT(resource_files != nullptr,
               "ModelExporter::SceneContext::AddResourceFile",
               "resource files container is null");

  fiber::FiberLockGuard lock{resource_mutex};
  resource_files->PushBack(file);
}
}  // namespace asset
}  // namespace editor
}  // namespace comet