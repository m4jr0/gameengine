// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "comet_pch.h"

#include "physics_manager.h"

#include "comet/entity/entity_id.h"
#include "comet/entity/entity_manager.h"
#include "comet/event/event_manager.h"
#include "comet/math/geometry.h"
#include "comet/math/math_commons.h"
#include "comet/math/vector.h"
#include "comet/time/time_manager.h"

namespace comet {
namespace physics {
PhysicsManager& PhysicsManager::Get() {
  static PhysicsManager singleton{};
  return singleton;
}

void PhysicsManager::Initialize() {
  Manager::Initialize();
  fixed_delta_time_ = time::TimeManager::Get().GetFixedDeltaTime();
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

void PhysicsManager::Update(frame::FramePacket* packet) {
  current_frame_packet_ = packet;

  if (current_time_ > 1000) {
    frame_rate_ = counter_;
    current_time_ = 0;
    counter_ = 0;
  }

  current_time_ += time::TimeManager::Get().GetDeltaTime();

  while (packet->lag > fixed_delta_time_) {
    // TODO(m4jr0): Investigate. This seems to prevent round errors.
    if (counter_ == max_frame_rate_) {
      packet->lag = 0;
      return;
    }

    // TODO(m4jr0): Use a transform system.
    UpdateEntityTransforms(packet);

    ++counter_;
    packet->lag -= fixed_delta_time_;
  }
}

void PhysicsManager::UpdateTree(
    frame::FramePacket* packet, entity::EntityId parent_entity_id,
    const TransformComponent* parent_transform_cmp) const {
  auto& entity_manager{entity::EntityManager::Get()};

  entity_manager.EachChild<TransformComponent>(
      [&](auto entity_id) {
        auto* transform_cmp{
            entity_manager.GetComponent<TransformComponent>(entity_id)};

        transform_cmp->global =
            transform_cmp->local * parent_transform_cmp->global;
        packet->RegisterDirtyTransform(entity_id, transform_cmp);
        UpdateTree(packet, entity_id, transform_cmp);
      },
      parent_entity_id);
}

u32 PhysicsManager::GetFrameRate() const noexcept { return frame_rate_; }

f32 PhysicsManager::GetFrameTime() const noexcept {
  return (1 / static_cast<f32>(frame_rate_)) * 1000;
}

void PhysicsManager::UpdateEntityTransforms(frame::FramePacket* packet) {
  auto& entity_manager{entity::EntityManager::Get()};

  entity_manager.Each<TransformRootComponent, TransformComponent>(
      [&](auto entity_id) {
        auto* root_cmp{
            entity_manager.GetComponent<TransformRootComponent>(entity_id)};

        if (!root_cmp->is_child_dirty) {
          return;
        }

        auto* transform_cmp{
            entity_manager.GetComponent<TransformComponent>(entity_id)};

        if (transform_cmp->is_dirty) {
          transform_cmp->global = transform_cmp->local;
        }

        UpdateTree(packet, entity_id, transform_cmp);
        root_cmp->is_child_dirty = false;
      });
}
}  // namespace physics
}  // namespace comet
