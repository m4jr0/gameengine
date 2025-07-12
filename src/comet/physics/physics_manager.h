// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_PHYSICS_PHYSICS_MANAGER_H_
#define COMET_COMET_PHYSICS_PHYSICS_MANAGER_H_

#include "comet/core/essentials.h"
#include "comet/core/frame/frame_packet.h"
#include "comet/core/manager.h"
#include "comet/entity/entity_id.h"
#include "comet/math/matrix.h"
#include "comet/physics/component/transform_component.h"

namespace comet {
namespace physics {
class PhysicsManager : public Manager {
 public:
  static PhysicsManager& Get();

  PhysicsManager() = default;
  PhysicsManager(const PhysicsManager&) = delete;
  PhysicsManager(PhysicsManager&&) = delete;
  PhysicsManager& operator=(const PhysicsManager&) = delete;
  PhysicsManager& operator=(PhysicsManager&&) = delete;
  virtual ~PhysicsManager() = default;

  void Initialize() override;
  void Shutdown() override;
  void Update(frame::FramePacket* packet);
  void UpdateTree(frame::FramePacket* packet, entity::EntityId parent_entity_id,
                  const TransformComponent* parent_transform_cmp) const;

  TransformRootComponent GenerateTransformRootComponent() const;
  TransformComponent GenerateTransformComponent(
      entity::EntityId root_entity_id,
      entity::EntityId parent_entity_id = entity::kInvalidEntityId,
      math::Mat4 local = math::Mat4{1.0f},
      math::Mat4 global = math::Mat4{1.0f}) const;

  void DestroyTransformRootComponent(
      TransformRootComponent* transform_root_cmp) const;
  void DestroyTransformComponent(TransformComponent* transform_cmp) const;

  u32 GetFrameRate() const noexcept;
  f64 GetFrameTime() const noexcept;

 private:
  void UpdateEntityTransforms(frame::FramePacket* packet);

  u32 counter_{0};
  u32 frame_rate_{0};
  u32 max_frame_rate_{60};  // 60 Hz refresh by default.
  f64 current_time_{0};
  f64 last_current_time_{0};
  f64 fixed_delta_time_{.01666f};  // 60 Hz refresh by default.
  f64 lag_{.0};
  frame::FramePacket* current_frame_packet_{nullptr};
};
}  // namespace physics
}  // namespace comet

#endif  // COMET_COMET_PHYSICS_PHYSICS_MANAGER_H_
