// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_ANIMATION_ANIMATION_TYPE_H_
#define COMET_COMET_ANIMATION_ANIMATION_TYPE_H_

#include "comet/animation/animation_clip.h"
#include "comet/core/essentials.h"
#include "comet/core/type/array.h"
#include "comet/entity/entity_id.h"
#include "comet/math/matrix.h"

namespace comet {
namespace animation {
using AnimationOverrideFlags = u8;

enum AnimationOverrideFlagBits : AnimationOverrideFlags {
  kAnimationOverrideFlagBitsNone = 0x0,
  kAnimationOverrideFlagBitsIsLoop = 0x1,
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
}  // namespace animation
}  // namespace comet

#endif  // COMET_COMET_ANIMATION_ANIMATION_TYPE_H_
