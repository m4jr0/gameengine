// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "comet/core/type/ring_queue.h"

#include "catch.hpp"

#include "comet/core/essentials.h"
#include "comet/core/memory/allocator/platform_allocator.h"
#include "comet/core/memory/memory.h"
#include "comet/core/type/exception.h"
#include "tests/dummies/dummy_object.h"

namespace comet {
namespace comettests {
namespace memory {
enum TestsMemoryTag : comet::memory::MemoryTag {
  kTestsMemoryTagGeneral = comet::memory::kEngineMemoryTagUserBase + 1
};
}  // namespace memory

comet::memory::PlatformAllocator allocator{memory::kTestsMemoryTagGeneral};

class TestsFileSystemEventListener : public Catch::EventListenerBase {
  using Catch::EventListenerBase::EventListenerBase;

 public:
  void testCaseStarting(Catch::TestCaseInfo const&) override {
    allocator.Initialize();
  }

  void testCaseEnded(Catch::TestCaseStats const&) override {
    allocator.Destroy();
  }
};

CATCH_REGISTER_LISTENER(TestsFileSystemEventListener)
}  // namespace comettests
}  // namespace comet

TEST_CASE("Ring queue creation with specific capacity", "[comet]") {
  const comet::usize capacity{15};

  auto queue{comet::RingQueue<std::unique_ptr<comet::comettests::DummyObject>>(
      &comet::comettests::allocator, capacity)};

  SECTION("Properties after creation with specific capacity.") {
    REQUIRE(queue.GetCapacity() == capacity);
    REQUIRE(queue.GetSize() == 0);
  }
}

TEST_CASE("Ring queue push operations in single thread", "[comet]") {
  const comet::usize capacity{15};

  SECTION("Properties after pushing one element.") {
    auto queue{
        comet::RingQueue<std::unique_ptr<comet::comettests::DummyObject>>(

            &comet::comettests::allocator, capacity)};

    const auto dummy1{comet::comettests::DummyObject(0)};
    queue.Push(std::make_unique<comet::comettests::DummyObject>(0));

    REQUIRE(queue.GetSize() == 1);
  }

  SECTION("Properties after pushing two elements.") {
    auto queue{
        comet::RingQueue<std::unique_ptr<comet::comettests::DummyObject>>(

            &comet::comettests::allocator, capacity)};

    queue.Push(std::make_unique<comet::comettests::DummyObject>(0));
    queue.Push(std::make_unique<comet::comettests::DummyObject>(0));
    REQUIRE(queue.GetSize() == 2);
  }

  SECTION("Properties after pushing the maximum amount of elements.") {
    auto queue{
        comet::RingQueue<std::unique_ptr<comet::comettests::DummyObject>>(
            &comet::comettests::allocator, 1)};

    queue.Push(std::make_unique<comet::comettests::DummyObject>(0));
    REQUIRE(queue.GetSize() == 1);
  }

  SECTION("Properties after pushing too many elements.") {
    auto queue{
        comet::RingQueue<std::unique_ptr<comet::comettests::DummyObject>>(
            &comet::comettests::allocator, 1)};

    queue.Push(std::make_unique<comet::comettests::DummyObject>(0));

    auto is_exception{false};

    try {
      queue.Push(std::make_unique<comet::comettests::DummyObject>(0));
    } catch (const comet::MaximumCapacityReachedError&) {
      is_exception = true;
    }

    REQUIRE(is_exception);
    REQUIRE(queue.GetSize() == 1);
  }

  SECTION("Specific error case: pushing one element in a 0-capacity queue.") {
    auto queue{
        comet::RingQueue<std::unique_ptr<comet::comettests::DummyObject>>(
            &comet::comettests::allocator, 0)};

    auto is_exception{false};

    try {
      queue.Push(std::make_unique<comet::comettests::DummyObject>(0));
    } catch (const comet::MaximumCapacityReachedError&) {
      is_exception = true;
    }

    REQUIRE(is_exception);
  }
}

TEST_CASE("Ring queue front & pop operations in single thread", "[comet]") {
  const comet::usize capacity{15};

  SECTION("Properties after get/pop operations on one element.") {
    auto queue{
        comet::RingQueue<std::unique_ptr<comet::comettests::DummyObject>>(

            &comet::comettests::allocator, capacity)};

    queue.Push(std::make_unique<comet::comettests::DummyObject>(42));
    REQUIRE(queue.Get()->GetValue() == 42);
    queue.TryPop();
    REQUIRE(queue.GetSize() == 0);
  }

  SECTION("Properties after get/pop operations on two elements.") {
    auto queue{
        comet::RingQueue<std::unique_ptr<comet::comettests::DummyObject>>(

            &comet::comettests::allocator, capacity)};

    queue.Push(std::make_unique<comet::comettests::DummyObject>(42));
    REQUIRE(queue.Get()->GetValue() == 42);
    queue.TryPop();
    REQUIRE(queue.GetSize() == 0);
  }

  SECTION(
      "Properties after get/pop operations on one element in a two-element "
      "queue.") {
    auto queue{
        comet::RingQueue<std::unique_ptr<comet::comettests::DummyObject>>(

            &comet::comettests::allocator, capacity)};

    queue.Push(std::make_unique<comet::comettests::DummyObject>(0));
    queue.Push(std::make_unique<comet::comettests::DummyObject>(0));
    REQUIRE(queue.Get());
    queue.TryPop();
    REQUIRE(queue.GetSize() == 1);
  }

  SECTION("Properties after get/pop operations on too many elements.") {
    auto queue{
        comet::RingQueue<std::unique_ptr<comet::comettests::DummyObject>>(
            &comet::comettests::allocator, 1)};

    queue.Push(std::make_unique<comet::comettests::DummyObject>(42));
    queue.Get();
    queue.TryPop();
    auto is_exception{false};

    try {
      queue.Get();
      queue.TryPop();
    } catch (const comet::EmptyError&) {
      is_exception = true;
    }

    REQUIRE(is_exception);
    REQUIRE(queue.GetSize() == 0);
  }

  SECTION("Order is respected.") {
    auto queue{
        comet::RingQueue<std::unique_ptr<comet::comettests::DummyObject>>(
            &comet::comettests::allocator, 3)};

    queue.Push(std::make_unique<comet::comettests::DummyObject>(1));
    queue.Push(std::make_unique<comet::comettests::DummyObject>(2));
    queue.Push(std::make_unique<comet::comettests::DummyObject>(3));

    REQUIRE(queue.Get().get()->GetValue() == 1);
    queue.TryPop();
    REQUIRE(queue.Get().get()->GetValue() == 2);
    queue.TryPop();
    REQUIRE(queue.Get().get()->GetValue() == 3);
    queue.TryPop();
  }

  SECTION(
      "Specific error case: get/pop operation on one element in a 0-capacity "
      "queue.") {
    auto queue{
        comet::RingQueue<std::unique_ptr<comet::comettests::DummyObject>>(
            &comet::comettests::allocator, 0)};

    auto is_exception{false};

    try {
      queue.Get();
      queue.TryPop();
    } catch (const comet::EmptyError&) {
      is_exception = true;
    }

    REQUIRE(is_exception);
  }
}