// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_ENGINE_H_
#define COMET_COMET_CORE_ENGINE_H_

#include "comet_precompile.h"

#include "comet/core/conf/configuration_manager.h"
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

  void Initialize();
  void Run();
  void Stop();
  void Destroy();
  void Quit();

  virtual void PreLoad();
  virtual void Load();
  virtual void PostLoad();

  virtual void PreUnload();
  virtual void Unload();
  virtual void PostUnload();

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

  void Exit();
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
};

std::unique_ptr<Engine> CreateEngine();
}  // namespace comet

#endif  // COMET_COMET_CORE_ENGINE_H_
