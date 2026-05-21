// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/data/animation/utils/animation_clip_utils.h"
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

  const auto rotation_vec{math::DecompressVec3Rl(
      compressed_pose.rotation_x, compressed_pose.rotation_y,
      compressed_pose.rotation_z, -1.0f, 1.0f, kCompressionBitCount)};
  const auto rotation_vec_mag{math::GetSquaredMagnitude(rotation_vec)};
  const auto w{math::Sqrt(math::Max(1.0f - rotation_vec_mag, .0f))};

  pose.rotation = math::Quat{w, rotation_vec.x, rotation_vec.y, rotation_vec.z};
  math::Normalize(pose.rotation);

  pose.scale = DecompressF32Rl(compressed_pose.scale, kCompressionBitCount);
#endif  // !COMET_COMPRESS_ANIMATIONS

  return pose;
}
}  // namespace animation
}  // namespace comet