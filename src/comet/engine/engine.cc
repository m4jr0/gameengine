// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "engine.h"

#include "comet/animation/animation_manager.h"
#include "comet/core/conf/configuration_manager.h"
#include "comet/core/memory/memory_manager.h"
#include "comet/entity/entity_manager.h"
#include "comet/entity/factory/entity_factory_manager.h"
#include "comet/event/event.h"
#include "comet/event/event_manager.h"
#include "comet/event/window_event.h"
#include "comet/geometry/geometry_manager.h"
#include "comet/input/input_manager.h"
#include "comet/physics/physics_manager.h"
#include "comet/rendering/camera/camera_manager.h"
#include "comet/rendering/rendering_manager.h"
#include "comet/resource/resource_manager.h"
#include "comet/time/time_manager.h"

#ifdef COMET_PROFILING
#include "comet/profiler/profiler_manager.h"
#endif  // COMET_PROFILIN

#ifdef COMET_DEBUG
#include "comet/rendering/debugger/debugger_displayer_manager.h"
#endif  // COMET_DEBUG

namespace comet {
Engine::~Engine() {
  COMET_ASSERT(!is_initialized_,
               "Destructor called for engine, but it is still initialized!");
}

void Engine::Initialize() {
  COMET_ASSERT(!is_initialized_,
               "Tried to initialize engine, but it is already done!");
  PreLoad();
  Load();
  PostLoad();
  is_initialized_ = true;
}

void Engine::Run() {
  try {
    is_running_ = true;
    time::TimeManager::Get().Initialize();
    // To catch up time taken to render.
    f64 lag{0.0};
    COMET_LOG_CORE_INFO("Comet started");

    while (is_running_) {
      if (is_exit_requested_) {
        break;
      }

      Update(lag);
    }
  } catch ([[maybe_unused]] const std::runtime_error& runtime_error) {
    COMET_LOG_CORE_ERROR("Runtime error: ", runtime_error.what());
    Quit();

    std::cin.get();
  } catch ([[maybe_unused]] const std::exception& exception) {
    COMET_LOG_CORE_ERROR("Exception: ", exception.what());
    Quit();

    std::cin.get();
  } catch (...) {
    COMET_LOG_CORE_ERROR(
        "Unknown failure occurred. Possible memory corruption");
    std::cin.get();
  }

  Exit();
}

void Engine::Update(f64& lag) {
  time::TimeManager::Get().Update();
  lag += time::TimeManager::Get().GetDeltaTime();
  physics::PhysicsManager::Get().Update(lag);
  animation::AnimationManager::Get().Update(lag);
  rendering::RenderingManager::Get().Update(
      lag / time::TimeManager::Get().GetFixedDeltaTime());

#ifdef COMET_PROFILING
  profiler::ProfilerManager::Get().Update();
#endif  // COMET_PROFILING
}

void Engine::Stop() {
  is_running_ = false;
  COMET_LOG_CORE_INFO("Comet stopped");
}

void Engine::Shutdown() {
  COMET_ASSERT(is_initialized_,
               "Tried to shutdown engine, but it is not initialized!");
  PreUnload();
  Unload();
  PostUnload();
  is_initialized_ = false;

  COMET_LOG_CORE_INFO("Comet destroyed");
}

void Engine::Quit() {
  if (is_exit_requested_) {
    return;
  }

  is_exit_requested_ = true;
  COMET_LOG_CORE_INFO("Comet is required to quit");
}

void Engine::PreLoad() {
#ifdef COMET_PROFILING
  profiler::ProfilerManager::Get().Initialize();
#endif  // COMET_PROFILING

  conf::ConfigurationManager::Get().Initialize();
  memory::MemoryManager::Get().Initialize();
  event::EventManager::Get().Initialize();
  resource::ResourceManager::Get().Initialize();
}

void Engine::Load() {
#ifdef COMET_DEBUG
  rendering::DebuggerDisplayerManager::Get().Initialize();
#endif  // COMET_DEBUG
  rendering::RenderingManager::Get().Initialize();
  rendering::CameraManager::Get().Initialize();
  physics::PhysicsManager::Get().Initialize();
  input::InputManager::Get().Initialize();

  const auto event_function{COMET_EVENT_BIND_FUNCTION(Engine::OnEvent)};

  event::EventManager::Get().Register(event_function,
                                      event::WindowCloseEvent::kStaticType_);

  animation::AnimationManager::Get().Initialize();
  entity::EntityManager::Get().Initialize();
  geometry::GeometryManager::Get().Initialize();
  entity::EntityFactoryManager::Get().Initialize();
}

void Engine::PostLoad() {}

void Engine::PreUnload() {
  entity::EntityFactoryManager::Get().Shutdown();
  geometry::GeometryManager::Get().Shutdown();
  entity::EntityManager::Get().Shutdown();
  animation::AnimationManager::Get().Shutdown();
  physics::PhysicsManager::Get().Shutdown();
  input::InputManager::Get().Shutdown();
  rendering::CameraManager::Get().Shutdown();
  rendering::RenderingManager::Get().Shutdown();
#ifdef COMET_DEBUG
  rendering::DebuggerDisplayerManager::Get().Shutdown();
#endif  // COMET_DEBUG
  time::TimeManager::Get().Shutdown();

#ifdef COMET_PROFILING
  profiler::ProfilerManager::Get().Shutdown();
#endif  // COMET_PROFILING
  COMET_STRING_ID_DESTROY();
}

void Engine::Unload() {
  resource::ResourceManager::Get().Shutdown();
  event::EventManager::Get().Shutdown();
  memory::MemoryManager::Get().Shutdown();
  conf::ConfigurationManager::Get().Shutdown();
  is_running_ = false;
  is_exit_requested_ = false;
}

void Engine::PostUnload() { Engine::engine_ = nullptr; }

Engine::Engine() { Engine::engine_ = this; }

void Engine::Exit() {
  Stop();
  COMET_LOG_CORE_INFO("Comet quit");
}

Engine& Engine::Get() { return *Engine::engine_; }

void Engine::OnEvent(const event::Event& event) {
  const auto& event_type{event.GetType()};

  if (event_type == event::WindowCloseEvent::kStaticType_) {
    COMET_LOG_CORE_DEBUG("Close event.");
    Quit();
  }
}

bool Engine::IsRunning() const noexcept { return is_running_; }

bool Engine::IsInitialized() const noexcept { return is_initialized_; }
}  // namespace comet
