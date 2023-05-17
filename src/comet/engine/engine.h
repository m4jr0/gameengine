// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_ENGINE_H_
#define COMET_COMET_CORE_ENGINE_H_

#include "comet_precompile.h"

#include "comet/core/conf/configuration_manager.h"
#include "comet/core/memory/memory_manager.h"
#include "comet/entity/entity_manager.h"
#include "comet/entity/factory/entity_factory_manager.h"
#include "comet/event/event_manager.h"
#include "comet/input/input_manager.h"
#include "comet/physics/physics_manager.h"
#include "comet/rendering/camera/camera_manager.h"
#include "comet/rendering/rendering_manager.h"
#include "comet/resource/resource_manager.h"
#include "comet/time/time_manager.h"

#ifdef COMET_DEBUG
#include "comet/rendering/debugger/debugger_displayer_manager.h"
#endif  // COMET_DEBUG

#ifdef COMET_PROFILING
#include "comet/profiler/profiler_manager.h"
#endif  // COMET_PROFILING

namespace comet {
class Engine {
 public:
  Engine(const Engine&) = delete;
  Engine(Engine&&) = delete;
  Engine& operator=(const Engine&) = delete;
  Engine& operator=(Engine&&) = delete;
  virtual ~Engine();

  void Initialize();
  void Run();
  virtual void Update(f64& lag);
  void Stop();
  void Shutdown();
  void Quit();

  virtual void PreLoad();
  virtual void Load();
  virtual void PostLoad();

  virtual void PreUnload();
  virtual void Unload();
  virtual void PostUnload();

  bool IsRunning() const noexcept;
  bool IsInitialized() const noexcept;

 protected:
  inline static Engine* engine_{nullptr};

  Engine();

  void Exit();
  static Engine& Get();
  void OnEvent(const event::Event& event);

  std::unique_ptr<conf::ConfigurationManager> configuration_manager_{nullptr};
  std::unique_ptr<entity::EntityManager> entity_manager_{nullptr};
  std::unique_ptr<entity::EntityFactoryManager> entity_factory_manager_{
      nullptr};
  std::unique_ptr<event::EventManager> event_manager_{nullptr};
  std::unique_ptr<input::InputManager> input_manager_{nullptr};
  std::unique_ptr<physics::PhysicsManager> physics_manager_{nullptr};
  std::unique_ptr<rendering::CameraManager> camera_manager_{nullptr};
  std::unique_ptr<rendering::RenderingManager> rendering_manager_{nullptr};
  std::unique_ptr<resource::ResourceManager> resource_manager_{nullptr};
  std::unique_ptr<time::TimeManager> time_manager_{nullptr};
  std::unique_ptr<memory::MemoryManager> memory_manager_{nullptr};

#ifdef COMET_DEBUG
  std::unique_ptr<rendering::DebuggerDisplayerManager>
      debugger_displayer_manager_{nullptr};
#endif  // COMET_DEBUG
#ifdef COMET_PROFILING
  std::unique_ptr<profiler::ProfilerManager> profiler_manager_{nullptr};
#endif  // COMET_PROFILING

 private:
  bool is_initialized_{false};
  bool is_running_{false};
  bool is_exit_requested_{false};
};

std::unique_ptr<Engine> GenerateEngine();
}  // namespace comet

#endif  // COMET_COMET_CORE_ENGINE_H_
