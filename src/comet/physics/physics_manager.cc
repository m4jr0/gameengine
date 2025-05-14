// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "comet_pch.h"

#include "physics_manager.h"

#include "comet/entity/entity_id.h"
#include "comet/entity/entity_manager.h"
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
  max_frame_rate_ = static_cast<u32>((1.0 / fixed_delta_time_));
};

void PhysicsManager::Shutdown() {
  frame_rate_ = 0;
  max_frame_rate_ = 60;
  current_time_ = 0;
  fixed_delta_time_ = .01666f;
  lag_ = 0;
  current_frame_packet_ = nullptr;
  Manager::Shutdown();
};

void PhysicsManager::Update(frame::FramePacket* packet) {
  current_frame_packet_ = packet;

  const auto delta_time{time::TimeManager::Get().GetDeltaTime()};
  lag_ += delta_time;

  const int max_steps = 5;
  int step_count = 0;

  while (lag_ >= fixed_delta_time_ && step_count < max_steps) {
    UpdateEntityTransforms(packet);
    lag_ -= fixed_delta_time_;
    current_time_ += fixed_delta_time_;
    ++step_count;
  }

  counter_ += step_count;
  packet->time = current_time_;
  packet->lag = lag_;

  auto now{time::TimeManager::Get().GetCurrentTime()};

  if (now - last_current_time_ >= 1.0f) {
    frame_rate_ = counter_;
    counter_ = 0;
    last_current_time_ = now;
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

f64 PhysicsManager::GetFrameTime() const noexcept {
  return frame_rate_ == 0 ? 0.0 : (1.0 / static_cast<f64>(frame_rate_));
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
