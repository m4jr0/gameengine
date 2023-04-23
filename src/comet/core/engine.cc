// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "engine.h"

#include "comet/event/event.h"
#include "comet/event/event_manager.h"
#include "comet/event/window_event.h"
#include "comet/input/input_manager.h"

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
    time_manager_.Initialize();
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
  time_manager_.Update();
  lag += time_manager_.GetDeltaTime();
  physics_manager_.Update(lag);
  rendering_manager_.Update(lag / time_manager_.GetFixedDeltaTime());
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
  configuration_manager_.Initialize();
  event_manager_.Initialize();
  resource_manager_.Initialize();
}

void Engine::Load() {
  rendering_manager_.Initialize();
  camera_manager_.Initialize();
  physics_manager_.Initialize();
  input_manager_.Initialize();

  const auto event_function{COMET_EVENT_BIND_FUNCTION(Engine::OnEvent)};

  event_manager_.Register(event_function,
                          event::WindowCloseEvent::kStaticType_);

  entity_manager_.Initialize();
}

void Engine::PostLoad() {}

void Engine::PreUnload() {
  entity_manager_.Shutdown();
  physics_manager_.Shutdown();
  input_manager_.Shutdown();
  camera_manager_.Shutdown();
  rendering_manager_.Shutdown();
  time_manager_.Shutdown();
  COMET_STRING_ID_DESTROY();
}

void Engine::Unload() {
  resource_manager_.Shutdown();
  configuration_manager_.Shutdown();
  event_manager_.Shutdown();
  is_running_ = false;
  is_exit_requested_ = false;
}

void Engine::PostUnload() { Engine::engine_ = nullptr; }

Engine::Engine() { Engine::engine_ = this; }

void Engine::Exit() {
  Stop();
  COMET_LOG_CORE_INFO("Comet quit");
}

void Engine::OnEvent(const event::Event& event) {
  const auto& event_type{event.GetType()};

  if (event_type == event::WindowCloseEvent::kStaticType_) {
    COMET_LOG_CORE_DEBUG("Close event.");
    Quit();
  }
}

Engine& Engine::Get() { return *Engine::engine_; }

conf::ConfigurationManager& Engine::GetConfigurationManager() {
  return configuration_manager_;
}

entity::EntityManager& Engine::GetEntityManager() { return entity_manager_; }

event::EventManager& Engine::GetEventManager() { return event_manager_; }

input::InputManager& Engine::GetInputManager() { return input_manager_; }

physics::PhysicsManager& Engine::GetPhysicsManager() {
  return physics_manager_;
}

rendering::CameraManager& Engine::GetCameraManager() { return camera_manager_; }

rendering::RenderingManager& Engine::GetRenderingManager() {
  return rendering_manager_;
}

resource::ResourceManager& Engine::GetResourceManager() {
  return resource_manager_;
}

time::TimeManager& Engine::GetTimeManager() { return time_manager_; }

bool Engine::IsRunning() const noexcept { return is_running_; }

bool Engine::IsInitialized() const noexcept { return is_initialized_; }
}  // namespace comet
