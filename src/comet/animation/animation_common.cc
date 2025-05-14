// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "comet_pch.h"

#include "animation_common.h"

#include "comet/core/compression.h"
#include "comet/core/frame/frame_utils.h"
#include "comet/math/math_common.h"
#include "comet/math/math_compression.h"
#include "comet/math/math_interpolation.h"
#include "comet/math/quaternion.h"
#include "comet/math/vector.h"

namespace comet {
namespace animation {
CompressedJointPose CompressJointPose(const JointPose& pose) {
  CompressedJointPose compressed_pose{};

#ifndef COMET_COMPRESS_ANIMATIONS
  compressed_pose.translation = pose.translation;
  compressed_pose.rotation = pose.rotation;
  compressed_pose.scale = pose.scale;
#else
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
#endif  // !COMET_COMPRESS_ANIMATIONS
  return compressed_pose;
}

JointPose DecompressJointPose(const CompressedJointPose& compressed_pose) {
  JointPose pose{};
#ifndef COMET_COMPRESS_ANIMATIONS
  pose.translation = compressed_pose.translation;
  pose.rotation = compressed_pose.rotation;
  pose.scale = compressed_pose.scale;
#else
  pose.translation = math::DecompressVec3Rl(
      compressed_pose.translation_x, compressed_pose.translation_y,
      compressed_pose.translation_z, -kMaxTranslation, kMaxTranslation,
      kCompressionBitCount);

  auto rotation_vec{math::DecompressVec3Rl(
      compressed_pose.rotation_x, compressed_pose.rotation_y,
      compressed_pose.rotation_z, -1.0f, 1.0f, kCompressionBitCount)};
  auto rotation_vec_mag{math::GetSquaredMagnitude(rotation_vec)};
  auto w{math::Sqrt(math::Max(1.0f - rotation_vec_mag, 0.0f))};
  pose.rotation = math::Quat{w, rotation_vec.x, rotation_vec.y, rotation_vec.z};
  math::Normalize(pose.rotation);

  pose.scale = DecompressF32Rl(compressed_pose.scale, kCompressionBitCount);
#endif  // !COMET_COMPRESS_ANIMATIONS
  return pose;
}

AnimationPose PopulatePoseFromSample(const CompressedAnimationClip& clip,
                                     FrameIndex frame) {
  const auto& sample{clip.samples[frame]};
  auto joint_pose_count{sample.joint_poses.GetSize()};

  AnimationPose out_pose{};
  out_pose.local_pose =
      COMET_DOUBLE_FRAME_ARRAY(animation::JointPose, joint_pose_count);

  for (usize i{0}; i < joint_pose_count; ++i) {
    auto pose{DecompressJointPose(sample.joint_poses[i])};
    out_pose.local_pose->PushBack(pose);
  }

  return out_pose;
}

AnimationPose PopulatePoseFromSamples(const CompressedAnimationClip& clip,
                                      FrameIndex frame_a, FrameIndex frame_b,
                                      f32 alpha) {
  if (alpha <= 0.0f) {
    return PopulatePoseFromSample(clip, frame_a);
  }

  if (alpha >= 1.0f) {
    return PopulatePoseFromSample(clip, frame_b);
  }

  const auto& a_sample{clip.samples[frame_a]};
  const auto& b_sample{clip.samples[frame_b]};
  COMET_ASSERT(a_sample.joint_poses.GetSize() == b_sample.joint_poses.GetSize(),
               "Joint count mismatch on animation clip ",
               COMET_STRING_ID_LABEL(clip.id), " at frames ", frame_a, "/",
               frame_b, ": ", a_sample.joint_poses.GetSize(),
               " != ", b_sample.joint_poses.GetSize(), "!");
  auto joint_pose_count{a_sample.joint_poses.GetSize()};

  AnimationPose out_pose{};
  out_pose.local_pose =
      COMET_DOUBLE_FRAME_ARRAY(animation::JointPose, joint_pose_count);

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

AnimationPose DecompressClipAndExtractPose(const CompressedAnimationClip& clip,
                                           f64 time, f32 speed,
                                           AnimationOverrideFlags overrides,
                                           bool is_loop) {
  const auto ticks_per_frame{1.0 / clip.frames_per_second};
  const auto duration{clip.frame_count * ticks_per_frame};

  auto effective_is_loop{(overrides & kAnimationOverrideFlagBitsIsLoop) != 0
                             ? is_loop
                             : clip.is_loop};

  time *= speed;
  time = effective_is_loop ? math::Fmod(time, duration)
                           : math::Min(time, duration);

  auto frame{math::Min(static_cast<FrameIndex>(time / ticks_per_frame),
                       clip.frame_count - 1)};
  auto next_frame{effective_is_loop
                      ? (frame + 1) % clip.frame_count
                      : math::Min(frame + 1, clip.frame_count - 1)};
  auto alpha{
      static_cast<f32>((time - (frame * ticks_per_frame)) / ticks_per_frame)};

  return PopulatePoseFromSamples(clip, frame, next_frame, alpha);
}

void PopulateGlobalPose(const geometry::Skeleton& skeleton,
                        animation::AnimationPose& pose) {
  auto joint_count{skeleton.joints.GetSize()};
  COMET_ASSERT(joint_count == pose.local_pose->GetSize(),
               "Joint count mismatch with skeleton #", skeleton.id, "!");

  pose.global_pose =
      COMET_DOUBLE_FRAME_ARRAY(math::Mat4, pose.local_pose->GetSize());

  for (usize i{0}; i < joint_count; ++i) {
    const auto& local_pose{pose.local_pose->Get(i)};
    const auto& joint{skeleton.joints[i]};

    auto local_matrix{math::ComposeTransform(
        local_pose.translation, local_pose.rotation, local_pose.scale)};

    auto translation{math::ToTranslateMatrix(local_pose.translation)};
    auto rotation{math::ToRotationMatrix(local_pose.rotation)};
    auto scale{math::ToScaleMatrix(local_pose.scale)};

    if (joint.parent_index == geometry::kInvalidSkeletonJointIndex) {
      pose.global_pose->PushBack(local_matrix);
    } else {
      auto& parent{pose.global_pose->Get(joint.parent_index)};
      pose.global_pose->PushBack(parent * local_matrix);
    }
  }
}

void PopulateSkinningBinding(entity::EntityId entity_id, u32 joint_count,
                             u32 matrix_offset, SkinningBinding& binding) {
  binding.entity_id = entity_id;
  binding.joint_count = joint_count;
  binding.matrix_offset = matrix_offset;
}

void PopulateMatrixPalette(const geometry::Skeleton* skeleton,
                           const Array<math::Mat4>* global_pose,
                           MatrixPalette& matrix_palette) {
  auto joint_count{skeleton->joints.GetSize()};
  COMET_ASSERT(joint_count == global_pose->GetSize(),
               "Joint count mismatch with skeleton #", skeleton->id, "!");
  matrix_palette.skinning_matrix_count = joint_count;
  matrix_palette.skinning_matrices = COMET_DOUBLE_FRAME_ALLOC_MANY(
      math::Mat4, matrix_palette.skinning_matrix_count);
  auto* skinning_matrices{matrix_palette.skinning_matrices};

  for (usize i{0}; i < matrix_palette.skinning_matrix_count; ++i) {
    const auto& joint{skeleton->joints[i]};
    skinning_matrices[i] = global_pose->Get(i) * joint.bind_pose_inv;
  }
}
}  // namespace animation
}  // namespace comet
