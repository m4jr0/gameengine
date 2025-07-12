// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_ANIMATION_ANIMATION_MANAGER_H_
#define COMET_COMET_ANIMATION_ANIMATION_MANAGER_H_

#include <optional>

#include "comet/animation/animation_common.h"
#include "comet/animation/component/animation_component.h"
#include "comet/core/concurrency/job/job.h"
#include "comet/core/essentials.h"
#include "comet/core/frame/frame_packet.h"
#include "comet/core/manager.h"
#include "comet/entity/entity_id.h"
#include "comet/resource/animation_resource.h"
#include "comet/resource/resource.h"

namespace comet {
namespace animation {
namespace internal {
struct AnimationJob {
  usize index{kInvalidIndex};
  f64 time{.0f};
  entity::EntityId entity_id{entity::kInvalidEntityId};
  frame::SkinningBindings* skinning_bindings{nullptr};
  frame::MatrixPalettes* matrix_palettes{nullptr};
};
}  // namespace internal

class AnimationManager : public Manager {
 public:
  static AnimationManager& Get();

  AnimationManager() = default;
  AnimationManager(const AnimationManager&) = delete;
  AnimationManager(AnimationManager&&) = delete;
  AnimationManager& operator=(const AnimationManager&) = delete;
  AnimationManager& operator=(AnimationManager&&) = delete;
  virtual ~AnimationManager() = default;

  void Update(frame::FramePacket* packet);

  void Play(entity::EntityId entity_id, const schar* name, f32 speed = 1.0f,
            std::optional<bool> is_loop = std::nullopt);
  void Play(entity::EntityId entity_id, const wchar* name, f32 speed = 1.0f,
            std::optional<bool> is_loop = std::nullopt);
  void Play(entity::EntityId entity_id, AnimationClipId id, f32 speed = 1.0f,
            std::optional<bool> is_loop = std::nullopt);

  AnimationComponent GenerateAnimationComponent(
      const schar* name, f32 speed = 1.0f,
      std::optional<bool> is_loop = std::nullopt,
      resource::ResourceLifeSpan life_span =
          resource::ResourceLifeSpan::Manual);
  AnimationComponent GenerateAnimationComponent(
      const wchar* name, f32 speed = 1.0f,
      std::optional<bool> is_loop = std::nullopt,
      resource::ResourceLifeSpan life_span =
          resource::ResourceLifeSpan::Manual);
  AnimationComponent GenerateAnimationComponent(
      AnimationClipId id = kInvalidAnimationClipId, f32 speed = 1.0f,
      std::optional<bool> is_loop = std::nullopt,
      resource::ResourceLifeSpan life_span =
          resource::ResourceLifeSpan::Manual);

  void DestroyAnimationComponent(AnimationComponent* animation_cmp);

 private:
  static void OnAnimationProcessing(job::JobParamsHandle params_handle);

  void PlayInternal(entity::EntityId entity_id,
                    const resource::AnimationClipResource* resource,
                    f32 speed = 1.0f,
                    std::optional<bool> is_loop = std::nullopt);

  f64 last_time_{.0f};
};
}  // namespace animation
}  // namespace comet

#endif  // COMET_COMET_ANIMATION_ANIMATION_MANAGER_H_
