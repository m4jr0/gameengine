// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "physics_manager.h"

#include "comet/entity/entity_id.h"
#include "comet/math/geometry.h"
#include "comet/math/math_commons.h"
#include "comet/math/vector.h"

namespace comet {
namespace physics {
PhysicsManager::PhysicsManager(const PhysicsManagerDescr& descr)
    : Manager{descr},
      entity_manager_{descr.entity_manager},
      event_manager_{descr.event_manager},
      time_manager_{descr.time_manager} {
  COMET_ASSERT(entity_manager_ != nullptr, "Entity manager is null!");
  COMET_ASSERT(event_manager_ != nullptr, "Event manager is null!");
  COMET_ASSERT(time_manager_ != nullptr, "Time manager is null!");
}

void PhysicsManager::Initialize() {
  Manager::Initialize();
  fixed_delta_time_ = time_manager_->GetFixedDeltaTime();
  max_frame_rate_ = static_cast<u32>((1 / fixed_delta_time_) * 1000);
};

void PhysicsManager::Shutdown() {
  frame_rate_ = 0;
  counter_ = 0;
  max_frame_rate_ = 60;
  current_time_ = 0;
  fixed_delta_time_ = 16.66;
  Manager::Shutdown();
};

void PhysicsManager::Update(f64& lag) {
  if (current_time_ > 1000) {
    frame_rate_ = counter_;
    current_time_ = 0;
    counter_ = 0;
  }

  current_time_ += time_manager_->GetDeltaTime();

  while (lag > fixed_delta_time_) {
    // TODO(m4jr0): Investigate. This seems to prevent round errors.
    if (counter_ == max_frame_rate_) {
      lag = 0;
      return;
    }

    event_manager_->FireAllEvents();

    // TODO(m4jr0): Remove temporary code.
    ApplyTmpCode();

    // TODO(m4jr0): Use a transform system.
    UpdateEntityTransforms();

    ++counter_;
    lag -= fixed_delta_time_;
  }
}

void PhysicsManager::SetLocal(TransformComponent* cmp,
                              const math::Mat4& local) const {
  cmp->local = local;
  cmp->is_dirty = true;

  entity_manager_->GetComponent<TransformRootComponent>(cmp->root_entity_id)
      ->is_child_dirty = true;
};

void PhysicsManager::UpdateTree(
    entity::EntityId parent_entity_id,
    const TransformComponent* parent_transform_cmp) const {
  entity_manager_->EachChild<TransformComponent>(
      [&](auto entity_id) {
        auto* transform_cmp{
            entity_manager_->GetComponent<TransformComponent>(entity_id)};

        if (transform_cmp->is_dirty) {
          transform_cmp->global =
              transform_cmp->local * parent_transform_cmp->global;
        }

        UpdateTree(entity_id, transform_cmp);
      },
      parent_entity_id);
}

u32 PhysicsManager::GetFrameRate() const noexcept { return frame_rate_; }

f32 PhysicsManager::GetFrameTime() const noexcept {
  return (1 / static_cast<f32>(frame_rate_)) * 1000;
}

void PhysicsManager::UpdateEntityTransforms() {
  entity_manager_->Each<TransformRootComponent, TransformComponent>(
      [&](auto entity_id) {
        auto* root_cmp{
            entity_manager_->GetComponent<TransformRootComponent>(entity_id)};

        if (!root_cmp->is_child_dirty) {
          return;
        }

        auto* transform_cmp{
            entity_manager_->GetComponent<TransformComponent>(entity_id)};

        if (transform_cmp->is_dirty) {
          transform_cmp->global = transform_cmp->local;
        }

        UpdateTree(entity_id, transform_cmp);
        root_cmp->is_child_dirty = false;
      });
}

void PhysicsManager::ApplyTmpCode() {
  constexpr auto rot_speed{math::ConvertToRadians(0.5f)};

  entity_manager_->Each<TransformRootComponent, TransformComponent>(
      [&](auto entity_id) {
        auto* transform_cmp{
            entity_manager_->GetComponent<TransformComponent>(entity_id)};
        SetLocal(transform_cmp, math::Rotate(transform_cmp->local, rot_speed,
                                             math::Vec3{0.0f, 1.0f, 0.0f}));
      });
}
}  // namespace physics
}  // namespace comet
