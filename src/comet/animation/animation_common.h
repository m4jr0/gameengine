// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_ANIMATION_ANIMATION_COMMON_H_
#define COMET_COMET_ANIMATION_ANIMATION_COMMON_H_

#include "comet/core/essentials.h"
#include "comet/core/type/array.h"
#include "comet/geometry/geometry_common.h"

namespace comet {
namespace animation {
struct AnimationSample {
  Array<geometry::JointPose*> joint_poses{};
};

using FrameIndex = u32;

struct AnimationClip {
  geometry::Skeleton* skeleton{nullptr};
  FrameIndex frames_per_second{0};
  FrameIndex frame_count{0};
  Array<AnimationSample> resource_samples{};
  bool is_loop{false};
};
}  // namespace animation
}  // namespace comet

#endif  // COMET_COMET_ANIMATION_ANIMATION_COMMON_H_
