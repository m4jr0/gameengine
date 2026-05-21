// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/runtime/animation/utils/animation_pose_utils.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/compression.h"
#include "comet/core/frame/frame_container.h"
#include "comet/math/math_compression.h"
#include "comet/math/math_interpolation.h"
#include "comet/math/math_scalar.h"
#include "comet/math/quaternion.h"
#include "comet/math/vector.h"
#include "comet/profiler/profiler.h"

namespace comet {
namespace animation {
AnimationPose PopulatePoseFromSample(const CompressedAnimationClip& clip,
                                     FrameIndex frame) {
  COMET_PROFILE("PopulatePoseFromSample");

  const auto& sample{clip.samples[frame]};
  const auto joint_pose_count{sample.joint_poses.GetSize()};

  AnimationPose out_pose{};
  out_pose.local_pose =
      COMET_FRAME_ARRAY_WITH_CAPACITY(JointPose, joint_pose_count);

  for (usize i{0}; i < joint_pose_count; ++i) {
    const auto pose{DecompressJointPose(sample.joint_poses[i])};
    out_pose.local_pose->PushLast(pose);
  }

  return out_pose;
}

AnimationPose PopulatePoseFromSamples(const CompressedAnimationClip& clip,
                                      FrameIndex frame_a, FrameIndex frame_b,
                                      f32 alpha) {
  COMET_PROFILE("PopulatePoseFromSamples");

  if (alpha <= .0f) {
    return PopulatePoseFromSample(clip, frame_a);
  }

  if (alpha >= 1.0f) {
    return PopulatePoseFromSample(clip, frame_b);
  }

  const auto& a_sample{clip.samples[frame_a]};
  const auto& b_sample{clip.samples[frame_b]};

  COMET_ASSERT(
      a_sample.joint_poses.GetSize() == b_sample.joint_poses.GetSize(),
      "animation_clip_utils::PopulatePoseFromSamples", "joint count mismatch",
      "clip", COMET_STRING_ID_LABEL(clip.id.GetValue()), "frame_a", frame_a,
      "frame_b", frame_b, "joint_count_a", a_sample.joint_poses.GetSize(),
      "joint_count_b", b_sample.joint_poses.GetSize());

  const auto joint_pose_count{a_sample.joint_poses.GetSize()};

  AnimationPose out_pose{};
  out_pose.local_pose =
      COMET_FRAME_ARRAY_WITH_CAPACITY(JointPose, joint_pose_count);

  for (usize i{0}; i < joint_pose_count; ++i) {
    const auto pose_a{DecompressJointPose(a_sample.joint_poses[i])};
    const auto pose_b{DecompressJointPose(b_sample.joint_poses[i])};

    auto& joint{out_pose.local_pose->EmplaceLast()};
    joint.translation =
        math::Lerp(pose_a.translation, pose_b.translation, alpha);
    joint.scale = math::Lerp(pose_a.scale, pose_b.scale, alpha);
    joint.rotation = math::Slerp(pose_a.rotation, pose_b.rotation, alpha);
  }

  return out_pose;
}

AnimationPose DecompressClipAndExtractPose(const CompressedAnimationClip& clip,
                                           f64 time, f32 speed,
                                           AnimationOverrideFlags overrides,
                                           bool is_loop) {
  COMET_PROFILE("DecompressClipAndExtractPose");

  const auto ticks_per_frame{1.0 / clip.frames_per_second};
  const auto duration{clip.frame_count * ticks_per_frame};

  const auto effective_is_loop{
      (overrides & kAnimationOverrideFlagBitsIsLoop) != 0 ? is_loop
                                                          : clip.is_loop};

  time *= speed;
  time = effective_is_loop ? math::Fmod(time, duration)
                           : math::Min(time, duration);

  const auto frame{math::Min(static_cast<FrameIndex>(time / ticks_per_frame),
                             clip.frame_count - 1)};
  const auto next_frame{effective_is_loop
                            ? (frame + 1) % clip.frame_count
                            : math::Min(frame + 1, clip.frame_count - 1)};
  const auto alpha{
      static_cast<f32>((time - (frame * ticks_per_frame)) / ticks_per_frame)};

  return PopulatePoseFromSamples(clip, frame, next_frame, alpha);
}

void PopulateGlobalPose(const geometry::Skeleton& skeleton,
                        AnimationPose& pose) {
  COMET_PROFILE("PopulateGlobalPose");

  const auto joint_count{skeleton.joints.GetSize()};

  COMET_ASSERT(joint_count == pose.local_pose->GetSize(),
               "animation_clip_utils::PopulateGlobalPose",
               "joint count mismatch", "skeleton", skeleton.id, "joint_count",
               joint_count, "pose_joint_count", pose.local_pose->GetSize());

  pose.global_pose =
      COMET_FRAME_ARRAY_WITH_CAPACITY(math::Mat4, pose.local_pose->GetSize());

  for (usize i{0}; i < joint_count; ++i) {
    const auto& local_pose{pose.local_pose->Get(i)};
    const auto& joint{skeleton.joints[i]};

    const auto local_matrix{math::ComposeTransform(
        local_pose.translation, local_pose.rotation, local_pose.scale)};

    if (joint.parent_index == geometry::kInvalidSkeletonJointIndex) {
      pose.global_pose->PushLast(local_matrix);
    } else {
      auto& parent{pose.global_pose->Get(joint.parent_index)};
      pose.global_pose->PushLast(parent * local_matrix);
    }
  }
}
}  // namespace animation
}  // namespace comet