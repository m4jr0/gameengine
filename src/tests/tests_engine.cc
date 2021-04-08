// Copyright 2021 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "tests_engine.h"

namespace comettests {
const double StopComponent::kTimeDelta = 500;  // 0.5 seconds.

CometTester::CometTester() : comet::core::Engine() {}

void StopComponent::Update() {
  auto now = comet::core::Engine::engine()->time_manager()->GetRealNow();

  if (now - starting_time_ > kTimeDelta) {
    comet::core::Engine::engine()->engine()->Quit();
  }
}

void StopComponent::Initialize() {
  starting_time_ = comet::core::Engine::engine()->time_manager()->GetRealNow();
}
};  // namespace comettests

TEST_CASE("Game state handling", "[comet::core::Engine]") {
  auto game = comettests::CometTester();
  game.Initialize();

  SECTION("Game runs, then quits after.") {
    auto game_object = comet::game_object::GameObject::Create();
    auto component = std::make_shared<comettests::StopComponent>();

    game.game_object_manager()->AddGameObject(game_object);
    game_object->AddComponent(component);

    game.Run();

    REQUIRE(game.is_running() == false);
  }
}
