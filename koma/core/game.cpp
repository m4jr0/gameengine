// Copyright 2018 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "game.hpp"

namespace koma {
Game::Game() {
  this->physics_manager_ = PhysicsManager();
  this->rendering_manager_ = RenderingManager();
  this->game_object_manager_ = GameObjectManager();

  this->time_manager_ = TimeManager();

  Locator::Initialize(this);
  Locator::time_manager(&this->time_manager_);
  Locator::game_object_manager(&this->game_object_manager_);
}

Game::~Game() {}

void Game::Initialize() {
  this->rendering_manager_.Initialize();
  this->physics_manager_.Initialize();
}

void Game::Run() {
  this->is_running_ = true;
  this->time_manager_.Initialize();
  // To catch up time taken to render.
  double lag = 0.0;
  // To known when we reach a second to display counters accordingly.
  double time_counter = 0;

  Logger::Get(LOGGER_KOMA_CORE_GAME)->Message("Game started");

  while (this->is_running_) {
    this->time_manager_.Update();
    double time_delta = this->time_manager_.time_delta();
    time_counter += time_delta;

    // Here, we reset the counters which check for the physics and the
    // rendering refreshing rate.
    if (time_counter > 1000) {
      this->ResetCounters();
      time_counter = 0;
    }

    lag += time_delta;
    Locator::input_manager().GetInput(Input::TO_BE_IMPLEMENTED);

    // To render physics properly, we have to catch up with the lag.
    while (lag >= Game::MS_PER_UPDATE) {
      this->physics_manager_.Update(&this->game_object_manager_);
      lag -= Game::MS_PER_UPDATE;
    }

    // Rendering a frame can take quite a huge amount of time.
    this->rendering_manager_.Update(
      lag / Game::MS_PER_UPDATE,
      &this->game_object_manager_
    );
  }
}

void Game::Stop() {
  this->is_running_ = false;

  Logger::Get(LOGGER_KOMA_CORE_GAME)->Message("Game stopped");
}

void Game::Quit() {
  this->Stop();

  this->rendering_manager_.Destroy();
  this->physics_manager_.Destroy();

  Logger::Get(LOGGER_KOMA_CORE_GAME)->Message("Game quit");
}

void Game::ResetCounters() {
  auto logger = Logger::Get(LOGGER_KOMA_CORE_GAME);

  // TODO(m4jr0): Remove these messages when the rendering part will be (at
  // least) partially implemented.
  logger->Message(
    "Physics ", this->physics_manager_.counter()
  );

  logger->Message(
    "FPS ", this->rendering_manager_.counter()
  );

  this->physics_manager_.ResetCounter();
  this->rendering_manager_.ResetCounter();
}
};  // namespace koma
