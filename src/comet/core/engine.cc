// Copyright 2021 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "engine.h"

#include "comet/game_object/camera/camera_controls.h"
#include "comet/game_object/camera/perspective_camera.h"
#include "comet/input/input_manager.h"

#ifdef _WIN32
#include "debug_windows.h"
#endif  // _WIN32

namespace comet {
namespace core {
void Engine::Initialize() {
  // TODO(m4jr0): Move the lines about the main camera elsewere, when a
  // configuration file (of some sort) will be available for default/saved
  // settings.
  auto camera_container = game_object::GameObject::Create();
  main_camera_ = std::make_shared<game_object::PerspectiveCamera>();
  auto camera_controls = std::make_shared<game_object::CameraControls>();
  main_camera_->position(0, 0, 3);
  main_camera_->direction(0.715616, 0.691498, -0.098611);

  rendering_manager_->Initialize();
  resource_manager_->Initialize();
  physics_manager_->Initialize();
  input_manager_->Initialize();

  // TODO(m4jr0): Move these lines as well (see TODO(m4jr0) above).
  camera_container->AddComponent(main_camera_);
  camera_container->AddComponent(camera_controls);
  game_object_manager_->AddGameObject(camera_container);
}

void Engine::Run() {
  try {
    is_running_ = true;
    time_manager_->Initialize();
    // To catch up time taken to render.
    double lag = 0.0;

    Logger::Get(LoggerType::Core)->Info("Engine started");

    while (is_running_) {
      if (is_exit_requested_) {
        break;
      }

      time_manager_->Update();
      const auto time_delta = time_manager_->time_delta();

      lag += time_delta;

      // To render physics properly, we have to catch up with the lag.
      while (lag >= kMsPerUpdate_) {
        event_manager_->FireAllEvents();
        physics_manager_->Update(game_object_manager());
        lag -= kMsPerUpdate_;
      }

      // Rendering a frame can take quite a huge amount of time.
      rendering_manager_->Update(lag / kMsPerUpdate_, game_object_manager());
    }
  } catch (const std::runtime_error& runtime_error) {
    Logger::Get(LoggerType::Core)
        ->Error("Runtime error: ", runtime_error.what());

    Quit();

    std::cin.get();
  } catch (const std::exception& exception) {
    Logger::Get(LoggerType::Core)->Error("Exception: ", exception.what());

    Quit();

    std::cin.get();
  } catch (...) {
    Logger::Get(LoggerType::Core)
        ->Error("Unknown failure occurred. Possible memory corruption");

    std::cin.get();
  }

  Exit();
}

void Engine::Stop() {
  is_running_ = false;

  Logger::Get(LoggerType::Core)->Info("Engine stopped");
}

void Engine::Destroy() {
  game_object_manager_->Destroy();
  physics_manager_->Destroy();
  rendering_manager_->Destroy();
  Engine::engine_ = nullptr;

  Logger::Get(LoggerType::Core)->Info("Engine destroyed");
}

void Engine::Quit() {
  is_exit_requested_ = true;
  Logger::Get(LoggerType::Core)->Info("Engine is required to quit");
}

Engine::Engine() {
  resource_manager_ = std::make_unique<resource::ResourceManager>();
  physics_manager_ = std::make_unique<physics::PhysicsManager>();
  rendering_manager_ = std::make_unique<rendering::RenderingManager>();
  game_object_manager_ = std::make_unique<game_object::GameObjectManager>();
  time_manager_ = std::make_unique<time::TimeManager>();
  input_manager_ = std::make_unique<input::InputManager>();
  event_manager_ = std::make_unique<event::EventManager>();

  Engine::engine_ = this;
}

void Engine::Exit() {
  Stop();
  Destroy();

  Logger::Get(LoggerType::Core)->Info("Engine quit");
}

Engine* const Engine::engine() { return Engine::engine_; }

resource::ResourceManager* const Engine::resource_manager() {
  return resource_manager_.get();
}

rendering::RenderingManager* const Engine::rendering_manager() {
  return rendering_manager_.get();
}

input::InputManager* const Engine::input_manager() {
  return input_manager_.get();
}

time::TimeManager* const Engine::time_manager() { return time_manager_.get(); }

game_object::GameObjectManager* const Engine::game_object_manager() {
  return game_object_manager_.get();
}

event::EventManager* const Engine::event_manager() {
  return event_manager_.get();
}

game_object::Camera* const Engine::main_camera() { return main_camera_.get(); }

const bool Engine::is_running() const noexcept { return is_running_; }
}  // namespace core
}  // namespace comet
