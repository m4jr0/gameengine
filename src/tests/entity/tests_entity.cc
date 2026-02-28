// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "tests_entity.h"
////////////////////////////////////////////////////////////////////////////////

// Tested. /////////////////////////////////////////////////////////////////////
#include "comet/entity/entity_manager.h"
////////////////////////////////////////////////////////////////////////////////

// External. ///////////////////////////////////////////////////////////////////
#include "catch.hpp"
////////////////////////////////////////////////////////////////////////////////

TEST_CASE("Components management", "[comet::entity]") {
  auto& entity_manager{comet::entity::EntityManager::Get()};
  const comet::entity::EntityId entity_id1{entity_manager.Generate()};
  const comet::entity::EntityId entity_id2{entity_manager.Generate()};
  const comet::entity::EntityId entity_id3{entity_manager.Generate()};

  SECTION("Create operations.") {
    entity_manager.AddComponents(entity_id1,
                                 comet::comettests::DummyTagComponent{});

    entity_manager.DispatchComponentChanges();

    REQUIRE(entity_manager.GetComponent<comet::comettests::DummyTagComponent>(
                entity_id1) != nullptr);

    entity_manager.AddComponents(entity_id1,
                                 comet::comettests::DummyMeshComponent{});

    entity_manager.DispatchComponentChanges();

    REQUIRE(entity_manager.GetComponent<comet::comettests::DummyTagComponent>(
                entity_id1) != nullptr);

    REQUIRE(entity_manager.GetComponent<comet::comettests::DummyMeshComponent>(
                entity_id1) != nullptr);

    entity_manager.AddComponents(entity_id1,
                                 comet::comettests::DummyTransformComponent{},
                                 comet::comettests::DummyHpComponent{});

    entity_manager.DispatchComponentChanges();

    REQUIRE(entity_manager.GetComponent<comet::comettests::DummyTagComponent>(
                entity_id1) != nullptr);

    REQUIRE(entity_manager.GetComponent<comet::comettests::DummyMeshComponent>(
                entity_id1) != nullptr);

    REQUIRE(
        entity_manager.GetComponent<comet::comettests::DummyTransformComponent>(
            entity_id1) != nullptr);

    REQUIRE(entity_manager.GetComponent<comet::comettests::DummyHpComponent>(
                entity_id1) != nullptr);

    entity_manager.AddComponents(entity_id2,
                                 comet::comettests::DummyTagComponent{});

    entity_manager.DispatchComponentChanges();

    REQUIRE(entity_manager.GetComponent<comet::comettests::DummyTagComponent>(
                entity_id2) != nullptr);

    entity_manager.AddComponents(entity_id2,
                                 comet::comettests::DummyMeshComponent{});

    entity_manager.DispatchComponentChanges();

    REQUIRE(entity_manager.GetComponent<comet::comettests::DummyTagComponent>(
                entity_id2) != nullptr);

    REQUIRE(entity_manager.GetComponent<comet::comettests::DummyMeshComponent>(
                entity_id2) != nullptr);

    entity_manager.AddComponents(entity_id2,
                                 comet::comettests::DummyTransformComponent{},
                                 comet::comettests::DummyHpComponent{});

    entity_manager.DispatchComponentChanges();

    REQUIRE(entity_manager.GetComponent<comet::comettests::DummyTagComponent>(
                entity_id2) != nullptr);

    REQUIRE(entity_manager.GetComponent<comet::comettests::DummyMeshComponent>(
                entity_id2) != nullptr);

    REQUIRE(
        entity_manager.GetComponent<comet::comettests::DummyTransformComponent>(
            entity_id2) != nullptr);

    REQUIRE(entity_manager.GetComponent<comet::comettests::DummyHpComponent>(
                entity_id2) != nullptr);

    entity_manager.AddComponents(entity_id3,
                                 comet::comettests::DummyTagComponent{});

    entity_manager.DispatchComponentChanges();

    REQUIRE(entity_manager.GetComponent<comet::comettests::DummyTagComponent>(
                entity_id3) != nullptr);

    entity_manager.AddComponents(entity_id3,
                                 comet::comettests::DummyMeshComponent{});

    entity_manager.DispatchComponentChanges();

    REQUIRE(entity_manager.GetComponent<comet::comettests::DummyTagComponent>(
                entity_id3) != nullptr);

    REQUIRE(entity_manager.GetComponent<comet::comettests::DummyMeshComponent>(
                entity_id3) != nullptr);

    entity_manager.AddComponents(entity_id3,
                                 comet::comettests::DummyTransformComponent{},
                                 comet::comettests::DummyHpComponent{});

    entity_manager.DispatchComponentChanges();

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
    entity_manager.AddComponents(entity_id1,
                                 comet::comettests::DummyTagComponent{});

    entity_manager.RemoveComponents<comet::comettests::DummyTagComponent>(
        entity_id1);

    entity_manager.DispatchComponentChanges();

    REQUIRE(entity_manager.GetComponent<comet::comettests::DummyTagComponent>(
                entity_id1) == nullptr);

    entity_manager.AddComponents(entity_id1,
                                 comet::comettests::DummyTransformComponent{},
                                 comet::comettests::DummyHpComponent{});

    entity_manager.RemoveComponents<comet::comettests::DummyTransformComponent>(
        entity_id1);

    entity_manager.DispatchComponentChanges();

    REQUIRE(
        entity_manager.GetComponent<comet::comettests::DummyTransformComponent>(
            entity_id1) == nullptr);

    REQUIRE(entity_manager.GetComponent<comet::comettests::DummyHpComponent>(
                entity_id1) != nullptr);

    entity_manager.AddComponents(entity_id1,
                                 comet::comettests::DummyTransformComponent{});

    entity_manager.RemoveComponents<comet::comettests::DummyTransformComponent,
                                    comet::comettests::DummyHpComponent>(
        entity_id1);

    entity_manager.DispatchComponentChanges();

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

    entity_manager.DispatchComponentChanges();

    REQUIRE(!entity_manager.IsEntity(entity_id1));
    REQUIRE(entity_manager.IsEntity(entity_id2));
    REQUIRE(entity_manager.IsEntity(entity_id3));

    entity_manager.Destroy(entity_id2);

    entity_manager.DispatchComponentChanges();

    REQUIRE(!entity_manager.IsEntity(entity_id1));
    REQUIRE(!entity_manager.IsEntity(entity_id2));
    REQUIRE(entity_manager.IsEntity(entity_id3));

    entity_manager.Destroy(entity_id3);

    entity_manager.DispatchComponentChanges();

    REQUIRE(!entity_manager.IsEntity(entity_id1));
    REQUIRE(!entity_manager.IsEntity(entity_id2));
    REQUIRE(!entity_manager.IsEntity(entity_id3));
  }

  SECTION("View operations.") {
    auto is_entity_id1{false};
    auto is_entity_id2{false};
    auto is_entity_id3{false};

    entity_manager.AddComponents(entity_id1,
                                 comet::comettests::DummyTransformComponent{},
                                 comet::comettests::DummyHpComponent{});

    entity_manager.AddComponents(entity_id2,
                                 comet::comettests::DummyTransformComponent{});

    entity_manager.DispatchComponentChanges();

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
}
