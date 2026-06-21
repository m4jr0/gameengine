// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RUNTIME_ANIMATION_COMPONENT_ANIMATION_COMPONENT_H_
#define COMET_RUNTIME_ANIMATION_COMPONENT_ANIMATION_COMPONENT_H_

#include "comet/data/animation/animation_id.h"
#include "comet/data/animation/animation_clip.h"
#include "comet/runtime/resource/animation/animation_resource_handle.h"
#include "comet/runtime/animation/animation_override.h"
#include "comet/runtime/animation/animation_pose.h"
#include "comet/core/essentials.h"

namespace comet {
namespace animation {
struct AnimationComponent {
  AnimationClipResourceHandle clip_handle{};
  f64 start_time{.0f};
  FrameIndex frame{0};
  f32 speed{1.0f};
  bool is_loop{false};
  AnimationOverrideFlags override_flags{kAnimationOverrideFlagBitsNone};
};
}  // namespace animation
}  // namespace comet

#endif  // COMET_RUNTIME_ANIMATION_COMPONENT_ANIMATION_COMPONENT_H_
