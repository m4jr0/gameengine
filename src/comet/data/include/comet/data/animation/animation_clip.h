// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_DATA_ANIMATION_ANIMATION_CLIP_H_
#define COMET_DATA_ANIMATION_ANIMATION_CLIP_H_

#include "comet/core/container/array.h"
#include "comet/core/essentials.h"
#include "comet/core/math/quaternion.h"
#include "comet/core/math/vector.h"
#include "comet/data/animation/animation_id.h"

namespace comet {
namespace animation {
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
  AnimationClipId id{};
  FrameIndex frames_per_second{0};
  FrameIndex frame_count{0};
  Array<AnimationSample> samples{};
  bool is_loop{false};
};

struct CompressedAnimationClip {
  AnimationClipId id{};
  FrameIndex frames_per_second{0};
  FrameIndex frame_count{0};
  Array<CompressedAnimationSample> samples{};
  bool is_loop{false};
};

inline constexpr f32 kMaxTranslation{500.0f};
inline constexpr u8 kCompressionBitCount{16};
}  // namespace animation
}  // namespace comet

#endif  // COMET_DATA_ANIMATION_ANIMATION_CLIP_H_