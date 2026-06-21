// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_runtime_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/runtime/animation/animation_manager.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/data/animation/animation_clip.h"
#include "comet/runtime/animation/animation_pose.h"
#include "comet/data/animation/utils/animation_clip_utils.h"
#include "comet/core/fiber/fiber.h"
#include "comet/core/job/job_utils.h"
#include "comet/runtime/animation/animation_override.h"
#include "comet/runtime/animation/utils/animation_skinning_utils.h"
#include "comet/core/job/scheduler.h"
#include "comet/runtime/entity/entity_manager.h"
#include "comet/runtime/geometry/component/skeleton_component.h"
#include "comet/runtime/profiler/profiler.h"
#include "comet/data/resource/animation/animation_resource.h"
#include "comet/runtime/resource/resource_manager.h"
#include "comet/runtime/scene/scene_manager.h"

namespace comet {
namespace animation {
AnimationManager& AnimationManager::Get() {
  static AnimationManager singleton{};
  return singleton;
}

void AnimationManager::Update(frame::FramePacket* packet) {
  COMET_PROFILE("AnimationManager::Update");
  COMET_ASSERT(packet != nullptr, "AnimationManager::Update",
               "frame packet is null");

  last_time_ = packet->time + packet->lag;

  auto& entity_manager{entity::EntityManager::Get()};
  auto* entity_ids{COMET_FRAME_ARRAY_WITH_CAPACITY(
      entity::EntityId, scene::SceneManager::Get().GetExpectedEntityCount())};

  entity_manager.ForEachId<geometry::SkeletonComponent, AnimationComponent>(
      [&](auto entity_id) { entity_ids->PushLast(entity_id); });

  const auto entity_count{entity_ids->GetSize()};
  packet->skinning_bindings->Resize(entity_count);
  packet->matrix_palettes->Resize(entity_count);

  auto* jobs{
      COMET_FRAME_ARRAY_WITH_CAPACITY(internal::AnimationJob, entity_count)};
  auto& scheduler{job::Scheduler::Get()};
  job::CounterGuard guard{};

  for (usize i{0}; i < entity_count; ++i) {
    const auto entity_id{entity_ids->Get(i)};

    auto* animation_cmp{
        entity_manager.GetComponent<AnimationComponent>(entity_id)};

    if (!animation_cmp->clip_handle) {
      continue;
    }

#ifdef COMET_FIBER_DEBUG_LABEL
    schar debug_label[fiber::Fiber::kDebugLabelMaxLen_ + 1];
    const auto anim_entity_id_len{GetLength("anim_entity_id_")};
    Copy(debug_label, "anim_entity_id_", anim_entity_id_len);
    ConvertToStr(entity_id, debug_label + anim_entity_id_len,
                 fiber::Fiber::kDebugLabelMaxLen_ - anim_entity_id_len);
#else
    const schar* debug_label{nullptr};
#endif  // COMET_FIBER_DEBUG_LABEL

    auto& job{jobs->EmplaceLast()};
    job.index = i;
    job.entity_id = entity_id;
    job.time = last_time_;
    job.skinning_bindings = packet->skinning_bindings;
    job.matrix_palettes = packet->matrix_palettes;

    scheduler.Kick(job::GenerateJobDescr(
        job::JobPriority::High, OnAnimationProcessing, &job,
        job::JobStackSize::Normal, guard.GetCounter(), debug_label));
  }

  guard.Wait();
}

void AnimationManager::Play(entity::EntityId entity_id,
                            const schar* qualified_name, f32 speed,
                            std::optional<bool> is_loop) {
  COMET_PROFILE("AnimationManager::Play");
  Play(entity_id, GenerateAnimationClipId(qualified_name), speed, is_loop);
}

void AnimationManager::Play(entity::EntityId entity_id,
                            const wchar* qualified_name, f32 speed,
                            std::optional<bool> is_loop) {
  COMET_PROFILE("AnimationManager::Play");
  Play(entity_id, GenerateAnimationClipId(qualified_name), speed, is_loop);
}

void AnimationManager::Play(entity::EntityId entity_id, AnimationClipId id,
                            f32 speed, std::optional<bool> is_loop) {
  COMET_PROFILE("AnimationManager::Play");
  auto* animation_cmp{
      entity::EntityManager::Get().GetComponent<AnimationComponent>(entity_id)};
  COMET_ASSERT(animation_cmp != nullptr, "AnimationManager::Play",
               "no animation component", "entity", entity_id);

  auto* handler{resource::ResourceManager::Get().GetAnimationClips()};

  if (animation_cmp->clip_handle) {
    const auto* current_resource{handler->Get(animation_cmp->clip_handle)};

    if (current_resource != nullptr && current_resource->clip.id == id) {
      animation_cmp->start_time = last_time_;
      animation_cmp->speed = speed;
      animation_cmp->override_flags = kAnimationOverrideFlagBitsNone;

      if (is_loop.has_value()) {
        animation_cmp->override_flags |= kAnimationOverrideFlagBitsIsLoop;
        animation_cmp->is_loop = is_loop.value();
      } else {
        animation_cmp->is_loop = false;
      }

      return;
    }

    handler->Unload(animation_cmp->clip_handle);
    animation_cmp->clip_handle.Invalidate();
  }

  const auto handle{handler->Load(id)};

  if (!handle) {
    return;
  }

  PlayInternal(entity_id, handle, speed, is_loop);
}

AnimationComponent AnimationManager::GenerateAnimationComponent(
    const schar* qualified_name, f32 speed, std::optional<bool> is_loop,
    resource::ResourceLifeSpan life_span) {
  return GenerateAnimationComponent(GenerateAnimationClipId(qualified_name),
                                    speed, is_loop, life_span);
}

AnimationComponent AnimationManager::GenerateAnimationComponent(
    const wchar* qualified_name, f32 speed, std::optional<bool> is_loop,
    resource::ResourceLifeSpan life_span) {
  return GenerateAnimationComponent(GenerateAnimationClipId(qualified_name),
                                    speed, is_loop, life_span);
}

AnimationComponent AnimationManager::GenerateAnimationComponent(
    AnimationClipId id, f32 speed, std::optional<bool> is_loop,
    resource::ResourceLifeSpan life_span) {
  AnimationComponent animation_cmp{};

  if (id) {
    animation_cmp.clip_handle =
        resource::ResourceManager::Get().GetAnimationClips()->Load(id,
                                                                   life_span);
  } else {
    animation_cmp.clip_handle = AnimationClipResourceHandle::Invalid();
  }

  animation_cmp.start_time = .0f;
  animation_cmp.frame = 0;
  animation_cmp.speed = speed;
  animation_cmp.override_flags = kAnimationOverrideFlagBitsNone;

  if (is_loop.has_value()) {
    animation_cmp.override_flags |= kAnimationOverrideFlagBitsIsLoop;
    animation_cmp.is_loop = is_loop.value();
  }

  return animation_cmp;
}

void AnimationManager::DestroyAnimationComponent(
    AnimationComponent* animation_cmp) {
  if (animation_cmp->clip_handle) {
    resource::ResourceManager::Get().GetAnimationClips()->Unload(
        animation_cmp->clip_handle);
    animation_cmp->clip_handle.Invalidate();
  }

  animation_cmp->start_time = .0f;
  animation_cmp->frame = 0;
  animation_cmp->speed = 1.0f;
  animation_cmp->override_flags = kAnimationOverrideFlagBitsNone;
  animation_cmp->is_loop = false;
}

void AnimationManager::OnAnimationProcessing(
    job::JobParamsHandle params_handle) {
  auto& entity_manager{entity::EntityManager::Get()};
  auto* job{static_cast<internal::AnimationJob*>(params_handle)};
  const auto index{job->index};
  const auto time{job->time};
  const auto entity_id{job->entity_id};
  auto* skinning_bindings{job->skinning_bindings};
  auto* matrix_palettes{job->matrix_palettes};

  auto* skeleton_cmp{
      entity_manager.GetComponent<geometry::SkeletonComponent>(entity_id)};

  auto* animation_cmp{
      entity_manager.GetComponent<AnimationComponent>(entity_id)};

  const auto animation_time{time - animation_cmp->start_time};

  auto* clip_resource{resource::ResourceManager::Get().GetAnimationClips()->Get(
      animation_cmp->clip_handle)};

  if (clip_resource == nullptr) {
    return;
  }

  auto pose{DecompressClipAndExtractPose(
      clip_resource->clip, animation_time, animation_cmp->speed,
      animation_cmp->override_flags, animation_cmp->is_loop)};

  // TODO(m4jr0): Support pose blending.
  auto& resource_manager{resource::ResourceManager::Get()};

  const auto* skeleton_resource =
      resource_manager.GetSkeletons()->Get(skeleton_cmp->resource_handle);

  const auto& skeleton = skeleton_resource->skeleton;
  PopulateGlobalPose(skeleton, pose);

  auto& binding{skinning_bindings->Get(index)};
  PopulateSkinningBinding(entity_id,
                          static_cast<u32>(skeleton.joints.GetSize()),
                          static_cast<u32>(index), binding);

  // TODO(m4jr0): Support some post-process poses.

  auto& matrix_palette{matrix_palettes->Get(index)};
  PopulateMatrixPalette(&skeleton, pose.global_pose, matrix_palette);
}

void AnimationManager::PlayInternal(entity::EntityId entity_id,
                                    AnimationClipResourceHandle handle, f32 speed,
                                    std::optional<bool> is_loop) {
  COMET_PROFILE("AnimationManager::PlayInternal");
  COMET_ASSERT(handle, "AnimationManager::PlayInternal",
               "animation handle is invalid", "entity", entity_id);

  auto* animation_cmp{
      entity::EntityManager::Get().GetComponent<AnimationComponent>(entity_id)};
  COMET_ASSERT(animation_cmp != nullptr, "AnimationManager::PlayInternal",
               "no animation component", "entity", entity_id);

  auto* handler{resource::ResourceManager::Get().GetAnimationClips()};

  if (animation_cmp->clip_handle) {
    handler->Unload(animation_cmp->clip_handle);
  }

  animation_cmp->clip_handle = handle;
  animation_cmp->start_time = last_time_;
  animation_cmp->speed = speed;
  animation_cmp->override_flags = kAnimationOverrideFlagBitsNone;

  if (is_loop.has_value()) {
    animation_cmp->override_flags |= kAnimationOverrideFlagBitsIsLoop;
    animation_cmp->is_loop = is_loop.value();
  } else {
    animation_cmp->is_loop = false;
  }
}
}  // namespace animation
}  // namespace comet
