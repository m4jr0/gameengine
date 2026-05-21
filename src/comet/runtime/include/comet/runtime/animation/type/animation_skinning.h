// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RUNTIME_ANIMATION_TYPE_ANIMATION_SKINNING_H_
#define COMET_RUNTIME_ANIMATION_TYPE_ANIMATION_SKINNING_H_

#include "comet/core/essentials.h"
#include "comet/entity/type/entity_id.h"
#include "comet/math/matrix.h"

namespace comet {
namespace animation {
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

#endif  // COMET_RUNTIME_ANIMATION_TYPE_ANIMATION_SKINNING_H_