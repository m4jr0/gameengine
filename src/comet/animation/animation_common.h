// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_ANIMATION_ANIMATION_COMMON_H_
#define COMET_COMET_ANIMATION_ANIMATION_COMMON_H_

#include "comet/core/essentials.h"
#include "comet/core/type/array.h"
#include "comet/entity/entity_id.h"
#include "comet/geometry/geometry_common.h"
#include "comet/math/matrix.h"
#include "comet/math/quaternion.h"
#include "comet/resource/resource.h"

namespace comet {
namespace animation {
using AnimationClipId = resource::ResourceId;
constexpr auto kInvalidAnimationClipId{resource::kInvalidResourceId};

using FrameIndex = u32;

enum AnimationOverrideFlagBits {
  kAnimationOverrideFlagBitsNone = 0x0,
  kAnimationOverrideFlagBitsIsLoop = 0x1,
};

using AnimationOverrideFlags = u8;

struct JointPose {
  math::Quat rotation{};
  math::Vec3 translation{};
  f32 scale{1.0f};
};

struct CompressedJointPose {
#ifndef COMET_COMPRESS_ANIMATIONS
  math::Quat rotation{};
  math::Vec3 translation{};
  f32 scale{1.0f};
#else
  u16 rotation_x{0};
  u16 rotation_y{0};
  u16 rotation_z{0};
  u16 translation_x{0};
  u16 translation_y{0};
  u16 translation_z{0};
  u16 scale{0};
#endif  // !COMET_COMPRESS_ANIMATIONS
};

struct AnimationSample {
  Array<JointPose> joint_poses{};
};

struct CompressedAnimationSample {
  Array<CompressedJointPose> joint_poses{};
};

struct AnimationClip {
  AnimationClipId id{kInvalidAnimationClipId};
  FrameIndex frames_per_second{0};
  FrameIndex frame_count{0};
  Array<AnimationSample> samples{};
  bool is_loop{false};
};

struct CompressedAnimationClip {
  AnimationClipId id{kInvalidAnimationClipId};
  FrameIndex frames_per_second{0};
  FrameIndex frame_count{0};
  Array<CompressedAnimationSample> samples{};
  bool is_loop{false};
};

struct AnimationPose {
  Array<animation::JointPose>* local_pose{nullptr};
  Array<math::Mat4>* global_pose{nullptr};
};

constexpr f32 kMaxTranslation{500.0f};
constexpr u8 kCompressionBitCount{16};

struct SkinningBinding {
  entity::EntityId entity_id{entity::kInvalidEntityId};
  u32 matrix_offset{0};
  u32 joint_count{0};
};

struct MatrixPalette {
  usize skinning_matrix_count{0};
  math::Mat4* skinning_matrices{nullptr};
};

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
                        animation::AnimationPose& pose);
void PopulateSkinningBinding(entity::EntityId entity_id, u32 joint_count,
                             u32 matrix_offset, SkinningBinding& binding);
void PopulateMatrixPalette(const geometry::Skeleton* skeleton,
                           const Array<math::Mat4>* global_pose,
                           MatrixPalette& matrix_palette);
}  // namespace animation
}  // namespace comet

#endif  // COMET_COMET_ANIMATION_ANIMATION_COMMON_H_
