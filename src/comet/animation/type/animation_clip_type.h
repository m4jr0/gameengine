// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_ANIMATION_TYPE_ANIMATION_CLIP_TYPE_H_
#define COMET_COMET_ANIMATION_TYPE_ANIMATION_CLIP_TYPE_H_

#include "comet/animation/animation_id.h"
#include "comet/core/essentials.h"
#include "comet/core/type/array.h"
#include "comet/math/quaternion.h"
#include "comet/math/vector.h"

namespace comet {
namespace animation {
using FrameIndex = u32;

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
#endif  // COMET_COMPRESS_ANIMATIONS
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

constexpr f32 kMaxTranslation{500.0f};
constexpr u8 kCompressionBitCount{16};
}  // namespace animation
}  // namespace comet

#endif  // COMET_COMET_ANIMATION_TYPE_ANIMATION_CLIP_TYPE_H_