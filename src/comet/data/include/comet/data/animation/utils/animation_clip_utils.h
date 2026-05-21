// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_DATA_ANIMATION_UTILS_ANIMATION_CLIP_UTILS_H_
#define COMET_DATA_ANIMATION_UTILS_ANIMATION_CLIP_UTILS_H_

#include "comet/animation/type/animation_clip.h"
#include "comet/animation/type/animation_pose.h"
#include "comet/animation/type/animation_skinning.h"
#include "comet/core/essentials.h"
#include "comet/core/type/array.h"
#include "comet/entity/type/entity_id.h"
#include "comet/geometry/type/skeleton.h"
#include "comet/math/matrix.h"

namespace comet {
namespace animation {
CompressedJointPose CompressJointPose(const JointPose& pose);
JointPose DecompressJointPose(const CompressedJointPose& compressed_pose);
}  // namespace animation
}  // namespace comet

#endif  // COMET_DATA_ANIMATION_UTILS_ANIMATION_CLIP_UTILS_H_