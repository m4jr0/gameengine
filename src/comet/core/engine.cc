// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "engine.h"

#include "comet/event/event.h"
#include "comet/event/event_manager.h"
#include "comet/event/window_event.h"
#include "comet/input/input_manager.h"

namespace comet {
void Engine::Initialize() {
  PreLoad();
  Load();
  PostLoad();
}

void Engine::Run() {
  try {
    is_running_ = true;
    time_manager_.SetFixedDeltaTime(COMET_CONF_CORE(f64, "ms_per_update"));
    time_manager_.Initialize();
    // To catch up time taken to render.
    f64 lag{0.0};
    COMET_LOG_CORE_INFO("Comet started");

    while (is_running_) {
      if (is_exit_requested_) {
        break;
      }

      time_manager_.Update();
      const auto fixed_delta_time{time_manager_.GetFixedDeltaTime()};
      const auto delta_time{time_manager_.GetDeltaTime()};

      lag += delta_time;

      // To render physics properly, we have to catch up with the lag.
      while (lag >= fixed_delta_time) {
        event_manager_.FireAllEvents();
        physics_manager_.Update(GetEntityManager());
        lag -= fixed_delta_time;
      }

      // Rendering a frame can take quite a huge amount of time.
      rendering_manager_.Update(lag / fixed_delta_time, GetEntityManager());
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

void Engine::Stop() {
  is_running_ = false;
  COMET_LOG_CORE_INFO("Comet stopped");
}

void Engine::Destroy() {
  PreUnload();
  Unload();
  PostUnload();

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
  resource_manager_.Initialize();
}

void Engine::Load() {
  rendering_manager_.Initialize();
  physics_manager_.Initialize();

  input_manager_.AttachGlfwWindow(const_cast<GLFWwindow*>(
      static_cast<const rendering::GlfwWindow*>(
          Engine::Get().GetRenderingManager().GetWindow())
          ->GetHandle()));

  input_manager_.Initialize();

  const auto event_function{COMET_EVENT_BIND_FUNCTION(Engine::OnEvent)};

  event_manager_.Register(event_function,
                          event::WindowCloseEvent::kStaticType_);

  entity_manager_.Initialize();
}

void Engine::PostLoad() {}

void Engine::PreUnload() {
  entity_manager_.Destroy();
  physics_manager_.Destroy();
  rendering_manager_.Destroy();
  COMET_STRING_ID_DESTROY();
}

void Engine::Unload() {
  resource_manager_.Destroy();
  configuration_manager_.Destroy();
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

resource::ResourceManager& Engine::GetResourceManager() {
  return resource_manager_;
}

rendering::RenderingManager& Engine::GetRenderingManager() {
  return rendering_manager_;
}

input::InputManager& Engine::GetInputManager() { return input_manager_; }

time::TimeManager& Engine::GetTimeManager() { return time_manager_; }

entity::EntityManager& Engine::GetEntityManager() { return entity_manager_; }

event::EventManager& Engine::GetEventManager() { return event_manager_; }

const bool Engine::is_running() const noexcept { return is_running_; }
}  // namespace comet
