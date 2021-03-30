// Copyright 2021 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "tests_game.hpp"

#include <memory>

#include "core/game.hpp"

namespace comettests {
const double StopComponent::kTimeDelta = 500;  // 0.5 seconds.

void StopComponent::Update() {
  auto now = comet::Game::game()->time_manager()->GetRealNow();

  if (now - starting_time_ > kTimeDelta) {
    comet::Game::game()->game()->Quit();
  }
}

void StopComponent::Initialize() {
  starting_time_ = comet::Game::game()->time_manager()->GetRealNow();
}
};  // namespace comettests

TEST_CASE("Game state handling", "[comet::Game]") {
  auto game = comet::Game::game();
  game->Initialize();

  SECTION("Game runs, then quits after.") {
    auto game_object = comet::GameObject::Create();
    auto component = std::make_shared<comettests::StopComponent>();

    game->game_object_manager()->AddGameObject(game_object);
    game_object->AddComponent(component);

    game->Run();

    REQUIRE(game->is_running() == false);
  }
}
