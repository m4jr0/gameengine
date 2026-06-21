// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RUNTIME_ANIMATION_ANIMATION_POSE_H_
#define COMET_RUNTIME_ANIMATION_ANIMATION_POSE_H_

#include "comet/data/animation/animation_clip.h"
#include "comet/core/essentials.h"
#include "comet/core/container/array.h"
#include "comet/core/math/matrix.h"

namespace comet {
namespace animation {
struct AnimationPose {
  Array<JointPose>* local_pose{nullptr};
  Array<math::Mat4>* global_pose{nullptr};
};
}  // namespace animation
}  // namespace comet

#endif  // COMET_RUNTIME_ANIMATION_ANIMATION_POSE_H_