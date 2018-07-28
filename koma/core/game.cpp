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
}

Game::~Game() {}

void Game::Run() {
  this->is_running_ = true;
  this->time_manager.Initialize();
  double lag = 0.0;

  while (this->is_running_) {
    this->time_manager.Update();

    lag += this->time_manager.time_delta();
    Locator::input_manager().GetInput(Input::TO_BE_IMPLEMENTED);

    while (lag >= Game::MS_PER_UPDATE) {
      this->physics_manager_.Update(&this->game_object_manager_);
      // TODO(m4jr): Remove the line below when the time comes.
      this->physics_frame_counter_++;
      lag -= Game::MS_PER_UPDATE;
    }

    this->rendering_manager_.Update(
      lag / Game::MS_PER_UPDATE,
      &this->game_object_manager_
    );

    // TODO(m4jr): Remove the line below when the time comes.
    this->rendering_frame_counter_++;
  }
}

void Game::ReceiveEvent(std::string event) {
  // TODO(m4jr): Remove both of the frame counters when the time comes.
  std::cout << "PHYSICS " << this->physics_frame_counter_ << std::endl;
  std::cout << "RENDERING " << this->rendering_frame_counter_ << std::endl;

  this->physics_frame_counter_ = 0;
  this->rendering_frame_counter_ = 0;
}
};  //  namespace koma
