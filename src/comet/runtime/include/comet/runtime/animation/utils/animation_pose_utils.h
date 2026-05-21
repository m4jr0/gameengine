// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RUNTIME_ANIMATION_UTILS_ANIMATION_POSE_UTILS_H_
#define COMET_RUNTIME_ANIMATION_UTILS_ANIMATION_POSE_UTILS_H_

#include "comet/animation/type/animation_clip.h"
#include "comet/animation/type/animation_pose.h"
#include "comet/animation/type/animation_skinning.h"
#include "comet/core/essentials.h"
#include "comet/core/type/array.h"
#include "comet/entity/type/entity_id.h"
#include "comet/geometry/type/skeleton.h"
#include "comet/math/matrix.h"

namespace comet {
namespace animation {
AnimationPose PopulatePoseFromSample(const CompressedAnimationClip& clip,
                                     FrameIndex frame);
AnimationPose PopulatePoseFromSamples(const CompressedAnimationClip& clip,
                                      FrameIndex frame_a, FrameIndex frame_b,
                                      f32 alpha);

AnimationPose DecompressClipAndExtractPose(
    const CompressedAnimationClip& clip, f64 time, f32 speed = 1.0f,
    AnimationOverrideFlags overrides = kAnimationOverrideFlagBitsNone,
    bool is_loop = false);

void PopulateGlobalPose(const geometry::Skeleton& skeleton,
                        AnimationPose& pose);
}  // namespace animation
}  // namespace comet

#endif  // COMET_RUNTIME_ANIMATION_UTILS_ANIMATION_POSE_UTILS_H_