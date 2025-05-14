// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_ANIMATION_ANIMATION_COMMON_H_
#define COMET_COMET_ANIMATION_ANIMATION_COMMON_H_

#include "comet/core/essentials.h"
#include "comet/core/type/array.h"
#include "comet/core/type/string_id.h"
#include "comet/geometry/geometry_common.h"

namespace comet {
namespace animation {
using AnimationId = stringid::StringId;
constexpr auto kInvalidAnimationId{stringid::kInvalidStringId};

using FrameIndex = u32;

struct JointPose {
  math::Quat rotation{};
  math::Vec3 translation{};
  f32 scale{1.0f};
};

struct CompressedJointPose {
  u16 rotation_x{0};
  u16 rotation_y{0};
  u16 rotation_z{0};
  u16 translation_x{0};
  u16 translation_y{0};
  u16 translation_z{0};
  u16 scale{0};
};

struct AnimationSample {
  Array<JointPose> joint_poses{};
};

struct CompressedAnimationSample {
  Array<CompressedJointPose> joint_poses{};
};

struct AnimationClip {
  AnimationId animation_id{kInvalidAnimationId};
  FrameIndex frames_per_second{0};
  FrameIndex frame_count{0};
  Array<AnimationSample> samples{};
  bool is_loop{false};
};

struct CompressedAnimationClip {
  AnimationId animation_id{kInvalidAnimationId};
  FrameIndex frames_per_second{0};
  FrameIndex frame_count{0};
  Array<CompressedAnimationSample> samples{};
  bool is_loop{false};
};

struct SkeletonPose {
  geometry::Skeleton* skeleton{nullptr};
  Array<animation::JointPose>* local_pose{nullptr};
  math::Mat4* global_pose{nullptr};
};  // >:3

// Should it be in the settings?
constexpr f32 kMaxTranslation{2.0f};
constexpr u8 kCompressionBitCount{16};

CompressedJointPose CompressJointPose(const JointPose& pose);
JointPose DecompressJointPose(const CompressedJointPose& compressed_pose);
SkeletonPose DecompressClipAndExtractPose(const CompressedAnimationClip& clip,
                                          f64 time);
void PopulateGlobalPose(const geometry::Skeleton& skeleton,
                        animation::SkeletonPose& pose);
}  // namespace animation
}  // namespace comet

#endif  // COMET_COMET_ANIMATION_ANIMATION_COMMON_H_
