// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_ANIMATION_UTILS_ANIMATION_CLIP_UTILS_H_
#define COMET_COMET_ANIMATION_UTILS_ANIMATION_CLIP_UTILS_H_

#include "comet/animation/type/animation_clip_type.h"
#include "comet/animation/type/animation_pose_type.h"
#include "comet/animation/type/animation_skinning_type.h"
#include "comet/core/essentials.h"
#include "comet/core/type/array.h"
#include "comet/entity/entity_id.h"
#include "comet/geometry/type/geometry_skeleton_type.h"
#include "comet/math/matrix.h"

namespace comet {
namespace animation {
CompressedJointPose CompressJointPose(const JointPose& pose);
JointPose DecompressJointPose(const CompressedJointPose& compressed_pose);

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

void PopulateSkinningBinding(entity::EntityId entity_id, u32 joint_count,
                             u32 matrix_offset, SkinningBinding& binding);

void PopulateMatrixPalette(const geometry::Skeleton* skeleton,
                           const Array<math::Mat4>* global_pose,
                           MatrixPalette& matrix_palette);
}  // namespace animation
}  // namespace comet

#endif  // COMET_COMET_ANIMATION_UTILS_ANIMATION_CLIP_UTILS_H_