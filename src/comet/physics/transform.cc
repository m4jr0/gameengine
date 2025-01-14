// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "comet_pch.h"

#include "transform.h"

#include "comet/entity/entity_manager.h"
#include "comet/math/geometry.h"

namespace comet {
namespace physics {
namespace internal {
void MakeDirty(TransformComponent* cmp) {
  cmp->is_dirty = true;

  entity::EntityManager::Get()
      .GetComponent<TransformRootComponent>(cmp->root_entity_id)
      ->is_child_dirty = true;
}
}  // namespace internal

void SetLocalTransform(TransformComponent* cmp,
                       const comet::math::Mat4& new_local) {
  cmp->local = new_local;
  internal::MakeDirty(cmp);
}

void SetGlobalTransform(TransformComponent* cmp,
                        const comet::math::Mat4& new_global) {
  cmp->global = new_global;
  internal::MakeDirty(cmp);
}

void TranslateLocal(TransformComponent* cmp,
                    const comet::math::Vec3& translation) {
  cmp->local = math::Translate(cmp->local, translation);
  internal::MakeDirty(cmp);
}

void RotateLocal(TransformComponent* cmp, float rotation_angle,
                 const comet::math::Vec3& rotation_axis) {
  cmp->local = math::Rotate(cmp->local, rotation_angle, rotation_axis);
  internal::MakeDirty(cmp);
}

void ScaleLocal(TransformComponent* cmp, f32 scale_factor) {
  cmp->local = math::Scale(cmp->local, scale_factor);
  internal::MakeDirty(cmp);
}

void ScaleLocal(TransformComponent* cmp,
                const comet::math::Vec3& scale_factors) {
  cmp->local = math::Scale(cmp->local, scale_factors);
  internal::MakeDirty(cmp);
}

void ResetLocalTransform(TransformComponent* cmp) {
  cmp->local = comet::math::Mat4(1.0f);
  internal::MakeDirty(cmp);
}

void SetParentEntity(TransformComponent* cmp, entity::EntityId new_parent) {
  cmp->parent_entity_id = new_parent;
  internal::MakeDirty(cmp);
}

void SetRootEntity(TransformComponent* cmp, entity::EntityId new_root) {
  cmp->root_entity_id = new_root;
  internal::MakeDirty(cmp);
}
}  // namespace physics
}  // namespace comet
