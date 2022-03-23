// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "tests_game_object.h"

#include "boost/algorithm/string/find.hpp"
#include "catch.hpp"
#include "comet/game_object/component.h"
#include "comet/game_object/game_object.h"
#include "comet_precompile.h"

namespace comettests {
DummyComponent::DummyComponent(int value) : dummy_object_(value) {}

DummyComponent::DummyComponent(const DummyComponent& other)
    : comet::game_object::Component(other),
      dummy_object_(other.dummy_object_) {}

DummyComponent::DummyComponent(DummyComponent&& other) noexcept
    : comet::game_object::Component(other),
      dummy_object_(std::move(other.dummy_object_)) {}

DummyComponent& DummyComponent::operator=(const DummyComponent& other) {
  if (this == &other) {
    return *this;
  }

  Component::operator=(other);
  dummy_object_ = other.dummy_object_;
  return *this;
}

DummyComponent& DummyComponent::operator=(DummyComponent&& other) noexcept {
  if (this == &other) {
    return *this;
  }

  Component::operator=(std::move(other));
  dummy_object_ = std::move(other.dummy_object_);
  return *this;
}

std::shared_ptr<comet::game_object::Component> DummyComponent::Clone() const {
  return std::make_shared<DummyComponent>(*this);
}

int DummyComponent::GetValue() const { return dummy_object_.GetValue(); }
}  // namespace comettests

TEST_CASE("Game object initialization", "[comet::game_object]") {
  SECTION("A cloned game object does not share the same components.") {
    const auto game_object = comet::game_object::GameObject::Create();
    auto dummy_component = std::make_shared<comettests::DummyComponent>(1);

    game_object->AddComponent(dummy_component);

    const auto clone = game_object->Clone();
    const auto original_addr =
        &game_object->GetComponent<comettests::DummyComponent>();
    const auto copy_addr = &clone->GetComponent<comettests::DummyComponent>();

    REQUIRE(original_addr != copy_addr);
  }
}
