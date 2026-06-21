// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_data_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/data/animation/utils/animation_clip_utils.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/math/math_compression.h"
#include "comet/core/math/math_scalar.h"
#include "comet/core/math/quaternion.h"

namespace comet {
namespace animation {
CompressedJointPose CompressJointPose(const JointPose& pose) {
  CompressedJointPose compressed_pose{};

  u32 translation_x{0};
  u32 translation_y{0};
  u32 translation_z{0};

  math::CompressVec3Rl(pose.translation, -kMaxTranslation, kMaxTranslation,
                       kCompressionBitCount, translation_x, translation_y,
                       translation_z);

  compressed_pose.translation_x = static_cast<u16>(translation_x);
  compressed_pose.translation_y = static_cast<u16>(translation_y);
  compressed_pose.translation_z = static_cast<u16>(translation_z);

  compressed_pose.rotation_x = static_cast<u16>(
      math::CompressF32Rl(pose.rotation.x, -1.0f, 1.0f, kCompressionBitCount));
  compressed_pose.rotation_y = static_cast<u16>(
      math::CompressF32Rl(pose.rotation.y, -1.0f, 1.0f, kCompressionBitCount));
  compressed_pose.rotation_z = static_cast<u16>(
      math::CompressF32Rl(pose.rotation.z, -1.0f, 1.0f, kCompressionBitCount));

  compressed_pose.scale =
      static_cast<u16>(math::CompressF32Rl(pose.scale, kCompressionBitCount));

  return compressed_pose;
}

JointPose DecompressJointPose(const CompressedJointPose& compressed_pose) {
  JointPose pose{};

  pose.translation = math::DecompressVec3Rl(
      compressed_pose.translation_x, compressed_pose.translation_y,
      compressed_pose.translation_z, -kMaxTranslation, kMaxTranslation,
      kCompressionBitCount);

  const auto rotation_vec{math::DecompressVec3Rl(
      compressed_pose.rotation_x, compressed_pose.rotation_y,
      compressed_pose.rotation_z, -1.0f, 1.0f, kCompressionBitCount)};

  const auto rotation_vec_magnitude{math::GetSquaredMagnitude(rotation_vec)};
  const auto rotation_w{
      math::Sqrt(math::Max(1.0f - rotation_vec_magnitude, .0f))};

  pose.rotation =
      math::Quat{rotation_w, rotation_vec.x, rotation_vec.y, rotation_vec.z};
  math::Normalize(pose.rotation);

  pose.scale =
      math::DecompressF32Rl(compressed_pose.scale, kCompressionBitCount);

  return pose;
}
}  // namespace animation
}  // namespace comet