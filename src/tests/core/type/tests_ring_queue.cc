// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "comet_precompile.h"

#include "comet/core/type/exception.h"
#include "comet/core/type/ring_queue.h"

#include "catch.hpp"

#include "tests/dummies/dummy_object.h"

TEST_CASE("Ring queue creation with specific capacity", "[comet]") {
  const comet::uindex capacity{15};

  auto queue{comet::ring_queue<std::unique_ptr<comet::comettests::DummyObject>>(
      capacity)};

  SECTION("Properties after creation with specific capacity.") {
    REQUIRE(queue.capacity() == capacity);
    REQUIRE(queue.size() == 0);
  }
}

TEST_CASE("Ring queue push operations in single thread", "[comet]") {
  const comet::uindex capacity{15};

  SECTION("Properties after pushing one element.") {
    auto queue{
        comet::ring_queue<std::unique_ptr<comet::comettests::DummyObject>>(
            capacity)};

    const auto dummy1{comet::comettests::DummyObject(0)};
    queue.push(std::make_unique<comet::comettests::DummyObject>(0));

    REQUIRE(queue.size() == 1);
  }

  SECTION("Properties after pushing two elements.") {
    auto queue{
        comet::ring_queue<std::unique_ptr<comet::comettests::DummyObject>>(
            capacity)};

    queue.push(std::make_unique<comet::comettests::DummyObject>(0));
    queue.push(std::make_unique<comet::comettests::DummyObject>(0));
    REQUIRE(queue.size() == 2);
  }

  SECTION("Properties after pushing the maximum amount of elements.") {
    auto queue{
        comet::ring_queue<std::unique_ptr<comet::comettests::DummyObject>>(1)};

    queue.push(std::make_unique<comet::comettests::DummyObject>(0));
    REQUIRE(queue.size() == 1);
  }

  SECTION("Properties after pushing too many elements.") {
    auto queue{
        comet::ring_queue<std::unique_ptr<comet::comettests::DummyObject>>(1)};

    queue.push(std::make_unique<comet::comettests::DummyObject>(0));

    auto is_exception{false};

    try {
      queue.push(std::make_unique<comet::comettests::DummyObject>(0));
    } catch (const comet::maximum_capacity_reached_error&) {
      is_exception = true;
    }

    REQUIRE(is_exception);
    REQUIRE(queue.size() == 1);
  }

  SECTION("Specific error case: pushing one element in a 0-capacity queue.") {
    auto queue{
        comet::ring_queue<std::unique_ptr<comet::comettests::DummyObject>>(0)};

    auto is_exception{false};

    try {
      queue.push(std::make_unique<comet::comettests::DummyObject>(0));
    } catch (const comet::maximum_capacity_reached_error&) {
      is_exception = true;
    }

    REQUIRE(is_exception);
  }
}

TEST_CASE("Ring queue front & pop operations in single thread", "[comet]") {
  const comet::uindex capacity{15};

  SECTION("Properties after front/pop operations on one element.") {
    auto queue{
        comet::ring_queue<std::unique_ptr<comet::comettests::DummyObject>>(
            capacity)};

    queue.push(std::make_unique<comet::comettests::DummyObject>(42));
    REQUIRE(queue.front()->GetValue() == 42);
    queue.pop();
    REQUIRE(queue.size() == 0);
  }

  SECTION("Properties after front/pop operations on two elements.") {
    auto queue{
        comet::ring_queue<std::unique_ptr<comet::comettests::DummyObject>>(
            capacity)};

    queue.push(std::make_unique<comet::comettests::DummyObject>(42));
    REQUIRE(queue.front()->GetValue() == 42);
    queue.pop();
    REQUIRE(queue.size() == 0);
  }

  SECTION(
      "Properties after front/pop operations on one element in a two-element "
      "queue.") {
    auto queue{
        comet::ring_queue<std::unique_ptr<comet::comettests::DummyObject>>(
            capacity)};

    queue.push(std::make_unique<comet::comettests::DummyObject>(0));
    queue.push(std::make_unique<comet::comettests::DummyObject>(0));
    REQUIRE(queue.front());
    queue.pop();
    REQUIRE(queue.size() == 1);
  }

  SECTION("Properties after front/pop operations on too many elements.") {
    auto queue{
        comet::ring_queue<std::unique_ptr<comet::comettests::DummyObject>>(1)};

    queue.push(std::make_unique<comet::comettests::DummyObject>(42));
    queue.front();
    queue.pop();
    auto is_exception{false};

    try {
      queue.front();
      queue.pop();
    } catch (const comet::empty_error&) {
      is_exception = true;
    }

    REQUIRE(is_exception);
    REQUIRE(queue.size() == 0);
  }

  SECTION("Order is respected.") {
    auto queue{
        comet::ring_queue<std::unique_ptr<comet::comettests::DummyObject>>(3)};

    queue.push(std::make_unique<comet::comettests::DummyObject>(1));
    queue.push(std::make_unique<comet::comettests::DummyObject>(2));
    queue.push(std::make_unique<comet::comettests::DummyObject>(3));

    REQUIRE(queue.front().get()->GetValue() == 1);
    queue.pop();
    REQUIRE(queue.front().get()->GetValue() == 2);
    queue.pop();
    REQUIRE(queue.front().get()->GetValue() == 3);
    queue.pop();
  }

  SECTION(
      "Specific error case: front/pop operation on one element in a 0-capacity "
      "queue.") {
    auto queue{
        comet::ring_queue<std::unique_ptr<comet::comettests::DummyObject>>(0)};

    auto is_exception{false};

    try {
      queue.front();
      queue.pop();
    } catch (const comet::empty_error&) {
      is_exception = true;
    }

    REQUIRE(is_exception);
  }
}