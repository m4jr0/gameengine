// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_ENGINE_H_
#define COMET_COMET_CORE_ENGINE_H_

#include "comet_precompile.h"

#include "comet/core/configuration_manager.h"
#include "comet/entity/entity.h"
#include "comet/event/event_manager.h"
#include "comet/input/input_manager.h"
#include "comet/physics/physics_manager.h"
#include "comet/rendering/rendering_manager.h"
#include "comet/resource/resource_manager.h"
#include "comet/time/time_manager.h"

namespace comet {
class Engine {
 public:
  Engine(const Engine&) = delete;
  Engine(Engine&&) = delete;
  Engine& operator=(const Engine&) = delete;
  Engine& operator=(Engine&&) = delete;
  virtual ~Engine() = default;

  virtual void Initialize();
  virtual void Run();
  virtual void Stop();
  virtual void Destroy();
  virtual void Quit();

  static Engine& Get();
  conf::ConfigurationManager& GetConfigurationManager();
  resource::ResourceManager& GetResourceManager();
  rendering::RenderingManager& GetRenderingManager();
  input::InputManager& GetInputManager();
  time::TimeManager& GetTimeManager();
  entity::EntityManager& GetEntityManager();
  event::EventManager& GetEventManager();

  const bool is_running() const noexcept;

 protected:
  inline static Engine* engine_;

  Engine();

  virtual void Exit();
  void OnEvent(const event::Event& event);

  conf::ConfigurationManager configuration_manager_{};
  resource::ResourceManager resource_manager_{};
  input::InputManager input_manager_{};
  physics::PhysicsManager physics_manager_{};
  rendering::RenderingManager rendering_manager_{};
  entity::EntityManager entity_manager_{};
  time::TimeManager time_manager_{};
  event::EventManager event_manager_{};

 private:
  bool is_running_{false};
  bool is_exit_requested_{false};
  f64 msPerUpdate_{16.66};  // 60 Hz refresh.
};

std::unique_ptr<Engine> CreateEngine();
}  // namespace comet

#endif  // COMET_COMET_CORE_ENGINE_H_
