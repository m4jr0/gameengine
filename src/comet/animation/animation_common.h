// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_ANIMATION_ANIMATION_COMMON_H_
#define COMET_COMET_ANIMATION_ANIMATION_COMMON_H_

#include "comet/core/essentials.h"
#include "comet/core/type/array.h"
#include "comet/geometry/geometry_common.h"

namespace comet {
namespace animation {
struct AnimationSample {
  Array<geometry::JointPose*> joint_poses{};
};

using FrameIndex = u32;

// >:3 Support compressed animation clips.
struct AnimationClip {
  geometry::Skeleton* skeleton{nullptr};
  FrameIndex frames_per_second{0};
  FrameIndex frame_count{0};
  Array<AnimationSample> resource_samples{};
  bool is_loop{false};
};

struct CompressedJointTrack {
  u16* rotation_keys{nullptr};
  u16* translation_keys{nullptr};
  u16* scale_keys{nullptr};
  FrameIndex* frame_indices{nullptr};
  u16 key_count{0};
};

struct CompressedAnimationClip {
  geometry::Skeleton* skeleton{nullptr};
  CompressedJointTrack* joint_tracks{nullptr};
  FrameIndex frames_per_second{0};
  FrameIndex frame_count{0};
  u16 joint_count{0};
  bool is_loop{false};
};

// Should it be in the settings?
constexpr auto kMaxTranslation{2.0f};
constexpr auto kCompressionBitCount{16};

void ExtractPose(const CompressedAnimationClip& clip, f32 time,
                 Array<geometry::JointPose> pose);
}  // namespace animation
}  // namespace comet

#endif  // COMET_COMET_ANIMATION_ANIMATION_COMMON_H_
