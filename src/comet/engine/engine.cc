// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "engine.h"

#include "comet/event/event.h"
#include "comet/event/window_event.h"

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
    time_manager_->Initialize();
    // To catch up time taken to render.
    f64 lag{0.0};
    COMET_LOG_CORE_INFO("Comet started");

    while (is_running_) {
      if (is_exit_requested_) {
        break;
      }

      Update(lag);
    }
  } catch (const std::runtime_error& runtime_error) {
    COMET_LOG_CORE_ERROR("Runtime error: ", runtime_error.what());
    Quit();

    std::cin.get();
  } catch (const std::exception& exception) {
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
  memory_manager_->Update();
  time_manager_->Update();
  lag += time_manager_->GetDeltaTime();
  physics_manager_->Update(lag);
  rendering_manager_->Update(lag / time_manager_->GetFixedDeltaTime());

#ifdef COMET_PROFILING
  profiler_manager_->Update();
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
  conf::ConfigurationManagerDescr configuration_manager_descr{};
  configuration_manager_ =
      std::make_unique<conf::ConfigurationManager>(configuration_manager_descr);

  memory::MemoryManagerDescr memory_manager_descr{};
  memory_manager_ =
      std::make_unique<memory::MemoryManager>(memory_manager_descr);

  resource::ResourceManagerDescr resource_manager_descr{};
  resource_manager_descr.configuration_manager = configuration_manager_.get();
  resource_manager_ =
      std::make_unique<resource::ResourceManager>(resource_manager_descr);

  time::TimeManagerDescr time_manager_descr{};
  time_manager_descr.configuration_manager = configuration_manager_.get();
  time_manager_ = std::make_unique<time::TimeManager>(time_manager_descr);

  entity::EntityManagerDescr entity_manager_descr{};
  entity_manager_ =
      std::make_unique<entity::EntityManager>(entity_manager_descr);

  entity::EntityFactoryManagerDescr entity_factory_manager_descr{};
  entity_factory_manager_descr.entity_manager = entity_manager_.get();
  entity_factory_manager_descr.resource_manager = resource_manager_.get();
  entity_factory_manager_ = std::make_unique<entity::EntityFactoryManager>(
      entity_factory_manager_descr);

  event::EventManagerDescr event_manager_descr{};
  event_manager_ = std::make_unique<event::EventManager>(event_manager_descr);

  input::InputManagerDescr input_manager_descr{};
  input_manager_descr.event_manager = event_manager_.get();
  input_manager_ = std::make_unique<input::InputManager>(input_manager_descr);

  physics::PhysicsManagerDescr physics_manager_descr{};
  physics_manager_descr.entity_manager = entity_manager_.get();
  physics_manager_descr.event_manager = event_manager_.get();
  physics_manager_descr.time_manager = time_manager_.get();
  physics_manager_ =
      std::make_unique<physics::PhysicsManager>(physics_manager_descr);

  rendering::CameraManagerDescr camera_manager_descr{};
  camera_manager_descr.event_manager = event_manager_.get();
  camera_manager_ =
      std::make_unique<rendering::CameraManager>(camera_manager_descr);

#ifdef COMET_DEBUG
  rendering::DebuggerDisplayerManagerDescr debugger_displayer_manager_descr{};
  debugger_displayer_manager_ =
      std::make_unique<rendering::DebuggerDisplayerManager>(
          debugger_displayer_manager_descr);
#endif  // COMET_DEBUG

  rendering::RenderingManagerDescr rendering_manager_descr{};
  rendering_manager_descr.camera_manager = camera_manager_.get();
  rendering_manager_descr.configuration_manager = configuration_manager_.get();
#ifdef COMET_DEBUG
  rendering_manager_descr.debugger_displayer_manager =
      debugger_displayer_manager_.get();
#endif  // COMET_DEBUG
  rendering_manager_descr.entity_manager = entity_manager_.get();
  rendering_manager_descr.event_manager = event_manager_.get();
  rendering_manager_descr.input_manager = input_manager_.get();
  rendering_manager_descr.resource_manager = resource_manager_.get();
  rendering_manager_descr.time_manager = time_manager_.get();
  rendering_manager_ =
      std::make_unique<rendering::RenderingManager>(rendering_manager_descr);

#ifdef COMET_PROFILING
  profiler::ProfilerManagerDescr profiler_manager_descr{};
#ifdef COMET_DEBUG
  profiler_manager_descr.debugger_displayer_manager =
      debugger_displayer_manager_.get();
#endif  // COMET_DEBUG
  profiler_manager_descr.memory_manager = memory_manager_.get();
  profiler_manager_descr.physics_manager = physics_manager_.get();
  profiler_manager_descr.rendering_manager = rendering_manager_.get();
  profiler_manager_ =
      std::make_unique<profiler::ProfilerManager>(profiler_manager_descr);

  profiler_manager_->Initialize();
#endif  // COMET_PROFILING

  configuration_manager_->Initialize();
  memory_manager_->Initialize();
  event_manager_->Initialize();
  resource_manager_->Initialize();
}

void Engine::Load() {
#ifdef COMET_DEBUG
  debugger_displayer_manager_->Initialize();
#endif  // COMET_DEBUG
  rendering_manager_->Initialize();
  camera_manager_->Initialize();
  physics_manager_->Initialize();
  input_manager_->Initialize();

  const auto event_function{COMET_EVENT_BIND_FUNCTION(Engine::OnEvent)};

  event_manager_->Register(event_function,
                           event::WindowCloseEvent::kStaticType_);

  entity_manager_->Initialize();
  entity_factory_manager_->Initialize();
}

void Engine::PostLoad() {}

void Engine::PreUnload() {
  entity_factory_manager_->Shutdown();
  entity_manager_->Shutdown();
  physics_manager_->Shutdown();
  input_manager_->Shutdown();
  camera_manager_->Shutdown();
  rendering_manager_->Shutdown();
#ifdef COMET_DEBUG
  debugger_displayer_manager_->Shutdown();
#endif  // COMET_DEBUG
  time_manager_->Shutdown();

#ifdef COMET_PROFILING
  profiler_manager_->Shutdown();
#endif  // COMET_PROFILING
  COMET_STRING_ID_DESTROY();
}

void Engine::Unload() {
  resource_manager_->Shutdown();
  event_manager_->Shutdown();
  memory_manager_->Shutdown();
  configuration_manager_->Shutdown();
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
