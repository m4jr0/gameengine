// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "physics_manager.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include "comet/core/engine.h"
#include "comet/entity/component/transform_component.h"
#include "comet/entity/entity.h"
#include "comet/entity/entity_manager.h"

namespace comet {
namespace physics {
void PhysicsManager::Initialize() {
  Manager::Initialize();
  const auto& time_manager{Engine::Get().GetTimeManager()};
  fixed_delta_time_ = time_manager.GetFixedDeltaTime();
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
  const auto& time_manager{Engine::Get().GetTimeManager()};
  auto& event_manager{Engine::Get().GetEventManager()};

  if (current_time_ > 1000) {
    frame_rate_ = counter_;
    current_time_ = 0;
    counter_ = 0;
  }

  current_time_ += time_manager.GetDeltaTime();

  while (lag > fixed_delta_time_) {
    // TODO(m4jr0): Investigate. This seems to prevent round errors.
    if (counter_ == max_frame_rate_) {
      lag = 0;
      return;
    }

    event_manager.FireAllEvents();
    auto& entity_manager{Engine::Get().GetEntityManager()};

    // TODO(m4jr0): Remove temporary code.
    auto view{entity_manager.GetView<entity::TransformComponent>()};
    static double rot_step = 0;
    rot_step += 0.01;

    for (const auto entity_id : view) {
      auto* transform_cmp{
          entity_manager.GetComponent<entity::TransformComponent>(entity_id)};
      transform_cmp->global = glm::rotate(
          glm::mat4(1.0f), static_cast<float>(rot_step * glm::radians(90.0f)),
          glm::vec3(0.0f, 1.0f, 0.0f));
    }

    ++counter_;
    lag -= fixed_delta_time_;
  }
}

u32 PhysicsManager::GetFrameRate() const noexcept { return frame_rate_; }

f32 PhysicsManager::GetFrameTime() const noexcept {
  return (1 / static_cast<f32>(frame_rate_)) * 1000;
}
}  // namespace physics
}  // namespace comet
