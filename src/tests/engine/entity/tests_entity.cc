// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "tests_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "tests_entity.h"
////////////////////////////////////////////////////////////////////////////////

// Tested. /////////////////////////////////////////////////////////////////////
#include "comet/runtime/entity/entity_manager.h"
#include "comet/runtime/entity/entity_memory_context.h"
////////////////////////////////////////////////////////////////////////////////

// External. ///////////////////////////////////////////////////////////////////
#include "catch.hpp"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/fiber/fiber.h"
#include "comet/core/job/job.h"
#include "comet/core/job/job_utils.h"
#include "comet/core/job/scheduler.h"
#include "comet/runtime/frame/frame_container.h"

namespace comet {
namespace comettests {
namespace {
using comet::entity::EntityId;
using comet::entity::EntityManager;

template <typename ComponentType>
bool Has(EntityId id) {
  return EntityManager::Get().GetComponent<ComponentType>(id) != nullptr;
}

usize CountAllEntities() {
  usize count{0};
  EntityManager::Get().ForEachId([&](EntityId) { ++count; });
  return count;
}

template <typename... ComponentTypes>
usize CountView() {
  usize count{0};
  EntityManager::Get().ForEach<ComponentTypes...>(
      [&](EntityId, auto&...) { ++count; });
  return count;
}

template <typename... ComponentTypes>
usize CountIdView() {
  usize count{0};
  EntityManager::Get().ForEachId<ComponentTypes...>([&](EntityId) { ++count; });
  return count;
}

void FlushAndRequireNoPending() {
  auto& em{EntityManager::Get()};
  em.Flush();
  REQUIRE(!em.HasPendingStructuralChanges());
}

struct EntityScope {
  Array<EntityId> ids{Array<EntityId>::WithCapacity(
      &entity::EntityMemoryContext::Get().GetEntityIdAllocator(), 64)};

  ~EntityScope() {
    auto& em{EntityManager::Get()};

    for (const auto id : ids) {
      if (em.IsEntity(id)) {
        em.Destroy(id);
      }
    }

    em.Flush();

    COMET_ASSERT(
        em.GetEntityCount() == 0, "comettests::EntityScope::~EntityScope",
        "entity count is not zero", "entity_count", em.GetEntityCount());
    COMET_ASSERT(em.GetPendingEntityCount() == 0,
                 "comettests::EntityScope::~EntityScope",
                 "pending entity count is not zero", "pending_entity_count",
                 em.GetPendingEntityCount());
  }

  EntityId Generate() {
    const auto id{EntityManager::Get().Generate()};
    ids.PushLast(id);
    return id;
  }
};
}  // namespace
}  // namespace comettests
}  // namespace comet

TEST_CASE("Entity pending structural changes", "[comet::entity]") {
  using namespace comet;
  using namespace comet::comettests;

  auto& em{entity::EntityManager::Get()};
  EntityScope scope{};
  const auto baseline_count{em.GetEntityCount()};

  SECTION("Generate without components commits an entity") {
    const auto e{scope.Generate()};

    REQUIRE(em.IsEntity(e));
    REQUIRE(em.HasPendingStructuralChanges());

    FlushAndRequireNoPending();

    REQUIRE(em.IsEntity(e));
    REQUIRE(em.GetEntityCount() == baseline_count + 1);
    REQUIRE(CountAllEntities() == em.GetEntityCount());
  }

  SECTION("Create entity with one component before first flush") {
    const auto e{scope.Generate()};

    em.AddComponents(e, DummyHpComponent{10, 1, 20, 2});
    FlushAndRequireNoPending();

    REQUIRE(Has<DummyHpComponent>(e));

    const auto* hp{em.GetComponent<DummyHpComponent>(e)};
    REQUIRE(hp->hit_points == 10);
    REQUIRE(hp->shield_points == 1);
    REQUIRE(hp->max_hit_points == 20);
    REQUIRE(hp->max_shield_points == 2);

    REQUIRE(em.GetEntityCount() == baseline_count + 1);
  }

  SECTION("Create entity with multiple components before first flush") {
    const auto e{scope.Generate()};

    em.AddComponents(e, DummyTransformComponent{}, DummyMeshComponent{},
                     DummyHpComponent{5, 6, 7, 8});
    FlushAndRequireNoPending();

    REQUIRE(Has<DummyTransformComponent>(e));
    REQUIRE(Has<DummyMeshComponent>(e));
    REQUIRE(Has<DummyHpComponent>(e));
    REQUIRE(em.GetEntityCount() == baseline_count + 1);
  }

  SECTION("Pending add then pending remove cancels component before flush") {
    const auto e{scope.Generate()};

    em.AddComponents(e, DummyHpComponent{1, 2, 3, 4});
    em.RemoveComponents<DummyHpComponent>(e);
    FlushAndRequireNoPending();

    REQUIRE(em.IsEntity(e));
    REQUIRE(!Has<DummyHpComponent>(e));
    REQUIRE(em.GetEntityCount() == baseline_count + 1);
  }

  SECTION("Pending remove then pending add keeps component before flush") {
    const auto e{scope.Generate()};

    em.AddComponents(e, DummyHpComponent{1, 2, 3, 4});
    FlushAndRequireNoPending();

    em.RemoveComponents<DummyHpComponent>(e);
    em.AddComponents(e, DummyHpComponent{9, 8, 7, 6});
    FlushAndRequireNoPending();

    REQUIRE(Has<DummyHpComponent>(e));

    const auto* hp{em.GetComponent<DummyHpComponent>(e)};
    REQUIRE(hp->hit_points == 9);
    REQUIRE(hp->shield_points == 8);
    REQUIRE(hp->max_hit_points == 7);
    REQUIRE(hp->max_shield_points == 6);
  }

  SECTION("Adding same pending component twice keeps latest data") {
    const auto e{scope.Generate()};

    em.AddComponents(e, DummyHpComponent{1, 1, 1, 1});
    em.AddComponents(e, DummyHpComponent{2, 3, 4, 5});
    FlushAndRequireNoPending();

    const auto* hp{em.GetComponent<DummyHpComponent>(e)};
    REQUIRE(hp != nullptr);
    REQUIRE(hp->hit_points == 2);
    REQUIRE(hp->shield_points == 3);
    REQUIRE(hp->max_hit_points == 4);
    REQUIRE(hp->max_shield_points == 5);
  }

  SECTION(
      "Adding already committed component updates data without changing "
      "count") {
    const auto e{scope.Generate()};

    em.AddComponents(e, DummyHpComponent{1, 1, 1, 1});
    FlushAndRequireNoPending();

    em.AddComponents(e, DummyHpComponent{9, 8, 7, 6});
    FlushAndRequireNoPending();

    REQUIRE(em.GetEntityCount() == baseline_count + 1);

    const auto* hp{em.GetComponent<DummyHpComponent>(e)};
    REQUIRE(hp != nullptr);
    REQUIRE(hp->hit_points == 9);
    REQUIRE(hp->shield_points == 8);
    REQUIRE(hp->max_hit_points == 7);
    REQUIRE(hp->max_shield_points == 6);
  }

  SECTION("Removing non-existing component is a no-op") {
    const auto e{scope.Generate()};

    em.AddComponents(e, DummyTransformComponent{});
    FlushAndRequireNoPending();

    em.RemoveComponents<DummyHpComponent>(e);
    em.RemoveComponents<DummyMeshComponent>(e);
    FlushAndRequireNoPending();

    REQUIRE(em.GetEntityCount() == baseline_count + 1);
    REQUIRE(Has<DummyTransformComponent>(e));
    REQUIRE(!Has<DummyHpComponent>(e));
    REQUIRE(!Has<DummyMeshComponent>(e));
  }

  SECTION("Move entity across archetypes by adding and removing components") {
    const auto e{scope.Generate()};

    em.AddComponents(e, DummyTransformComponent{}, DummyHpComponent{});
    FlushAndRequireNoPending();

    REQUIRE(Has<DummyTransformComponent>(e));
    REQUIRE(Has<DummyHpComponent>(e));

    em.AddComponents(e, DummyMeshComponent{});
    FlushAndRequireNoPending();

    REQUIRE(Has<DummyTransformComponent>(e));
    REQUIRE(Has<DummyHpComponent>(e));
    REQUIRE(Has<DummyMeshComponent>(e));

    em.RemoveComponents<DummyTransformComponent>(e);
    FlushAndRequireNoPending();

    REQUIRE(!Has<DummyTransformComponent>(e));
    REQUIRE(Has<DummyHpComponent>(e));
    REQUIRE(Has<DummyMeshComponent>(e));
    REQUIRE(em.GetEntityCount() == baseline_count + 1);
  }

  SECTION("Destroy pending-created entity before first flush") {
    const auto e{scope.Generate()};

    em.AddComponents(e, DummyTransformComponent{}, DummyHpComponent{});
    em.Destroy(e);
    FlushAndRequireNoPending();

    REQUIRE(!em.IsEntity(e));
    REQUIRE(em.GetEntityCount() == baseline_count);
  }

  SECTION("Destroy committed entity removes it from all views") {
    const auto e{scope.Generate()};

    em.AddComponents(e, DummyTransformComponent{}, DummyHpComponent{});
    FlushAndRequireNoPending();

    REQUIRE(em.GetEntityCount() == baseline_count + 1);

    em.Destroy(e);
    FlushAndRequireNoPending();

    REQUIRE(!em.IsEntity(e));
    REQUIRE(em.GetEntityCount() == baseline_count);
    REQUIRE(CountView<DummyTransformComponent, DummyHpComponent>() == 0);
  }

  SECTION("Multiple entities move out of same archetype in one flush") {
    constexpr usize kCount{64};
    Array<EntityId> ids{Array<EntityId>::WithCapacity(
        &entity::EntityMemoryContext::Get().GetEntityIdAllocator(), kCount)};

    for (usize i{0}; i < kCount; ++i) {
      const auto e{scope.Generate()};
      ids.PushLast(e);
      em.AddComponents(e, DummyTransformComponent{},
                       DummyHpComponent{static_cast<u16>(i), 0, 100, 0});
    }

    FlushAndRequireNoPending();

    for (usize i{0}; i < kCount; ++i) {
      if (i % 2 == 0) {
        em.AddComponents(ids[i], DummyMeshComponent{});
      } else {
        em.RemoveComponents<DummyHpComponent>(ids[i]);
      }
    }

    FlushAndRequireNoPending();

    for (usize i{0}; i < kCount; ++i) {
      REQUIRE(Has<DummyTransformComponent>(ids[i]));

      if (i % 2 == 0) {
        REQUIRE(Has<DummyHpComponent>(ids[i]));
        REQUIRE(Has<DummyMeshComponent>(ids[i]));
      } else {
        REQUIRE(!Has<DummyHpComponent>(ids[i]));
        REQUIRE(!Has<DummyMeshComponent>(ids[i]));
      }
    }

    REQUIRE(em.GetEntityCount() == baseline_count + kCount);
  }

  SECTION("Mixed create move destroy in same flush") {
    const auto stay{scope.Generate()};
    const auto move{scope.Generate()};
    const auto die{scope.Generate()};
    const auto born{scope.Generate()};

    em.AddComponents(stay, DummyTransformComponent{});
    em.AddComponents(move, DummyTransformComponent{}, DummyHpComponent{});
    em.AddComponents(die, DummyTransformComponent{}, DummyMeshComponent{});
    FlushAndRequireNoPending();

    em.AddComponents(move, DummyMeshComponent{});
    em.Destroy(die);
    em.AddComponents(born, DummyHpComponent{42, 0, 42, 0});
    FlushAndRequireNoPending();

    REQUIRE(em.IsEntity(stay));
    REQUIRE(em.IsEntity(move));
    REQUIRE(!em.IsEntity(die));
    REQUIRE(em.IsEntity(born));

    REQUIRE(Has<DummyTransformComponent>(stay));
    REQUIRE(Has<DummyTransformComponent>(move));
    REQUIRE(Has<DummyHpComponent>(move));
    REQUIRE(Has<DummyMeshComponent>(move));
    REQUIRE(Has<DummyHpComponent>(born));

    REQUIRE(em.GetEntityCount() == baseline_count + 3);
  }

  SECTION("Create mutate remove readd destroy before first flush") {
    const auto e{scope.Generate()};

    em.AddComponents(e, DummyHpComponent{1, 1, 1, 1});
    em.AddComponents(e, DummyHpComponent{2, 2, 2, 2});
    em.RemoveComponents<DummyHpComponent>(e);
    em.AddComponents(e, DummyHpComponent{3, 3, 3, 3});
    em.Destroy(e);

    FlushAndRequireNoPending();

    REQUIRE(!em.IsEntity(e));
    REQUIRE(em.GetEntityCount() == baseline_count);
  }

  SECTION("Committed entity value update and structural move in same flush") {
    const auto e{scope.Generate()};

    em.AddComponents(e, DummyTransformComponent{},
                     DummyHpComponent{1, 2, 3, 4});
    FlushAndRequireNoPending();

    em.AddComponents(e, DummyHpComponent{9, 8, 7, 6});
    em.AddComponents(e, DummyMeshComponent{});
    em.RemoveComponents<DummyTransformComponent>(e);

    FlushAndRequireNoPending();

    REQUIRE(!Has<DummyTransformComponent>(e));
    REQUIRE(Has<DummyHpComponent>(e));
    REQUIRE(Has<DummyMeshComponent>(e));

    const auto* hp{em.GetComponent<DummyHpComponent>(e)};
    REQUIRE(hp->hit_points == 9);
    REQUIRE(hp->shield_points == 8);
    REQUIRE(hp->max_hit_points == 7);
    REQUIRE(hp->max_shield_points == 6);
  }

  SECTION("Removing all components moves entity to root archetype") {
    const auto e{scope.Generate()};

    em.AddComponents(e, DummyTransformComponent{}, DummyMeshComponent{},
                     DummyHpComponent{});
    FlushAndRequireNoPending();

    em.RemoveComponents<DummyTransformComponent, DummyMeshComponent,
                        DummyHpComponent>(e);
    FlushAndRequireNoPending();

    REQUIRE(em.IsEntity(e));
    REQUIRE(!Has<DummyTransformComponent>(e));
    REQUIRE(!Has<DummyMeshComponent>(e));
    REQUIRE(!Has<DummyHpComponent>(e));
    REQUIRE(em.GetEntityCount() == baseline_count + 1);
  }

  SECTION("Destroy cancels pending structural move") {
    const auto e{scope.Generate()};

    em.AddComponents(e, DummyTransformComponent{});
    FlushAndRequireNoPending();

    em.AddComponents(e, DummyHpComponent{});
    em.RemoveComponents<DummyTransformComponent>(e);
    em.Destroy(e);

    FlushAndRequireNoPending();

    REQUIRE(!em.IsEntity(e));
    REQUIRE(em.GetEntityCount() == baseline_count);
  }

  SECTION("Two archetypes exchange many entities in one flush") {
    constexpr usize kCount{128};
    Array<EntityId> a{Array<EntityId>::WithCapacity(
        &entity::EntityMemoryContext::Get().GetEntityIdAllocator(), kCount)};
    Array<EntityId> b{Array<EntityId>::WithCapacity(
        &entity::EntityMemoryContext::Get().GetEntityIdAllocator(), kCount)};

    for (usize i{0}; i < kCount; ++i) {
      const auto ea{scope.Generate()};
      const auto eb{scope.Generate()};

      a.PushLast(ea);
      b.PushLast(eb);

      em.AddComponents(ea, DummyTransformComponent{}, DummyHpComponent{});
      em.AddComponents(eb, DummyTransformComponent{}, DummyMeshComponent{});
    }

    FlushAndRequireNoPending();

    for (usize i{0}; i < kCount; ++i) {
      em.RemoveComponents<DummyHpComponent>(a[i]);
      em.AddComponents(a[i], DummyMeshComponent{});

      em.RemoveComponents<DummyMeshComponent>(b[i]);
      em.AddComponents(b[i], DummyHpComponent{});
    }

    FlushAndRequireNoPending();

    for (usize i{0}; i < kCount; ++i) {
      REQUIRE(Has<DummyTransformComponent>(a[i]));
      REQUIRE(Has<DummyMeshComponent>(a[i]));
      REQUIRE(!Has<DummyHpComponent>(a[i]));

      REQUIRE(Has<DummyTransformComponent>(b[i]));
      REQUIRE(Has<DummyHpComponent>(b[i]));
      REQUIRE(!Has<DummyMeshComponent>(b[i]));
    }

    REQUIRE(em.GetEntityCount() == baseline_count + kCount * 2);
  }

  SECTION("Destroy and move entities from same source archetype in one flush") {
    constexpr usize kCount{96};
    Array<EntityId> ids{Array<EntityId>::WithCapacity(
        &entity::EntityMemoryContext::Get().GetEntityIdAllocator(), kCount)};

    for (usize i{0}; i < kCount; ++i) {
      const auto e{scope.Generate()};
      ids.PushLast(e);
      em.AddComponents(e, DummyTransformComponent{}, DummyHpComponent{});
    }

    FlushAndRequireNoPending();

    usize destroyed_count{0};

    for (usize i{0}; i < kCount; ++i) {
      if (i % 3 == 0) {
        em.Destroy(ids[i]);
        ++destroyed_count;
      } else if (i % 3 == 1) {
        em.AddComponents(ids[i], DummyMeshComponent{});
      } else {
        em.RemoveComponents<DummyHpComponent>(ids[i]);
      }
    }

    FlushAndRequireNoPending();

    REQUIRE(em.GetEntityCount() == baseline_count + kCount - destroyed_count);

    for (usize i{0}; i < kCount; ++i) {
      if (i % 3 == 0) {
        REQUIRE(!em.IsEntity(ids[i]));
      } else {
        REQUIRE(em.IsEntity(ids[i]));
        REQUIRE(Has<DummyTransformComponent>(ids[i]));
      }
    }
  }

  SECTION("ForEach and ForEachId reflect committed archetypes only") {
    const auto a{scope.Generate()};
    const auto b{scope.Generate()};
    const auto c{scope.Generate()};

    em.AddComponents(a, DummyTransformComponent{}, DummyHpComponent{});
    em.AddComponents(b, DummyTransformComponent{});
    em.AddComponents(c, DummyMeshComponent{});
    FlushAndRequireNoPending();

    REQUIRE(CountView<DummyTransformComponent>() == 2);
    REQUIRE(CountView<DummyTransformComponent, DummyHpComponent>() == 1);
    REQUIRE(CountIdView<DummyMeshComponent>() == 1);

    em.AddComponents(b, DummyHpComponent{});
    em.RemoveComponents<DummyMeshComponent>(c);
    FlushAndRequireNoPending();

    REQUIRE(CountView<DummyTransformComponent>() == 2);
    REQUIRE(CountView<DummyTransformComponent, DummyHpComponent>() == 2);
    REQUIRE(CountIdView<DummyMeshComponent>() == 0);
  }

  SECTION("Hierarchy parent tag survives structural moves") {
    const auto parent{scope.Generate()};
    const auto child{scope.Generate()};

    em.AddComponents(parent, DummyTransformComponent{});
    em.AddComponents(child, DummyTransformComponent{});
    FlushAndRequireNoPending();

    em.AddParent(child, parent);
    FlushAndRequireNoPending();

    REQUIRE(em.HasAnyParent(child));
    REQUIRE(em.HasParent(child, parent));
    REQUIRE(em.GetParentId(child) == parent);

    em.AddComponents(child, DummyHpComponent{}, DummyMeshComponent{});
    FlushAndRequireNoPending();

    REQUIRE(em.HasAnyParent(child));
    REQUIRE(em.HasParent(child, parent));
    REQUIRE(Has<DummyHpComponent>(child));
    REQUIRE(Has<DummyMeshComponent>(child));

    bool found_child{false};
    em.ForEachChildId<>(
        [&](EntityId id) {
          if (id == child) {
            found_child = true;
          }
        },
        parent);

    REQUIRE(found_child);
  }
}

TEST_CASE(
    "Entity stress structural churn preserves entity count and data validity",
    "[comet::entity][stress][.]") {
  using namespace comet;
  using namespace comet::comettests;

  auto& em{entity::EntityManager::Get()};
  EntityScope scope{};
  const auto baseline_count{em.GetEntityCount()};

  constexpr usize kEntityCount{256};
  constexpr usize kIterationCount{256};

  Array<EntityId> ids{Array<EntityId>::WithCapacity(
      &entity::EntityMemoryContext::Get().GetEntityIdAllocator(),
      kEntityCount)};

  for (usize i{0}; i < kEntityCount; ++i) {
    const auto e{scope.Generate()};
    ids.PushLast(e);

    em.AddComponents(e, DummyTransformComponent{},
                     DummyHpComponent{static_cast<u16>(i), 0, 100, 0});
  }

  FlushAndRequireNoPending();

  const auto count_after_create{em.GetEntityCount()};
  REQUIRE(count_after_create == baseline_count + kEntityCount);

  for (usize iteration{0}; iteration < kIterationCount; ++iteration) {
    for (usize i{0}; i < kEntityCount; ++i) {
      const auto e{ids[i]};

      switch ((iteration + i) % 8) {
        case 0:
          em.AddComponents(e, DummyMeshComponent{});
          break;
        case 1:
          em.RemoveComponents<DummyMeshComponent>(e);
          break;
        case 2:
          em.AddComponents(e, DummyTagComponent{});
          break;
        case 3:
          em.RemoveComponents<DummyTagComponent>(e);
          break;
        case 4:
          em.AddComponents(
              e, DummyHpComponent{static_cast<u16>(iteration), 1, 200, 2});
          break;
        case 5:
          em.RemoveComponents<DummyHpComponent>(e);
          break;
        case 6:
          em.AddComponents(e, DummyHpComponent{7, 8, 9, 10},
                           DummyMeshComponent{});
          break;
        case 7:
          em.RemoveComponents<DummyMeshComponent, DummyTagComponent>(e);
          break;
      }
    }

    FlushAndRequireNoPending();

    REQUIRE(em.GetEntityCount() == count_after_create);

    for (const auto e : ids) {
      REQUIRE(em.IsEntity(e));
      REQUIRE(Has<DummyTransformComponent>(e));
    }
  }
}

TEST_CASE("Entity pending writes are safe while committed reads stay stable",
          "[comet::entity][stress][threading][.]") {
  using namespace comet;
  using namespace comet::comettests;

  auto& em{entity::EntityManager::Get()};
  EntityScope scope{};

  constexpr usize kEntityCount{256};
  constexpr usize kJobCount{8};
  constexpr usize kIterations{8192};
  constexpr usize kMinReadPassCount{128};

  Array<EntityId> ids{Array<EntityId>::WithCapacity(
      &entity::EntityMemoryContext::Get().GetEntityIdAllocator(),
      kEntityCount)};

  for (usize i{0}; i < kEntityCount; ++i) {
    const auto e{scope.Generate()};
    ids.PushLast(e);

    em.AddComponents(e, DummyTransformComponent{},
                     DummyHpComponent{static_cast<u16>(i), 0, 100, 0});
  }

  FlushAndRequireNoPending();

  REQUIRE(em.GetEntityCount() == kEntityCount);
  REQUIRE(CountView<DummyTransformComponent>() == kEntityCount);
  REQUIRE(CountView<DummyTransformComponent, DummyHpComponent>() ==
          kEntityCount);
  REQUIRE(CountView<DummyMeshComponent>() == 0);
  REQUIRE(CountView<DummyTagComponent>() == 0);

  struct JobParams {
    Array<EntityId>* ids{nullptr};
    usize job_index{0};
    std::atomic<bool>* start{nullptr};
    std::atomic<usize>* ready_count{nullptr};
  };

  std::atomic<bool> start{false};
  std::atomic<usize> ready_count{0};

  auto& params{*COMET_FRAME_ARRAY_WITH_CAPACITY(JobParams, kJobCount)};
  params.Resize(kJobCount);

  job::CounterGuard guard{};

  for (usize job_index{0}; job_index < kJobCount; ++job_index) {
    params[job_index].ids = &ids;
    params[job_index].job_index = job_index;
    params[job_index].start = &start;
    params[job_index].ready_count = &ready_count;

    job::Scheduler::Get().Kick(job::GenerateJobDescr(
        job::JobPriority::High,
        [](job::JobParamsHandle handle) {
          auto* params{reinterpret_cast<JobParams*>(handle)};
          COMET_ASSERT(params != nullptr,
                       "Entity pending threading stress test",
                       "job params are null");
          COMET_ASSERT(params->ids != nullptr,
                       "Entity pending threading stress test", "ids are null");
          COMET_ASSERT(params->start != nullptr,
                       "Entity pending threading stress test",
                       "start gate is null");
          COMET_ASSERT(params->ready_count != nullptr,
                       "Entity pending threading stress test",
                       "ready counter is null");

          auto& em{entity::EntityManager::Get()};
          auto& ids{*params->ids};

          params->ready_count->fetch_add(1, std::memory_order_release);

          while (!params->start->load(std::memory_order_acquire)) {
            fiber::Yield();
          }

          for (usize iteration{0}; iteration < kIterations; ++iteration) {
            for (usize i{params->job_index}; i < ids.GetSize();
                 i += kJobCount) {
              const auto e{ids[i]};

              switch ((iteration + i + params->job_index) % 6) {
                case 0:
                  em.AddComponents(e, DummyMeshComponent{});
                  break;

                case 1:
                  em.RemoveComponents<DummyMeshComponent>(e);
                  break;

                case 2:
                  em.AddComponents(e, DummyTagComponent{});
                  break;

                case 3:
                  em.RemoveComponents<DummyTagComponent>(e);
                  break;

                case 4:
                  em.AddComponents(
                      e,
                      DummyHpComponent{static_cast<u16>(iteration), 1, 200, 2});
                  break;

                case 5:
                  em.RemoveComponents<DummyHpComponent>(e);
                  break;

                default:
                  COMET_ASSERT(false, "Entity pending threading stress test",
                               "unexpected mutation case");
                  break;
              }
            }

            if ((iteration & 31) == 0) {
              fiber::Yield();
            }
          }
        },
        &params[job_index], job::JobStackSize::Normal, guard.GetCounter(),
        "entity_pending_write_stress"));
  }

  while (ready_count.load(std::memory_order_acquire) < kJobCount) {
    fiber::Yield();
  }

  start.store(true, std::memory_order_release);

  usize read_pass_count{0};

  while (!guard.GetCounter()->IsZero() || read_pass_count < kMinReadPassCount) {
    REQUIRE(em.GetEntityCount() == kEntityCount);
    REQUIRE(CountView<DummyTransformComponent>() == kEntityCount);
    REQUIRE(CountView<DummyTransformComponent, DummyHpComponent>() ==
            kEntityCount);
    REQUIRE(CountView<DummyMeshComponent>() == 0);
    REQUIRE(CountView<DummyTagComponent>() == 0);

    for (const auto e : ids) {
      REQUIRE(em.IsEntity(e));
      REQUIRE(Has<DummyTransformComponent>(e));
      REQUIRE(Has<DummyHpComponent>(e));
      REQUIRE(!Has<DummyMeshComponent>(e));
      REQUIRE(!Has<DummyTagComponent>(e));
    }

    ++read_pass_count;
    fiber::Yield();
  }

  guard.Wait();

  REQUIRE(read_pass_count >= kMinReadPassCount);

  // Still unflushed: committed state must remain the original snapshot.
  REQUIRE(em.GetEntityCount() == kEntityCount);
  REQUIRE(CountView<DummyTransformComponent>() == kEntityCount);
  REQUIRE(CountView<DummyTransformComponent, DummyHpComponent>() ==
          kEntityCount);
  REQUIRE(CountView<DummyMeshComponent>() == 0);
  REQUIRE(CountView<DummyTagComponent>() == 0);

  FlushAndRequireNoPending();

  REQUIRE(em.GetEntityCount() == kEntityCount);
  REQUIRE(CountView<DummyTransformComponent>() == kEntityCount);

  for (const auto e : ids) {
    REQUIRE(em.IsEntity(e));
    REQUIRE(Has<DummyTransformComponent>(e));
  }
}