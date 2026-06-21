// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RUNTIME_ANIMATION_UTILS_ANIMATION_SKINNING_UTILS_H_
#define COMET_RUNTIME_ANIMATION_UTILS_ANIMATION_SKINNING_UTILS_H_

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
void PopulateSkinningBinding(entity::EntityId entity_id, u32 joint_count,
                             u32 matrix_offset, SkinningBinding& binding);

void PopulateMatrixPalette(const geometry::Skeleton* skeleton,
                           const Array<math::Mat4>* global_pose,
                           MatrixPalette& matrix_palette);
}  // namespace animation
}  // namespace comet

#endif  // COMET_RUNTIME_ANIMATION_UTILS_ANIMATION_SKINNING_UTILS_H_