// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_runtime_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/runtime/animation/utils/animation_clip_utils.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/compression.h"
#include "comet/runtime/frame/frame_container.h"
#include "comet/core/math/math_compression.h"
#include "comet/core/math/math_interpolation.h"
#include "comet/core/math/math_scalar.h"
#include "comet/core/math/quaternion.h"
#include "comet/core/math/vector.h"
#include "comet/runtime/profiler/profiler.h"

namespace comet {
namespace animation {
void PopulateSkinningBinding(entity::EntityId entity_id, u32 joint_count,
                             u32 matrix_offset, SkinningBinding& binding) {
  binding.entity_id = entity_id;
  binding.joint_count = joint_count;
  binding.matrix_offset = matrix_offset;
}

void PopulateMatrixPalette(const geometry::Skeleton* skeleton,
                           const Array<math::Mat4>* global_pose,
                           MatrixPalette& matrix_palette) {
  COMET_PROFILE("PopulateMatrixPalette");

  const auto joint_count{skeleton->joints.GetSize()};

  COMET_ASSERT(joint_count == global_pose->GetSize(),
               "animation_clip_utils::PopulateMatrixPalette",
               "joint count mismatch", "skeleton", skeleton->id, "joint_count",
               joint_count, "global_pose_joint_count", global_pose->GetSize());

  matrix_palette.skinning_matrix_count = joint_count;
  matrix_palette.skinning_matrices = COMET_DOUBLE_FRAME_ALLOC_MANY(
      math::Mat4, matrix_palette.skinning_matrix_count);

  auto* skinning_matrices{matrix_palette.skinning_matrices};

  for (usize i{0}; i < matrix_palette.skinning_matrix_count; ++i) {
    const auto& joint{skeleton->joints[i]};
    skinning_matrices[i] = global_pose->Get(i) * joint.bind_pose_inv;
  }
}
}  // namespace animation
}  // namespace comet