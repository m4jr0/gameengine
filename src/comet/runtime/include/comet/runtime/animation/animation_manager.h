// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RUNTIME_ANIMATION_ANIMATION_MANAGER_H_
#define COMET_RUNTIME_ANIMATION_ANIMATION_MANAGER_H_

// External. ///////////////////////////////////////////////////////////////////
#include <optional>
////////////////////////////////////////////////////////////////////////////////

#include "comet/runtime/animation/component/animation_component.h"
#include "comet/core/job/job.h"
#include "comet/core/essentials.h"
#include "comet/runtime/frame/frame_packet.h"
#include "comet/runtime/resource/animation/animation_resource_handle.h"
#include "comet/runtime/manager.h"
#include "comet/runtime/entity/entity_id.h"
#include "comet/data/resource/common.h"

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
  ~AnimationManager() override = default;

  void Update(frame::FramePacket* packet);

  void Play(entity::EntityId entity_id, const schar* qualified_name,
            f32 speed = 1.0f, std::optional<bool> is_loop = std::nullopt);
  void Play(entity::EntityId entity_id, const wchar* qualified_name,
            f32 speed = 1.0f, std::optional<bool> is_loop = std::nullopt);
  void Play(entity::EntityId entity_id, AnimationClipId id, f32 speed = 1.0f,
            std::optional<bool> is_loop = std::nullopt);

  AnimationComponent GenerateAnimationComponent(
      const schar* qualified_name, f32 speed = 1.0f,
      std::optional<bool> is_loop = std::nullopt,
      resource::ResourceLifeSpan life_span =
          resource::ResourceLifeSpan::Manual);
  AnimationComponent GenerateAnimationComponent(
      const wchar* qualified_name, f32 speed = 1.0f,
      std::optional<bool> is_loop = std::nullopt,
      resource::ResourceLifeSpan life_span =
          resource::ResourceLifeSpan::Manual);
  AnimationComponent GenerateAnimationComponent(
      AnimationClipId id = AnimationClipId::Invalid(), f32 speed = 1.0f,
      std::optional<bool> is_loop = std::nullopt,
      resource::ResourceLifeSpan life_span =
          resource::ResourceLifeSpan::Manual);

  void DestroyAnimationComponent(AnimationComponent* animation_cmp);

 private:
  static void OnAnimationProcessing(job::JobParamsHandle params_handle);

  void PlayInternal(entity::EntityId entity_id, AnimationClipResourceHandle handle,
                    f32 speed = 1.0f,
                    std::optional<bool> is_loop = std::nullopt);

  f64 last_time_{.0f};
};
}  // namespace animation
}  // namespace comet

#endif  // COMET_RUNTIME_ANIMATION_ANIMATION_MANAGER_H_
