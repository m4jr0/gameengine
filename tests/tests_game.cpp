// Copyright 2018 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "tests_game.hpp"

namespace komatests {
const double StopComponent::TIME_DELTA = 2000;  // 2 seconds.

void StopComponent::Update(double interpolation) {
  koma::TimeManager &time_manager = koma::Locator::time_manager();
  double now = time_manager.GetRealNow();

  if (now - this->starting_time_ > StopComponent::TIME_DELTA) {
    koma::Locator::game().Quit();
  }
}

void StopComponent::Initialize() {
  this->starting_time_ = koma::Locator::time_manager().GetRealNow();
}
};  // namespace komatests

TEST_CASE("Game state handling", "[koma::Game]") {
  koma::Game game = koma::Game();

  SECTION("Game can run, then quit after.") {
    koma::GameObject* game_object = new koma::GameObject();
    koma::Component* component = new komatests::StopComponent();

    koma::Locator::game_object_manager().AddGameObject(game_object);
    game_object->AddComponent(component);

    game.Run();

    REQUIRE(game.is_running() == false);
  }
}