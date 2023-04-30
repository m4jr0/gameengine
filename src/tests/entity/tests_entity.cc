// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "tests_entity.h"

#include "comet/entity/entity_component.h"
#include "comet/entity/entity_manager.h"

#include "catch.hpp"

namespace comet {
namespace comettests {
entity::EntityManager entity_manager{{}};
}  // namespace comettests
}  // namespace comet

TEST_CASE("Components management", "[comet::entity]") {
  auto& entity_manager{comet::comettests::entity_manager};
  entity_manager.Initialize();
  const comet::entity::EntityId entity_id1{entity_manager.Generate()};
  const comet::entity::EntityId entity_id2{entity_manager.Generate()};
  const comet::entity::EntityId entity_id3{entity_manager.Generate()};

  auto entity_cmp_gen1{comet::entity::EntityComponentGenerator::Get(
      &entity_manager, entity_id1)};
  auto entity_cmp_gen2{comet::entity::EntityComponentGenerator::Get(
      &entity_manager, entity_id2)};
  auto entity_cmp_gen3{comet::entity::EntityComponentGenerator::Get(
      &entity_manager, entity_id3)};

  auto entity_cmp_des1{comet::entity::EntityComponentDestroyer::Get(
      &entity_manager, entity_id1)};

  SECTION("Create operations.") {
    entity_cmp_gen1.AddComponent(comet::comettests::DummyTagComponent{})
        .Submit();

    REQUIRE(entity_manager.GetComponent<comet::comettests::DummyTagComponent>(
                entity_id1) != nullptr);

    entity_cmp_gen1.AddComponent(comet::comettests::DummyMeshComponent{})
        .Submit();

    REQUIRE(entity_manager.GetComponent<comet::comettests::DummyTagComponent>(
                entity_id1) != nullptr);

    REQUIRE(entity_manager.GetComponent<comet::comettests::DummyMeshComponent>(
                entity_id1) != nullptr);

    entity_cmp_gen1.AddComponent(comet::comettests::DummyTransformComponent{})
        .AddComponent(comet::comettests::DummyHpComponent{})
        .Submit();

    REQUIRE(entity_manager.GetComponent<comet::comettests::DummyTagComponent>(
                entity_id1) != nullptr);

    REQUIRE(entity_manager.GetComponent<comet::comettests::DummyMeshComponent>(
                entity_id1) != nullptr);

    REQUIRE(
        entity_manager.GetComponent<comet::comettests::DummyTransformComponent>(
            entity_id1) != nullptr);

    REQUIRE(entity_manager.GetComponent<comet::comettests::DummyHpComponent>(
                entity_id1) != nullptr);

    entity_cmp_gen2.AddComponent(comet::comettests::DummyTagComponent{})
        .Submit();

    REQUIRE(entity_manager.GetComponent<comet::comettests::DummyTagComponent>(
                entity_id2) != nullptr);

    entity_cmp_gen2.AddComponent(comet::comettests::DummyMeshComponent{})
        .Submit();

    REQUIRE(entity_manager.GetComponent<comet::comettests::DummyTagComponent>(
                entity_id2) != nullptr);

    REQUIRE(entity_manager.GetComponent<comet::comettests::DummyMeshComponent>(
                entity_id2) != nullptr);

    entity_cmp_gen2.AddComponent(comet::comettests::DummyTransformComponent{})
        .AddComponent(comet::comettests::DummyHpComponent{})
        .Submit();

    REQUIRE(entity_manager.GetComponent<comet::comettests::DummyTagComponent>(
                entity_id2) != nullptr);

    REQUIRE(entity_manager.GetComponent<comet::comettests::DummyMeshComponent>(
                entity_id2) != nullptr);

    REQUIRE(
        entity_manager.GetComponent<comet::comettests::DummyTransformComponent>(
            entity_id2) != nullptr);

    REQUIRE(entity_manager.GetComponent<comet::comettests::DummyHpComponent>(
                entity_id2) != nullptr);

    entity_cmp_gen3.AddComponent(comet::comettests::DummyTagComponent{})
        .Submit();

    REQUIRE(entity_manager.GetComponent<comet::comettests::DummyTagComponent>(
                entity_id3) != nullptr);

    entity_cmp_gen3.AddComponent(comet::comettests::DummyMeshComponent{})
        .Submit();

    REQUIRE(entity_manager.GetComponent<comet::comettests::DummyTagComponent>(
                entity_id3) != nullptr);

    REQUIRE(entity_manager.GetComponent<comet::comettests::DummyMeshComponent>(
                entity_id3) != nullptr);

    entity_cmp_gen3.AddComponent(comet::comettests::DummyTransformComponent{})
        .AddComponent(comet::comettests::DummyHpComponent{})
        .Submit();

    REQUIRE(entity_manager.GetComponent<comet::comettests::DummyTagComponent>(
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
    entity_cmp_gen1.AddComponent(comet::comettests::DummyTagComponent{})
        .Submit();

    entity_cmp_des1.RemoveComponent<comet::comettests::DummyTagComponent>()
        .Submit();

    REQUIRE(entity_manager.GetComponent<comet::comettests::DummyTagComponent>(
                entity_id1) == nullptr);

    entity_cmp_gen1.AddComponent(comet::comettests::DummyTransformComponent{})
        .AddComponent(comet::comettests::DummyHpComponent{})
        .Submit();

    entity_cmp_des1
        .RemoveComponent<comet::comettests::DummyTransformComponent>()
        .Submit();

    REQUIRE(
        entity_manager.GetComponent<comet::comettests::DummyTransformComponent>(
            entity_id1) == nullptr);

    REQUIRE(entity_manager.GetComponent<comet::comettests::DummyHpComponent>(
                entity_id1) != nullptr);

    entity_cmp_gen1.AddComponent(comet::comettests::DummyTransformComponent{})
        .Submit();

    entity_cmp_des1
        .RemoveComponent<comet::comettests::DummyTransformComponent>()
        .RemoveComponent<comet::comettests::DummyHpComponent>()
        .Submit();

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

    entity_manager.Destroy(entity_id1);

    REQUIRE(!entity_manager.IsEntity(entity_id1));
    REQUIRE(entity_manager.IsEntity(entity_id2));
    REQUIRE(entity_manager.IsEntity(entity_id3));

    entity_manager.Destroy(entity_id2);

    REQUIRE(!entity_manager.IsEntity(entity_id1));
    REQUIRE(!entity_manager.IsEntity(entity_id2));
    REQUIRE(entity_manager.IsEntity(entity_id3));

    entity_manager.Destroy(entity_id3);

    REQUIRE(!entity_manager.IsEntity(entity_id1));
    REQUIRE(!entity_manager.IsEntity(entity_id2));
    REQUIRE(!entity_manager.IsEntity(entity_id3));
  }

  SECTION("View operations.") {
    auto is_entity_id1{false};
    auto is_entity_id2{false};
    auto is_entity_id3{false};

    entity_cmp_gen1.AddComponent(comet::comettests::DummyTransformComponent{})
        .AddComponent(comet::comettests::DummyHpComponent{})
        .Submit();

    entity_cmp_gen2.AddComponent(comet::comettests::DummyTransformComponent{})
        .Submit();

    entity_manager.Each<comet::comettests::DummyTransformComponent,
                        comet::comettests::DummyHpComponent>(
        [&](auto entity_id) {
          REQUIRE(entity_manager
                      .GetComponent<comet::comettests::DummyTransformComponent>(
                          entity_id) != nullptr);

          REQUIRE(
              entity_manager.GetComponent<comet::comettests::DummyHpComponent>(
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
        });

    REQUIRE(is_entity_id1);
    REQUIRE(!is_entity_id2);
    REQUIRE(!is_entity_id3);

    is_entity_id1 = false;
    is_entity_id2 = false;
    is_entity_id3 = false;

    entity_manager.Each<comet::comettests::DummyTransformComponent>(
        [&](auto entity_id) {
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
        });

    REQUIRE(is_entity_id1);
    REQUIRE(is_entity_id2);
    REQUIRE(!is_entity_id3);

    is_entity_id1 = false;
    is_entity_id2 = false;
    is_entity_id3 = false;

    entity_manager.Each<comet::comettests::DummyMeshComponent>(
        [&](auto entity_id) {
          if (entity_id == entity_id1) {
            is_entity_id1 = true;
          }

          if (entity_id == entity_id2) {
            is_entity_id2 = true;
          }

          if (entity_id == entity_id3) {
            is_entity_id3 = true;
          }
        });

    REQUIRE(!is_entity_id1);
    REQUIRE(!is_entity_id2);
    REQUIRE(!is_entity_id3);

    entity_manager.Each([&](auto entity_id) {
      if (entity_id == entity_id1) {
        is_entity_id1 = true;
      }

      if (entity_id == entity_id2) {
        is_entity_id2 = true;
      }

      if (entity_id == entity_id3) {
        is_entity_id3 = true;
      }
    });

    REQUIRE(is_entity_id1);
    REQUIRE(is_entity_id2);
    REQUIRE(is_entity_id3);
  }

  entity_manager.Shutdown();
}
