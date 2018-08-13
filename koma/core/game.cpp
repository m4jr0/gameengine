// Copyright 2018 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Allow debugging memory leaks.
#include "../debug.hpp"

#include "game.hpp"

#include <iostream>
#include <string>

#include "../utils/logger.hpp"
#include "input/input_manager.hpp"
#include "locator/locator.hpp"

// TODO(m4jr0): Remove this include when the main camera will be set
// elsewhere.
#include "game_object/camera/perspective_camera.hpp"

namespace koma {
Game::Game() {
  this->physics_manager_ = PhysicsManager();
  this->rendering_manager_ = RenderingManager();
  this->game_object_manager_ = GameObjectManager();
  this->time_manager_ = TimeManager();
}

Game::~Game() {}

void Game::Initialize() {
  // TODO(m4jr0): Move the lines about the main camera elsewere, when a
  // configuration file (of some sort) will be available for default/saved
  // settings.
  GameObject *camera_container = new GameObject();
  PerspectiveCamera *main_camera = new PerspectiveCamera();
  main_camera->position(4, 3, 3);
  main_camera->direction(-4, -3, -3);

  Locator::Initialize(this);
  Locator::rendering_manager(&this->rendering_manager_);
  Locator::time_manager(&this->time_manager_);
  Locator::game_object_manager(&this->game_object_manager_);
  Locator::main_camera(main_camera);

  this->rendering_manager_.Initialize();
  this->physics_manager_.Initialize();

  // TODO(m4jr0): Move these lines as well (see TODO(m4jr0) above).
  camera_container->AddComponent(main_camera);
  this->game_object_manager_.AddGameObject(camera_container);
}

void Game::Run() {
  try {
    this->is_running_ = true;
    this->time_manager_.Initialize();
    // To catch up time taken to render.
    double lag = 0.0;

    Logger::Get(LOGGER_KOMA_CORE_GAME)->Info("Game started");

    while (this->is_running_) {
      this->time_manager_.Update();
      double time_delta = this->time_manager_.time_delta();

      lag += time_delta;
      Locator::input_manager().GetInput(Input::kToBeImplemented);

      // To render physics properly, we have to catch up with the lag.
      while (lag >= this->kMsPerUpdate_) {
        this->physics_manager_.Update(&this->game_object_manager_);
        lag -= this->kMsPerUpdate_;
      }

      // Rendering a frame can take quite a huge amount of time.
      this->rendering_manager_.Update(
        lag / this->kMsPerUpdate_,
        &this->game_object_manager_
      );
    }
  } catch (const std::runtime_error& runtime_error) {
    Logger::Get(LOGGER_KOMA_CORE_GAME)->Error(
      "Runtime error: ", runtime_error.what()
    );

    this->Quit();

    std::cin.get();
  } catch (const std::exception& exception) {
    Logger::Get(LOGGER_KOMA_CORE_GAME)->Error(
      "Exception: ", exception.what()
    );

    this->Quit();

    std::cin.get();
  } catch (...) {
    Logger::Get(LOGGER_KOMA_CORE_GAME)->Error(
      "Unknown failure occurred. Possible memory corruption"
    );

    std::cin.get();
  }
}

void Game::Stop() {
  this->is_running_ = false;

  Logger::Get(LOGGER_KOMA_CORE_GAME)->Info("Game stopped");
}

void Game::Quit() {
  this->Stop();

  this->rendering_manager_.Destroy();
  this->physics_manager_.Destroy();

  Logger::Get(LOGGER_KOMA_CORE_GAME)->Info("Game quit");
}

const bool Game::is_running() const noexcept {
  return this->is_running_;
};
};  // namespace koma
