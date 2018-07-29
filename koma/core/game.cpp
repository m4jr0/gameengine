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

void Game::Run() {
  this->is_running_ = true;
  this->time_manager_.Initialize();
  double lag = 0.0;
  double time_counter = 0;

  std::cout << "Game started" << std::endl;

  while (this->is_running_) {
    this->time_manager_.Update();
    double time_delta = this->time_manager_.time_delta();
    time_counter += time_delta;

    if (time_counter > 1000) {
      this->ResetCounters();
      time_counter = 0;
    }

    lag += time_delta;
    Locator::input_manager().GetInput(Input::TO_BE_IMPLEMENTED);

    while (lag >= Game::MS_PER_UPDATE) {
      this->physics_manager_.Update(&this->game_object_manager_);
      lag -= Game::MS_PER_UPDATE;
    }

    this->rendering_manager_.Update(
      lag / Game::MS_PER_UPDATE,
      &this->game_object_manager_
    );
  }
}

void Game::Stop() {
  this->is_running_ = false;

  std::cout << "Game stopped" << std::endl;
}

void Game::Quit() {
  this->Stop();

  std::cout << "Game quit" << std::endl;
}

void Game::ResetCounters() {
  std::cout << "PHYSICS " << this->physics_manager_.counter() << std::endl;
  std::cout << "FPS " << this->rendering_manager_.counter() << std::endl;

  this->physics_manager_.ResetCounter();
  this->rendering_manager_.ResetCounter();
}
};  // namespace koma
