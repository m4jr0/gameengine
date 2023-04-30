// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_PHYSICS_PHYSICS_MANAGER_H_
#define COMET_COMET_PHYSICS_PHYSICS_MANAGER_H_

#include "comet_precompile.h"

#include "comet/core/manager.h"
#include "comet/entity/entity_manager.h"
#include "comet/event/event_manager.h"
#include "comet/math/matrix.h"
#include "comet/physics/component/transform_component.h"
#include "comet/time/time_manager.h"

namespace comet {
namespace physics {
struct PhysicsManagerDescr : ManagerDescr {
  entity::EntityManager* entity_manager{nullptr};
  event::EventManager* event_manager{nullptr};
  time::TimeManager* time_manager{nullptr};
};

class PhysicsManager : public Manager {
 public:
  PhysicsManager() = delete;
  explicit PhysicsManager(const PhysicsManagerDescr& descr);
  PhysicsManager(const PhysicsManager&) = delete;
  PhysicsManager(PhysicsManager&&) = delete;
  PhysicsManager& operator=(const PhysicsManager&) = delete;
  PhysicsManager& operator=(PhysicsManager&&) = delete;
  virtual ~PhysicsManager() = default;

  void Initialize() override;
  void Shutdown() override;
  void Update(f64& lag);
  void SetLocal(TransformComponent* cmp, const math::Mat4& local) const;
  void UpdateTree(entity::EntityId parent_entity_id,
                  const TransformComponent* parent_transform_cmp) const;

  u32 GetFrameRate() const noexcept;
  f32 GetFrameTime() const noexcept;

 private:
  void UpdateEntityTransforms();
  void ApplyTmpCode();

  u32 frame_rate_{0};
  u32 counter_{0};
  u32 max_frame_rate_{60};  // 60 Hz refresh by default.
  f64 current_time_{0};
  f64 fixed_delta_time_{16.66};  // 60 Hz refresh by default.
  entity::EntityManager* entity_manager_{nullptr};
  event::EventManager* event_manager_{nullptr};
  time::TimeManager* time_manager_{nullptr};
};
}  // namespace physics
}  // namespace comet

#endif  // COMET_COMET_PHYSICS_PHYSICS_MANAGER_H_
