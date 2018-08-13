// Copyright 2018 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "tests_game.hpp"

#include <memory>

#include "../koma/core/locator/locator.hpp"

namespace komatests {
const double StopComponent::kTimeDelta = 2000;  // 2 seconds.

void StopComponent::Update() {
  koma::TimeManager &time_manager = koma::Locator::time_manager();
  double now = time_manager.GetRealNow();

  if (now - this->starting_time_ > this->kTimeDelta) {
    koma::Locator::game().Quit();
  }
}

void StopComponent::Initialize() {
  this->starting_time_ = koma::Locator::time_manager().GetRealNow();
}
};  // namespace komatests

TEST_CASE("Game state handling", "[koma::Game]") {
  koma::Game game = koma::Game();
  game.Initialize();

  SECTION("Game can run, then quit after.") {
    auto game_object = std::make_shared<koma::GameObject>();
    auto component = std::make_shared<komatests::StopComponent>();

    koma::Locator::game_object_manager().AddGameObject(game_object);
    game_object->AddComponent(component);

    game.Run();

    REQUIRE(game.is_running() == false);
  }
}
