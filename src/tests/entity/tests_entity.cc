// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "tests_entity.h"

#include "catch.hpp"

namespace comet {
namespace comettests {
entity::EntityManager entity_manager{};

const entity::ComponentTypeId DummyEmptyComponent::kComponentTypeId{
    COMET_STRING_ID("dummy_empty_component")};

const entity::ComponentTypeId DummyMeshComponent::kComponentTypeId{
    COMET_STRING_ID("dummy_mesh_component")};

const entity::ComponentTypeId DummyTransformComponent::kComponentTypeId{
    COMET_STRING_ID("dummy_transform_component")};

const entity::ComponentTypeId DummyHpComponent::kComponentTypeId{
    COMET_STRING_ID("dummy_hp_component")};
}  // namespace comettests
}  // namespace comet

TEST_CASE("Components management", "[comet::entity]") {
  auto& entity_manager{comet::comettests::entity_manager};
  entity_manager.Initialize();
  const comet::entity::EntityId entity_id1{entity_manager.CreateEntity()};
  const comet::entity::EntityId entity_id2{entity_manager.CreateEntity()};
  const comet::entity::EntityId entity_id3{entity_manager.CreateEntity()};

  SECTION("Create operations.") {
    entity_manager.AddComponent<comet::comettests::DummyEmptyComponent>(
        entity_id1);

    REQUIRE(entity_manager.GetComponent<comet::comettests::DummyEmptyComponent>(
                entity_id1) != nullptr);

    entity_manager.AddComponent<comet::comettests::DummyMeshComponent>(
        entity_id1);

    REQUIRE(entity_manager.GetComponent<comet::comettests::DummyEmptyComponent>(
                entity_id1) != nullptr);

    REQUIRE(entity_manager.GetComponent<comet::comettests::DummyMeshComponent>(
                entity_id1) != nullptr);

    entity_manager.AddComponents(entity_id1,
                                 comet::comettests::DummyTransformComponent{},
                                 comet::comettests::DummyHpComponent{});

    REQUIRE(entity_manager.GetComponent<comet::comettests::DummyEmptyComponent>(
                entity_id1) != nullptr);

    REQUIRE(entity_manager.GetComponent<comet::comettests::DummyMeshComponent>(
                entity_id1) != nullptr);

    REQUIRE(
        entity_manager.GetComponent<comet::comettests::DummyTransformComponent>(
            entity_id1) != nullptr);

    REQUIRE(entity_manager.GetComponent<comet::comettests::DummyHpComponent>(
                entity_id1) != nullptr);

    entity_manager.AddComponent<comet::comettests::DummyEmptyComponent>(
        entity_id2);

    REQUIRE(entity_manager.GetComponent<comet::comettests::DummyEmptyComponent>(
                entity_id2) != nullptr);

    entity_manager.AddComponent<comet::comettests::DummyMeshComponent>(
        entity_id2);

    REQUIRE(entity_manager.GetComponent<comet::comettests::DummyEmptyComponent>(
                entity_id2) != nullptr);

    REQUIRE(entity_manager.GetComponent<comet::comettests::DummyMeshComponent>(
                entity_id2) != nullptr);

    entity_manager.AddComponents(entity_id2,
                                 comet::comettests::DummyTransformComponent{},
                                 comet::comettests::DummyHpComponent{});

    REQUIRE(entity_manager.GetComponent<comet::comettests::DummyEmptyComponent>(
                entity_id2) != nullptr);

    REQUIRE(entity_manager.GetComponent<comet::comettests::DummyMeshComponent>(
                entity_id2) != nullptr);

    REQUIRE(
        entity_manager.GetComponent<comet::comettests::DummyTransformComponent>(
            entity_id2) != nullptr);

    REQUIRE(entity_manager.GetComponent<comet::comettests::DummyHpComponent>(
                entity_id2) != nullptr);

    entity_manager.AddComponent<comet::comettests::DummyEmptyComponent>(
        entity_id3);

    REQUIRE(entity_manager.GetComponent<comet::comettests::DummyEmptyComponent>(
                entity_id3) != nullptr);

    entity_manager.AddComponent<comet::comettests::DummyMeshComponent>(
        entity_id3);

    REQUIRE(entity_manager.GetComponent<comet::comettests::DummyEmptyComponent>(
                entity_id3) != nullptr);

    REQUIRE(entity_manager.GetComponent<comet::comettests::DummyMeshComponent>(
                entity_id3) != nullptr);

    entity_manager.AddComponents(entity_id3,
                                 comet::comettests::DummyTransformComponent{},
                                 comet::comettests::DummyHpComponent{});

    REQUIRE(entity_manager.GetComponent<comet::comettests::DummyEmptyComponent>(
                entity_id3) != nullptr);

    REQUIRE(entity_manager.GetComponent<comet::comettests::DummyMeshComponent>(
                entity_id3) != nullptr);

    REQUIRE(
        entity_manager.GetComponent<comet::comettests::DummyTransformComponent>(
            entity_id3) != nullptr);

    REQUIRE(entity_manager.GetComponent<comet::comettests::DummyHpComponent>(
                entity_id3) != nullptr);
  }

  SECTION("Remove operations.") {
    entity_manager.AddComponent<comet::comettests::DummyEmptyComponent>(
        entity_id1);

    entity_manager.RemoveComponent<comet::comettests::DummyEmptyComponent>(
        entity_id1);

    REQUIRE(entity_manager.GetComponent<comet::comettests::DummyEmptyComponent>(
                entity_id1) == nullptr);

    entity_manager.AddComponents(entity_id1,
                                 comet::comettests::DummyTransformComponent{},
                                 comet::comettests::DummyHpComponent{});

    entity_manager.RemoveComponent<comet::comettests::DummyTransformComponent>(
        entity_id1);

    REQUIRE(
        entity_manager.GetComponent<comet::comettests::DummyTransformComponent>(
            entity_id1) == nullptr);

    REQUIRE(entity_manager.GetComponent<comet::comettests::DummyHpComponent>(
                entity_id1) != nullptr);

    entity_manager.AddComponent<comet::comettests::DummyTransformComponent>(
        entity_id1);

    entity_manager.RemoveComponents<comet::comettests::DummyTransformComponent,
                                    comet::comettests::DummyHpComponent>(
        entity_id1);

    REQUIRE(
        entity_manager.GetComponent<comet::comettests::DummyTransformComponent>(
            entity_id1) == nullptr);

    REQUIRE(entity_manager.GetComponent<comet::comettests::DummyHpComponent>(
                entity_id1) == nullptr);
  }

  SECTION("Destroy operations.") {
    REQUIRE(entity_manager.IsEntity(entity_id1));
    REQUIRE(entity_manager.IsEntity(entity_id2));
    REQUIRE(entity_manager.IsEntity(entity_id3));

    entity_manager.DestroyEntity(entity_id1);

    REQUIRE(!entity_manager.IsEntity(entity_id1));
    REQUIRE(entity_manager.IsEntity(entity_id2));
    REQUIRE(entity_manager.IsEntity(entity_id3));

    entity_manager.DestroyEntity(entity_id2);

    REQUIRE(!entity_manager.IsEntity(entity_id1));
    REQUIRE(!entity_manager.IsEntity(entity_id2));
    REQUIRE(entity_manager.IsEntity(entity_id3));

    entity_manager.DestroyEntity(entity_id3);

    REQUIRE(!entity_manager.IsEntity(entity_id1));
    REQUIRE(!entity_manager.IsEntity(entity_id2));
    REQUIRE(!entity_manager.IsEntity(entity_id3));
  }

  SECTION("View operations.") {
    bool is_entity_id1{false};
    bool is_entity_id2{false};
    bool is_entity_id3{false};

    entity_manager.AddComponents(entity_id1,
                                 comet::comettests::DummyTransformComponent{},
                                 comet::comettests::DummyHpComponent{});

    entity_manager.AddComponent<comet::comettests::DummyTransformComponent>(
        entity_id2);

    const auto non_empty_view1{
        entity_manager.GetView<comet::comettests::DummyTransformComponent,
                               comet::comettests::DummyHpComponent>()};

    for (const auto entity_id : non_empty_view1) {
      REQUIRE(entity_manager
                  .GetComponent<comet::comettests::DummyTransformComponent>(
                      entity_id) != nullptr);

      REQUIRE(entity_manager.GetComponent<comet::comettests::DummyHpComponent>(
                  entity_id) != nullptr);

      if (entity_id == entity_id1) {
        is_entity_id1 = true;
      }

      if (entity_id == entity_id2) {
        is_entity_id2 = true;
      }

      if (entity_id == entity_id3) {
        is_entity_id3 = true;
      }
    }

    const auto non_empty_view2{
        entity_manager.GetView<comet::comettests::DummyTransformComponent>()};

    REQUIRE(is_entity_id1);
    REQUIRE(!is_entity_id2);
    REQUIRE(!is_entity_id3);

    is_entity_id1 = false;
    is_entity_id2 = false;
    is_entity_id3 = false;

    for (const auto entity_id : non_empty_view2) {
      REQUIRE(entity_manager
                  .GetComponent<comet::comettests::DummyTransformComponent>(
                      entity_id) != nullptr);

      if (entity_id == entity_id1) {
        is_entity_id1 = true;
      }

      if (entity_id == entity_id2) {
        is_entity_id2 = true;
      }

      if (entity_id == entity_id3) {
        is_entity_id3 = true;
      }
    }

    REQUIRE(is_entity_id1);
    REQUIRE(is_entity_id2);
    REQUIRE(!is_entity_id3);

    is_entity_id1 = false;
    is_entity_id2 = false;
    is_entity_id3 = false;

    const auto empty_view{
        entity_manager.GetView<comet::comettests::DummyMeshComponent>()};

    for (const auto entity_id : empty_view) {
      if (entity_id == entity_id1) {
        is_entity_id1 = true;
      }

      if (entity_id == entity_id2) {
        is_entity_id2 = true;
      }

      if (entity_id == entity_id3) {
        is_entity_id3 = true;
      }
    }

    REQUIRE(!is_entity_id1);
    REQUIRE(!is_entity_id2);
    REQUIRE(!is_entity_id3);

    const auto all_view{entity_manager.GetView()};

    for (const auto entity_id : all_view) {
      if (entity_id == entity_id1) {
        is_entity_id1 = true;
      }

      if (entity_id == entity_id2) {
        is_entity_id2 = true;
      }

      if (entity_id == entity_id3) {
        is_entity_id3 = true;
      }
    }

    REQUIRE(is_entity_id1);
    REQUIRE(is_entity_id2);
    REQUIRE(is_entity_id3);
  }

  entity_manager.Destroy();
}