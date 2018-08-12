// Copyright 2018 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Allows debugging memory leaks.
#include "../debug.hpp"

#include "game.hpp"

#include <iostream>
#include <string>

#include "../utils/logger.hpp"
#include "input/input_manager.hpp"
#include "locator/locator.hpp"

namespace koma {
Game::Game() {
  this->physics_manager_ = PhysicsManager();
  this->rendering_manager_ = RenderingManager();
  this->game_object_manager_ = GameObjectManager();

  this->time_manager_ = TimeManager();
}

Game::~Game() {}

void Game::Initialize() {
  Locator::Initialize(this);
  Locator::rendering_manager(&this->rendering_manager_);
  Locator::time_manager(&this->time_manager_);
  Locator::game_object_manager(&this->game_object_manager_);

  this->rendering_manager_.Initialize();
  this->physics_manager_.Initialize();
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
      this->rendering_manager_.Update(lag / this->kMsPerUpdate_,
        &this->game_object_manager_);
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
