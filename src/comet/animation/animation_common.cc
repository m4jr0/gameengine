// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "animation_common.h"

#include "comet/core/compression.h"
#include "comet/math/math_compression.h"
#include "comet/math/quaternion.h"
#include "comet/math/vector.h"
#include "comet/resource/model_resource.h"

namespace comet {
namespace animation {
geometry::JointPose ExtractPose(
    const geometry::CompressedJointPose& compressed_pose) {
  geometry::JointPose pose{};
  pose.translation = math::DecompressVecRl(
      compressed_pose.translation_x, compressed_pose.translation_y,
      compressed_pose.translation_z, -kMaxTranslation, kMaxTranslation,
      kCompressionBitCount);

  auto rotation_vec{math::DecompressVecRl(
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
}  // namespace animation
}  // namespace comet
