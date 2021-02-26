// Copyright 2021 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "game.hpp"

#include <iostream>
#include <string>

// TODO(m4jr0): Remove this include when the main camera will be set
// elsewhere.
#include <memory>  // Except if it is used elsewhere, of course.

#include "game_object/camera/camera_controls.hpp"
#include "game_object/camera/perspective_camera.hpp"
#include "input/input_manager.hpp"
#include "locator/locator.hpp"
#include "utils/logger.hpp"

#ifdef _WIN32
// Allow debugging memory leaks on Windows.
#include "debug_windows.hpp"
#endif  // _WIN32

namespace koma {
Game::Game() {
  resource_manager_ = ResourceManager();
  physics_manager_ = PhysicsManager();
  render_manager_ = RenderManager();
  game_object_manager_ = GameObjectManager();
  time_manager_ = TimeManager();
  input_manager_ = InputManager();
}

Game::~Game() {}

void Game::Initialize() {
  // TODO(m4jr0): Move the lines about the main camera elsewere, when a
  // configuration file (of some sort) will be available for default/saved
  // settings.
  auto camera_container = GameObject::Create();
  auto main_camera = std::make_shared<PerspectiveCamera>();
  auto camera_controls = std::make_shared<CameraControls>();
  main_camera->position(0, 0, 3);
  main_camera->direction(0.715616, 0.691498, -0.098611);

  Locator::Initialize(this);
  Locator::resource_manager(&resource_manager_);
  Locator::render_manager(&render_manager_);
  Locator::time_manager(&time_manager_);
  Locator::game_object_manager(&game_object_manager_);
  Locator::main_camera(main_camera);

  render_manager_.Initialize();
  resource_manager_.Initialize();
  physics_manager_.Initialize();
  input_manager_.Initialize();

  // TODO(m4jr0): Move these lines as well (see TODO(m4jr0) above).
  camera_container->AddComponent(main_camera);
  camera_container->AddComponent(camera_controls);
  game_object_manager_.AddGameObject(camera_container);
}

void Game::Run() {
  try {
    is_running_ = true;
    time_manager_.Initialize();
    // To catch up time taken to render.
    double lag = 0.0;

    Logger::Get(LOGGER_KOMA_CORE_GAME)->Info("Game started");

    while (is_running_) {
      if (is_exit_requested_) {
        break;
      }

      time_manager_.Update();
      double time_delta = time_manager_.time_delta();

      lag += time_delta;

      // To render physics properly, we have to catch up with the lag.
      while (lag >= kMsPerUpdate_) {
        physics_manager_.Update(&game_object_manager_);
        lag -= kMsPerUpdate_;
      }

      // Rendering a frame can take quite a huge amount of time.
      render_manager_.Update(lag / kMsPerUpdate_, &game_object_manager_);
    }
  } catch (const std::runtime_error& runtime_error) {
    Logger::Get(LOGGER_KOMA_CORE_GAME)
        ->Error("Runtime error: ", runtime_error.what());

    Quit();

    std::cin.get();
  } catch (const std::exception& exception) {
    Logger::Get(LOGGER_KOMA_CORE_GAME)->Error("Exception: ", exception.what());

    Quit();

    std::cin.get();
  } catch (...) {
    Logger::Get(LOGGER_KOMA_CORE_GAME)
        ->Error("Unknown failure occurred. Possible memory corruption");

    std::cin.get();
  }

  Exit();
}

void Game::Stop() {
  is_running_ = false;

  Logger::Get(LOGGER_KOMA_CORE_GAME)->Info("Game stopped");
}

void Game::Destroy() {
  game_object_manager_.Destroy();
  physics_manager_.Destroy();
  render_manager_.Destroy();

  Logger::Get(LOGGER_KOMA_CORE_GAME)->Info("Game destroyed");
}

void Game::Exit() {
  Stop();
  Destroy();

  Logger::Get(LOGGER_KOMA_CORE_GAME)->Info("Game quit");
}

void Game::Quit() {
  is_exit_requested_ = true;

  Logger::Get(LOGGER_KOMA_CORE_GAME)->Info("Game is required to quit");
}

const bool Game::is_running() const noexcept { return is_running_; }
}  // namespace koma
