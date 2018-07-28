// Copyright 2018 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "game.hpp"

namespace koma {
Game::Game() {
  this->physics_manager_ = PhysicsManager();
  this->rendering_manager_ = RenderingManager();
  this->game_object_manager_ = GameObjectManager();

  this->time_manager = TimeManager();
  this->time_manager.AddObserver(this);

  Locator::Initialize();
  Locator::time_manager(&this->time_manager);
  Locator::game_object_manager(&this->game_object_manager_);
}

Game::~Game() {}

void Game::Run() {
  std::cout << "Game started" << std::endl;
  this->is_running_ = true;
  this->time_manager.Initialize();
  double lag = 0.0;

  while (this->is_running_) {
    this->time_manager.Update();

    lag += this->time_manager.time_delta();
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
  std::cout << "Game stopped" << std::endl;

  this->is_running_ = false;
}

void Game::ReceiveEvent(std::string event) {
  if (event.compare("second") == 0) {
    // TODO(m4jr): Remove both of the frame counters when the time comes.
    std::cout << "PHYSICS " << this->physics_manager_.counter() << std::endl;
    std::cout << "FPS " << this->rendering_manager_.counter() << std::endl;

    this->physics_manager_.ResetCounter();
    this->rendering_manager_.ResetCounter();
  } else if (event.compare("stop") == 0) {
    this->Stop();
  } else if (event.compare("run") == 0) {
    this->Run();
  }
}
};  // namespace koma
