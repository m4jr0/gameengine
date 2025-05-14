// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "animation_common.h"

#include "comet/core/compression.h"
#include "comet/core/frame/frame_utils.h"
#include "comet/math/math_commons.h"
#include "comet/math/math_compression.h"
#include "comet/math/math_interpolation.h"
#include "comet/math/quaternion.h"
#include "comet/math/vector.h"
#include "comet/resource/model_resource.h"

namespace comet {
namespace animation {
CompressedJointPose CompressJointPose(const JointPose& pose) {
  CompressedJointPose compressed_pose{};
  u32 translation_x;
  u32 translation_y;
  u32 translation_z;

  math::CompressVec3Rl(pose.translation, -kMaxTranslation, kMaxTranslation,
                       kCompressionBitCount, translation_x, translation_y,
                       translation_z);

  compressed_pose.translation_x = static_cast<u16>(translation_x);
  compressed_pose.translation_y = static_cast<u16>(translation_y);
  compressed_pose.translation_z = static_cast<u16>(translation_z);

  // Only store x/y/z of quaternion: w is reconstructed on decompression.
  compressed_pose.rotation_x = static_cast<u16>(
      CompressF32Rl(pose.rotation.x, -1.0f, 1.0f, kCompressionBitCount));
  compressed_pose.rotation_y = static_cast<u16>(
      CompressF32Rl(pose.rotation.y, -1.0f, 1.0f, kCompressionBitCount));
  compressed_pose.rotation_z = static_cast<u16>(
      CompressF32Rl(pose.rotation.z, -1.0f, 1.0f, kCompressionBitCount));

  compressed_pose.scale =
      static_cast<u16>(CompressF32Rl(pose.scale, kCompressionBitCount));
  return compressed_pose;
}

JointPose DecompressJointPose(const CompressedJointPose& compressed_pose) {
  JointPose pose{};
  pose.translation = math::DecompressVec3Rl(
      compressed_pose.translation_x, compressed_pose.translation_y,
      compressed_pose.translation_z, -kMaxTranslation, kMaxTranslation,
      kCompressionBitCount);

  auto rotation_vec{math::DecompressVec3Rl(
      compressed_pose.rotation_x, compressed_pose.rotation_y,
      compressed_pose.rotation_z, -1.0f, 1.0f, kCompressionBitCount)};
  auto rotation_vec_mag{math::GetSquaredMagnitude(rotation_vec)};
  auto w{math::Sqrt(math::Max(1.0f - rotation_vec_mag, 0.0f))};
  math::Vec4 rotation{rotation_vec, w};
  pose.rotation = math::Quat{rotation.x, rotation.y, rotation.z, rotation.w};
  math::Normalize(pose.rotation);

  pose.scale = DecompressF32Rl(compressed_pose.scale, kCompressionBitCount);
  return pose;
}

SkeletonPose DecompressClipAndExtractPose(const CompressedAnimationClip& clip,
                                          f64 time) {
  const auto ticks_per_frame{1.0 / clip.frames_per_second};
  const auto duration{clip.frame_count * ticks_per_frame};

  // Wrap or clamp time
  if (clip.is_loop) {
    time = math::Fmod(time, duration);
  } else {
    time = math::Min(time, duration);
  }

  const auto frame{math::Min(static_cast<FrameIndex>(time / ticks_per_frame),
                             clip.frame_count - 1)};
  const auto next_frame{clip.is_loop
                            ? (frame + 1) % clip.frame_count
                            : math::Min(frame + 1, clip.frame_count - 1)};
  const auto alpha{
      static_cast<f32>((time - (frame * ticks_per_frame)) / ticks_per_frame)};

  const auto& a_sample{clip.samples[frame]};
  const auto& b_sample{clip.samples[next_frame]};

  auto joint_pose_count{math::Min(a_sample.joint_poses.GetSize(),
                                  b_sample.joint_poses.GetSize())};
  SkeletonPose out_pose{};
  out_pose.local_pose =
      COMET_FRAME_ARRAY(animation::JointPose, joint_pose_count);

  for (usize i{0}; i < joint_pose_count; ++i) {
    auto pose_a{DecompressJointPose(a_sample.joint_poses[i])};
    auto pose_b{DecompressJointPose(b_sample.joint_poses[i])};
    auto& joint{out_pose.local_pose->EmplaceBack()};

    joint.translation =
        math::Lerp(pose_a.translation, pose_b.translation, alpha);
    joint.scale = math::Lerp(pose_a.scale, pose_b.scale, alpha);
    joint.rotation = math::Slerp(pose_a.rotation, pose_b.rotation, alpha);
  }

  return out_pose;
}

void PopulateGlobalPose(const geometry::Skeleton& skeleton,
                        animation::SkeletonPose& pose) {
  auto joint_count{skeleton.joint_count};

  for (usize i{0}; i < joint_count; ++i) {
    const auto& local_pose{pose.local_pose->Get(i)};
    const auto& joint{skeleton.joints[i]};

    auto local_matrix{math::ToTranslateMatrix(local_pose.translation) *
                      math::ToRotationMatrix(local_pose.rotation) *
                      math::ToScaleMatrix(local_pose.scale)};

    if (joint.parent_index == geometry::kInvalidJointIndex) {
      pose.global_pose[i] = local_matrix;
    } else {
      pose.global_pose[i] = pose.global_pose[joint.parent_index] * local_matrix;
    }
  }
}
}  // namespace animation
}  // namespace comet
