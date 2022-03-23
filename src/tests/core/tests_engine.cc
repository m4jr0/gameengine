// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "tests_engine.h"

namespace comettests {
const double StopComponent::kTimeDelta = 500;  // 0.5 seconds.

StopComponent::StopComponent(const StopComponent& other)
    : comet::game_object::Component(other) {}

StopComponent::StopComponent(StopComponent&& other) noexcept
    : comet::game_object::Component(other) {}

StopComponent& StopComponent::operator=(const StopComponent& other) {
  if (this == &other) {
    return *this;
  }

  Component::operator=(other);
  return *this;
}

StopComponent& StopComponent::operator=(StopComponent&& other) noexcept {
  if (this == &other) {
    return *this;
  }

  Component::operator=(std::move(other));
  return *this;
}

void StopComponent::Update() {
  const auto now =
      comet::core::Engine::GetEngine().GetTimeManager().GetRealNow();

  if (now - starting_time_ > kTimeDelta) {
    comet::core::Engine::GetEngine().Quit();
  }
}

std::shared_ptr<comet::game_object::Component> StopComponent::Clone() const {
  return std::make_shared<StopComponent>(*this);
}

void StopComponent::Initialize() {
  starting_time_ =
      comet::core::Engine::GetEngine().GetTimeManager().GetRealNow();
}
};  // namespace comettests

TEST_CASE("Game state handling", "[comet::core::Engine]") {
  auto game = comettests::CometTester();
  game.Initialize();

  SECTION("Game runs, then quits after.") {
    auto game_object = comet::game_object::GameObject::Create();
    auto component = std::make_shared<comettests::StopComponent>();

    game.GetGameObjectManager().AddGameObject(game_object);
    game_object->AddComponent(component);
    game.Run();

    REQUIRE(game.is_running() == false);
  }
}
