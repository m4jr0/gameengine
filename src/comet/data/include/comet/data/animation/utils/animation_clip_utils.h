// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_DATA_ANIMATION_UTILS_ANIMATION_CLIP_UTILS_H_
#define COMET_DATA_ANIMATION_UTILS_ANIMATION_CLIP_UTILS_H_

#include "comet/data/animation/animation_clip.h"
#include "comet/runtime/animation/animation_pose.h"
#include "comet/runtime/animation/animation_skinning.h"
#include "comet/core/essentials.h"
#include "comet/core/container/array.h"
#include "comet/runtime/entity/entity_id.h"
#include "comet/data/geometry/skeleton.h"
#include "comet/core/math/matrix.h"

namespace comet {
namespace animation {
CompressedJointPose CompressJointPose(const JointPose& pose);
JointPose DecompressJointPose(const CompressedJointPose& compressed_pose);
}  // namespace animation
}  // namespace comet

#endif  // COMET_DATA_ANIMATION_UTILS_ANIMATION_CLIP_UTILS_H_