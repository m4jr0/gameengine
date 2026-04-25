// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "physics_manager.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/concurrency/job/job_utils.h"
#include "comet/core/concurrency/job/scheduler.h"
#include "comet/entity/entity_manager.h"
#include "comet/entity/type/entity_id.h"
#include "comet/profiler/profiler.h"
#include "comet/time/time_manager.h"

namespace comet {
namespace physics {
PhysicsManager& PhysicsManager::Get() {
  static PhysicsManager singleton{};
  return singleton;
}

void PhysicsManager::Update(frame::FramePacket* packet) {
  COMET_PROFILE("PhysicsManager::Update");
  COMET_ASSERT(packet != nullptr, "PhysicsManager::Update",
               "frame packet is null");

  current_frame_packet_ = packet;
  auto& time_manager{time::TimeManager::Get()};
  const auto delta_time{time_manager.GetDeltaTime()};
  const auto fixed_delta_time{time_manager.GetFixedDeltaTime()};

  lag_ += delta_time;

  constexpr u32 kMaxSteps{5};
  u32 step_count{0};

  while (lag_ >= fixed_delta_time && step_count < kMaxSteps) {
    UpdateEntityTransforms(packet);
    lag_ -= fixed_delta_time;
    current_time_ += fixed_delta_time;
    ++step_count;
  }

  counter_ += step_count;
  packet->time = current_time_;
  packet->lag = lag_;

  const auto real_now{time_manager.GetRealTime()};

  if (real_now - last_current_time_ >= 1.0) {
    frame_rate_ = counter_;
    counter_ = 0;
    last_current_time_ = real_now;
  }
}

TransformRootComponent PhysicsManager::GenerateTransformRootComponent() const {
  TransformRootComponent transform_root_cmp{};
  transform_root_cmp.is_child_dirty = false;
  return transform_root_cmp;
}

TransformComponent PhysicsManager::GenerateTransformComponent(
    entity::EntityId root_entity_id, entity::EntityId parent_entity_id,
    math::Mat4 local, math::Mat4 global) const {
  TransformComponent transform_cmp{};
  transform_cmp.is_dirty = false;
  transform_cmp.root_entity_id = root_entity_id;
  transform_cmp.parent_entity_id = parent_entity_id;
  transform_cmp.local = local;
  transform_cmp.global = global;
  return transform_cmp;
}

void PhysicsManager::DestroyTransformRootComponent(
    TransformRootComponent* transform_root_cmp) const {
  transform_root_cmp->is_child_dirty = false;
}

void PhysicsManager::DestroyTransformComponent(
    TransformComponent* transform_cmp) const {
  transform_cmp->is_dirty = false;
  transform_cmp->root_entity_id = entity::kInvalidEntityId;
  transform_cmp->parent_entity_id = entity::kInvalidEntityId;
  transform_cmp->local = math::Mat4{1.0f};
  transform_cmp->global = math::Mat4{1.0f};
}

u32 PhysicsManager::GetFrameRate() const noexcept { return frame_rate_; }

f64 PhysicsManager::GetFrameTime() const noexcept {
  return frame_rate_ == 0 ? .0 : (1.0 / static_cast<f64>(frame_rate_));
}

void PhysicsManager::OnShutdown() {
  frame_rate_ = 0;
  current_time_ = 0;
  lag_ = 0;
  current_frame_packet_ = nullptr;
};

void PhysicsManager::UpdateEntityTransforms(frame::FramePacket* packet) {
  COMET_PROFILE("PhysicsManager::UpdateEntityTransforms");
  auto& entity_manager{entity::EntityManager::Get()};

  struct DirtyRootEntry {
    entity::EntityId entity_id{entity::kInvalidEntityId};
    TransformRootComponent* root_cmp{nullptr};
    TransformComponent* transform_cmp{nullptr};
  };

  frame::FrameArray<DirtyRootEntry> dirty_roots{};
  constexpr usize kInitialHierarchyStackCapacity{16};
  dirty_roots.Reserve(kInitialHierarchyStackCapacity);

  entity_manager.ForEach<TransformRootComponent, TransformComponent>(
      [&](entity::EntityId entity_id, TransformRootComponent& root_cmp,
          TransformComponent& transform_cmp) {
        if (!root_cmp.is_child_dirty) {
          return;
        }

        dirty_roots.PushLast(
            DirtyRootEntry{entity_id, &root_cmp, &transform_cmp});
      });

  job::CounterGuard guard{};
  auto& scheduler{job::Scheduler::Get()};

  for (const auto& dirty_root : dirty_roots) {
    struct JobParams {
      PhysicsManager* physics_manager{nullptr};
      frame::FramePacket* packet{nullptr};
      DirtyRootEntry dirty_root{};
    };

    auto* params{COMET_FRAME_ALLOC_ONE_AND_POPULATE(JobParams, this, packet,
                                                    dirty_root)};

    scheduler.Kick(job::GenerateJobDescr(
        job::JobPriority::High,
        [](job::JobParamsHandle params_handle) {
          auto* params{reinterpret_cast<JobParams*>(params_handle)};
          auto& dirty_root{params->dirty_root};

          if (dirty_root.root_cmp == nullptr ||
              dirty_root.transform_cmp == nullptr ||
              !dirty_root.root_cmp->is_child_dirty) {
            return;
          }

          if (dirty_root.transform_cmp->is_dirty) {
            dirty_root.transform_cmp->global = dirty_root.transform_cmp->local;
            params->packet->RegisterDirtyTransform(dirty_root.entity_id,
                                                   dirty_root.transform_cmp);
          }

          params->physics_manager->UpdateTree(
              params->packet, dirty_root.entity_id, dirty_root.transform_cmp);

          dirty_root.root_cmp->is_child_dirty = false;
        },
        params, job::JobStackSize::Normal, guard.GetCounter(),
        "update_transform_tree"));
  }

  guard.Wait();
}

void PhysicsManager::UpdateTree(
    frame::FramePacket* packet, entity::EntityId root_entity_id,
    const TransformComponent* root_transform_cmp) const {
  auto& entity_manager{entity::EntityManager::Get()};

  struct StackEntry {
    entity::EntityId parent_entity_id{entity::kInvalidEntityId};
    const TransformComponent* parent_transform_cmp{nullptr};
  };

  frame::FrameArray<StackEntry> stack{};
  constexpr usize kInitialHierarchyStackCapacity{8};
  stack.Reserve(kInitialHierarchyStackCapacity);
  stack.PushLast(StackEntry{root_entity_id, root_transform_cmp});

  while (!stack.IsEmpty()) {
    const auto entry{stack.TakeLast()};

    entity_manager.ForEachChild<TransformComponent>(
        [&](entity::EntityId child_entity_id,
            TransformComponent& transform_cmp) {
          transform_cmp.global =
              transform_cmp.local * entry.parent_transform_cmp->global;

          packet->RegisterDirtyTransform(child_entity_id, &transform_cmp);
          stack.PushLast(StackEntry{child_entity_id, &transform_cmp});
        },
        entry.parent_entity_id);
  }
}
}  // namespace physics
}  // namespace comet
