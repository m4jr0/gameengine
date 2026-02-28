// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_ANIMATION_COMPONENT_ANIMATION_COMPONENT_H_
#define COMET_COMET_ANIMATION_COMPONENT_ANIMATION_COMPONENT_H_

#include "comet/animation/animation_common.h"
#include "comet/core/essentials.h"
#include "comet/resource/animation_resource.h"

namespace comet {
namespace animation {
struct AnimationComponent {
  const resource::AnimationClipResource* clip_resource{nullptr};
  f64 start_time{.0f};
  FrameIndex frame{0};
  f32 speed{1.0f};
  bool is_loop{false};
  AnimationOverrideFlags override_flags{kAnimationOverrideFlagBitsNone};
};
}  // namespace animation
}  // namespace comet

#endif  // COMET_COMET_ANIMATION_COMPONENT_ANIMATION_COMPONENT_H_
