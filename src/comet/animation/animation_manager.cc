// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "animation_manager.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/concurrency/fiber/fiber.h"
#include "comet/core/concurrency/job/job_utils.h"
#include "comet/core/concurrency/job/scheduler.h"
#include "comet/entity/entity_id.h"
#include "comet/entity/entity_manager.h"
#include "comet/geometry/component/skeleton_component.h"
#include "comet/resource/animation_resource.h"
#include "comet/resource/resource_manager.h"
#include "comet/scene/scene_manager.h"

namespace comet {
namespace animation {
AnimationManager& AnimationManager::Get() {
  static AnimationManager singleton{};
  return singleton;
}

void AnimationManager::Update(frame::FramePacket* packet) {
  last_time_ = packet->time + packet->lag;

  auto& entity_manager{entity::EntityManager::Get()};
  auto* entity_ids{COMET_FRAME_ARRAY(
      entity::EntityId, scene::SceneManager::Get().GetExpectedEntityCount())};

  entity_manager.Each<geometry::SkeletonComponent, AnimationComponent>(
      [&](auto entity_id) { entity_ids->PushBack(entity_id); });

  auto entity_count{entity_ids->GetSize()};
  packet->skinning_bindings->Resize(entity_count);
  packet->matrix_palettes->Resize(entity_count);

  auto* jobs{COMET_FRAME_ARRAY(internal::AnimationJob, entity_count)};
  auto& scheduler{job::Scheduler::Get()};
  job::CounterGuard guard{};

  for (usize i{0}; i < entity_count; ++i) {
    auto entity_id{entity_ids->Get(i)};

    auto* animation_cmp{
        entity_manager.GetComponent<AnimationComponent>(entity_id)};

    if (animation_cmp->clip_resource == nullptr) {
      continue;
    }

#ifdef COMET_FIBER_DEBUG_LABEL
    schar debug_label[fiber::Fiber::kDebugLabelMaxLen_ + 1];
    auto anim_entity_id_len{GetLength("anim_entity_id_")};
    Copy(debug_label, "anim_entity_id_", anim_entity_id_len);
    ConvertToStr(entity_id, debug_label + anim_entity_id_len,
                 fiber::Fiber::kDebugLabelMaxLen_ - anim_entity_id_len);
#else
    const schar* debug_label{nullptr};
#endif  // COMET_FIBER_DEBUG_LABEL

    auto& job{jobs->EmplaceBack()};
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

void AnimationManager::Play(entity::EntityId entity_id, const schar* name,
                            f32 speed, std::optional<bool> is_loop) {
  Play(entity_id, COMET_STRING_ID(name), speed, is_loop);
}

void AnimationManager::Play(entity::EntityId entity_id, const wchar* name,
                            f32 speed, std::optional<bool> is_loop) {
  Play(entity_id, COMET_STRING_ID(name), speed, is_loop);
}

void AnimationManager::Play(entity::EntityId entity_id, AnimationClipId id,
                            f32 speed, std::optional<bool> is_loop) {
  auto* animation_cmp{
      entity::EntityManager::Get().GetComponent<AnimationComponent>(entity_id)};
  const resource::AnimationClipResource* resource{nullptr};

  const auto* existing_resource{
      animation_cmp != nullptr ? animation_cmp->clip_resource : nullptr};
  auto* resource_animation_clips_handler{
      resource::ResourceManager::Get().GetAnimationClips()};

  if (existing_resource == nullptr) {
    resource = resource_animation_clips_handler->Load(
        static_cast<resource::ResourceId>(id));
  } else {
    if (existing_resource->id == id) {
      resource = existing_resource;
    } else {
      resource_animation_clips_handler->Unload(existing_resource->id);
      resource = resource_animation_clips_handler->Load(
          static_cast<resource::ResourceId>(id));
    }
  }

  if (resource == nullptr) {
    return;
  }

  PlayInternal(entity_id, resource, speed, is_loop);
}

AnimationComponent AnimationManager::GenerateAnimationComponent(
    const schar* name, f32 speed, std::optional<bool> is_loop,
    resource::ResourceLifeSpan life_span) {
  return GenerateAnimationComponent(COMET_STRING_ID(name), speed, is_loop,
                                    life_span);
}

AnimationComponent AnimationManager::GenerateAnimationComponent(
    const wchar* name, f32 speed, std::optional<bool> is_loop,
    resource::ResourceLifeSpan life_span) {
  return GenerateAnimationComponent(COMET_STRING_ID(name), speed, is_loop,
                                    life_span);
}

AnimationComponent AnimationManager::GenerateAnimationComponent(
    AnimationClipId id, f32 speed, std::optional<bool> is_loop,
    resource::ResourceLifeSpan life_span) {
  AnimationComponent animation_cmp{};

  if (id != kInvalidAnimationClipId) {
    animation_cmp.clip_resource =
        resource::ResourceManager::Get().GetAnimationClips()->Load(id,
                                                                   life_span);
  } else {
    animation_cmp.clip_resource = nullptr;
  }

  animation_cmp.start_time = .0f;
  animation_cmp.frame = 0;
  animation_cmp.speed = speed;

  if (is_loop.has_value()) {
    animation_cmp.override_flags |= kAnimationOverrideFlagBitsIsLoop;
    animation_cmp.is_loop = is_loop.value();
  } else {
    animation_cmp.is_loop = false;
    animation_cmp.override_flags = kAnimationOverrideFlagBitsNone;
  }

  return animation_cmp;
}

void AnimationManager::DestroyAnimationComponent(
    AnimationComponent* animation_cmp) {
  if (animation_cmp->clip_resource != nullptr) {
    resource::ResourceManager::Get().GetAnimationClips()->Unload(
        animation_cmp->clip_resource->id);
    animation_cmp->clip_resource = nullptr;
  }

  animation_cmp->start_time = .0f;
  animation_cmp->frame = 0;
  animation_cmp->speed = 1.0f;
  animation_cmp->is_loop = false;
  animation_cmp->override_flags = kAnimationOverrideFlagBitsNone;
}

void AnimationManager::OnAnimationProcessing(
    job::JobParamsHandle params_handle) {
  auto& entity_manager{entity::EntityManager::Get()};
  auto* job{static_cast<internal::AnimationJob*>(params_handle)};
  auto index{job->index};
  auto time{job->time};
  auto entity_id{job->entity_id};
  auto* skinning_bindings{job->skinning_bindings};
  auto* matrix_palettes{job->matrix_palettes};

  auto* skeleton_cmp{
      entity_manager.GetComponent<geometry::SkeletonComponent>(entity_id)};

  auto* animation_cmp{
      entity_manager.GetComponent<AnimationComponent>(entity_id)};

  auto animation_time{time - animation_cmp->start_time};

  auto pose{DecompressClipAndExtractPose(
      animation_cmp->clip_resource->clip, animation_time, animation_cmp->speed,
      animation_cmp->override_flags, animation_cmp->is_loop)};

  // TODO(m4jr0): Support pose blending.

  const auto& skeleton{skeleton_cmp->resource->skeleton};
  PopulateGlobalPose(skeleton, pose);

  auto& binding{skinning_bindings->Get(index)};
  PopulateSkinningBinding(entity_id,
                          static_cast<u32>(skeleton.joints.GetSize()),
                          static_cast<u32>(index), binding);

  // TODO(m4jr0): Support some post-process poses.

  auto& matrix_palette{matrix_palettes->Get(index)};
  PopulateMatrixPalette(&skeleton, pose.global_pose, matrix_palette);
}

void AnimationManager::PlayInternal(
    entity::EntityId entity_id, const resource::AnimationClipResource* resource,
    f32 speed, std::optional<bool> is_loop) {
  COMET_ASSERT(resource != nullptr,
               "Tried to play a non-existent animation clip for entity #",
               entity_id, "!");
  auto* animation_cmp{
      entity::EntityManager::Get().GetComponent<AnimationComponent>(entity_id)};
  COMET_ASSERT(animation_cmp != nullptr, "No animation component for entity #",
               entity_id, "!");

  if (animation_cmp->clip_resource != nullptr) {
    resource::ResourceManager::Get().GetAnimationClips()->Unload(
        animation_cmp->clip_resource->id);
  }

  animation_cmp->clip_resource = resource;
  animation_cmp->start_time = last_time_;
  animation_cmp->speed = speed;
  animation_cmp->override_flags = kAnimationOverrideFlagBitsNone;

  if (is_loop.has_value()) {
    animation_cmp->override_flags |= kAnimationOverrideFlagBitsIsLoop;
    animation_cmp->is_loop = is_loop.value();
  }
}
}  // namespace animation
}  // namespace comet
