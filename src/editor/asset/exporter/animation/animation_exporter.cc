// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "animation_exporter.h"

#include "assimp/postprocess.h"
#include "comet/core/concurrency/job/job_utils.h"
#include "comet/core/concurrency/job/scheduler.h"
#include "comet/core/file_system/file_system.h"
#include "comet/core/frame/frame_allocator.h"
#include "comet/core/generator.h"
#include "comet/core/logger.h"
#include "comet/core/memory/memory_utils.h"
#include "comet/resource/animation_resource.h"
#include "comet/resource/resource_manager.h"
#include "comet_pch.h"
#include "editor/asset/asset_utils.h"
#include "editor/asset/exporter/animation/animation_exporter_utils.h"

#ifdef COMET_FIBER_DEBUG_LABEL
#include "comet/core/concurrency/fiber/fiber.h"
#endif  // COMET_FIBER_DEBUG_LABEL

namespace comet {
namespace editor {
namespace asset {
bool AnimationExporter::IsCompatible(CTStringView extension) const {
  return extension == COMET_TCHAR("obj") || extension == COMET_TCHAR("fbx");
}

void AnimationExporter::PopulateFiles(ResourceFilesContext& context) const {
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

  scheduler.Kick(scene_context.GenerateAnimationsProcessingJobDescr(counter));
  scheduler.Wait(counter);
}

void AnimationExporter::OnSceneLoading(job::IOJobParamsHandle params_handle) {
  auto* scene_context{reinterpret_cast<SceneContext*>(params_handle)};
#ifdef COMET_WIDE_TCHAR
  auto length{GetLength(scene_context->asset_abs_path)};
  auto* scene_path{static_cast<schar*>(
      COMET_FRAME_ALLOC_ALIGNED(length * sizeof(tchar), alignof(tchar)))};
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
    COMET_LOG_RESOURCE_ERROR("Assimp error: ",
                             scene_context->assimp_importer.GetErrorString());
  }

  scene_context->scene = scene;
  COMET_LOG_GLOBAL_DEBUG("Scene loaded at ", scene_path);
}

void AnimationExporter::OnAnimationsProcessing(
    job::JobParamsHandle params_handle) {
  auto* scene_context{reinterpret_cast<SceneContext*>(params_handle)};
  auto* exporter{scene_context->exporter};
  auto* scene{scene_context->scene};

  if (!scene->HasAnimations()) {
    return;
  }

  COMET_LOG_GLOBAL_DEBUG(
      "Processing animations at: ", scene_context->asset_abs_path, "...");

  auto clips{LoadAnimationClips(scene_context->allocator, scene)};

  for (const auto& clip : clips) {
    scene_context->AddResourceFile(
        resource::ResourceManager::Get().GetResourceFile(
            clip, exporter->compression_mode_));
  }

  COMET_LOG_GLOBAL_DEBUG("Animations processed at: ",
                         scene_context->asset_abs_path);
}

job::IOJobDescr
AnimationExporter::SceneContext::GenerateSceneLoadingJobDescr() {
  return job::GenerateIOJobDescr(OnSceneLoading, this,
                                 job::Scheduler::Get().GenerateCounter());
}

job::JobDescr
AnimationExporter::SceneContext::GenerateAnimationsProcessingJobDescr(
    job::Counter* counter) {
#ifdef COMET_FIBER_DEBUG_LABEL
  auto* buffer{reinterpret_cast<schar*>(COMET_FRAME_ALLOC_ALIGNED(
      (fiber::Fiber::kDebugLabelMaxLen_ + 1) * sizeof(schar), alignof(schar)))};
#endif  // COMET_FIBER_DEBUG_LABEL

  return job::GenerateJobDescr(
      job::JobPriority::Normal, AnimationExporter::OnAnimationsProcessing, this,
      job::JobStackSize::Normal, counter,
      COMET_ASSET_HANDLE_FIBER_DEBUG_LABEL(asset_path, buffer,
                                           fiber::Fiber::kDebugLabelMaxLen_));
}

void AnimationExporter::SceneContext::AddResourceFile(
    const resource::ResourceFile& file) {
  fiber::FiberLockGuard lock{resource_mutex};
  resource_files->PushBack(file);
}
}  // namespace asset
}  // namespace editor
}  // namespace comet
