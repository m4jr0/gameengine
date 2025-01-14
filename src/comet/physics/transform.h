// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_PHYSICS_TRANSFORM_H_
#define COMET_COMET_PHYSICS_TRANSFORM_H_

#include "comet/core/essentials.h"
#include "comet/math/matrix.h"
#include "comet/math/vector.h"
#include "comet/physics/component/transform_component.h"

namespace comet {
namespace physics {
namespace internal {
void MakeDirty(TransformComponent* cmp);
}  // namespace internal

void SetLocalTransform(TransformComponent* cmp,
                       const comet::math::Mat4& new_local);
void SetGlobalTransform(TransformComponent* cmp,
                        const comet::math::Mat4& new_global);
void TranslateLocal(TransformComponent* cmp,
                    const comet::math::Vec3& translation);
void RotateLocal(TransformComponent* cmp, float rotation_angle,
                 const comet::math::Vec3& rotation_axis);
void ScaleLocal(TransformComponent* cmp, f32 scale_factor);
void ScaleLocal(TransformComponent* cmp,
                const comet::math::Vec3& scale_factors);
void ResetLocalTransform(TransformComponent* cmp);
void SetParentEntity(TransformComponent* cmp, entity::EntityId new_parent);
void SetRootEntity(TransformComponent* cmp, entity::EntityId new_root);
}  // namespace physics
}  // namespace comet

#endif  // COMET_COMET_PHYSICS_TRANSFORM_H_
