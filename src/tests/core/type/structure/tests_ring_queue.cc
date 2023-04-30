// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "comet_precompile.h"

#include "comet/core/type/structure/exception.h"
#include "comet/core/type/structure/ring_queue.h"

#include "catch.hpp"

#include "tests/dummies/dummy_object.h"

TEST_CASE("Ring queue creation with specific capacity",
          "[comet::utils::structure]") {
  const comet::uindex capacity{15};

  auto queue{comet::ring_queue<std::unique_ptr<comet::comettests::DummyObject>>(
      capacity)};

  SECTION("Properties after creation with specific capacity.") {
    REQUIRE(queue.capacity() == capacity);
    REQUIRE(queue.size() == 0);
  }
}

TEST_CASE("Ring queue push operations in single thread",
          "[comet::utils::structure]") {
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
    } catch (const comet::maximum_capacity_reached_error& error) {
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
    } catch (const comet::maximum_capacity_reached_error& error) {
      is_exception = true;
    }

    REQUIRE(is_exception);
  }
}

TEST_CASE("Ring queue front & pop operations in single thread",
          "[comet::utils::structure]") {
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
    } catch (const comet::empty_error& error) {
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
    } catch (const comet::empty_error& error) {
      is_exception = true;
    }

    REQUIRE(is_exception);
  }
}

TEST_CASE("Thread-safe ring queue creation with specific capacity",
          "[comet::utils::structure]") {
  const comet::uindex capacity{15};

  auto queue{comet::concurrent_ring_queue<
      std::unique_ptr<comet::comettests::DummyObject>>(capacity)};

  SECTION("Properties after creation with specific capacity.") {
    REQUIRE(queue.capacity() == capacity);
    REQUIRE(queue.size() == 0);
  }
}

TEST_CASE("Thread-safe ring queue push operations in single thread",
          "[comet::utils::structure]") {
  const comet::uindex capacity{15};

  SECTION("Properties after pushing one element.") {
    auto queue{comet::concurrent_ring_queue<
        std::unique_ptr<comet::comettests::DummyObject>>(capacity)};

    const auto dummy1{comet::comettests::DummyObject(0)};
    queue.push(std::make_unique<comet::comettests::DummyObject>(0));

    REQUIRE(queue.size() == 1);
  }

  SECTION("Properties after pushing two elements.") {
    auto queue{comet::concurrent_ring_queue<
        std::unique_ptr<comet::comettests::DummyObject>>(capacity)};

    queue.push(std::make_unique<comet::comettests::DummyObject>(0));
    queue.push(std::make_unique<comet::comettests::DummyObject>(0));
    REQUIRE(queue.size() == 2);
  }

  SECTION("Properties after pushing the maximum amount of elements.") {
    auto queue{comet::concurrent_ring_queue<
        std::unique_ptr<comet::comettests::DummyObject>>(1)};

    queue.push(std::make_unique<comet::comettests::DummyObject>(0));
    REQUIRE(queue.size() == 1);
  }

  SECTION("Properties after pushing too many elements.") {
    auto queue{comet::concurrent_ring_queue<
        std::unique_ptr<comet::comettests::DummyObject>>(1)};

    queue.push(std::make_unique<comet::comettests::DummyObject>(0));

    auto is_exception{false};

    try {
      queue.push(std::make_unique<comet::comettests::DummyObject>(0));
    } catch (const comet::maximum_capacity_reached_error& error) {
      is_exception = true;
    }

    REQUIRE(is_exception);
    REQUIRE(queue.size() == 1);
  }

  SECTION("Specific error case: pushing one element in a 0-capacity queue.") {
    auto queue{comet::concurrent_ring_queue<
        std::unique_ptr<comet::comettests::DummyObject>>(0)};

    auto is_exception{false};

    try {
      queue.push(std::make_unique<comet::comettests::DummyObject>(0));
    } catch (const comet::maximum_capacity_reached_error& error) {
      is_exception = true;
    }

    REQUIRE(is_exception);
  }
}

TEST_CASE("Thread-safe ring queue front & pop operations in single thread",
          "[comet::utils::structure]") {
  const comet::uindex capacity{15};

  SECTION("Properties after front/pop operations on one element.") {
    auto queue{comet::concurrent_ring_queue<
        std::unique_ptr<comet::comettests::DummyObject>>(capacity)};

    queue.push(std::make_unique<comet::comettests::DummyObject>(42));
    REQUIRE(queue.front()->GetValue() == 42);
    queue.pop();
    REQUIRE(queue.size() == 0);
  }

  SECTION("Properties after front/pop operations on two elements.") {
    auto queue{comet::concurrent_ring_queue<
        std::unique_ptr<comet::comettests::DummyObject>>(capacity)};

    queue.push(std::make_unique<comet::comettests::DummyObject>(42));
    REQUIRE(queue.front()->GetValue() == 42);
    queue.pop();
    REQUIRE(queue.size() == 0);
  }

  SECTION(
      "Properties after front/pop operations on one element in a two-element "
      "queue.") {
    auto queue{comet::concurrent_ring_queue<
        std::unique_ptr<comet::comettests::DummyObject>>(capacity)};

    queue.push(std::make_unique<comet::comettests::DummyObject>(0));
    queue.push(std::make_unique<comet::comettests::DummyObject>(0));
    REQUIRE(queue.front());
    queue.pop();
    REQUIRE(queue.size() == 1);
  }

  SECTION("Properties after front/pop operations on too many elements.") {
    auto queue{comet::concurrent_ring_queue<
        std::unique_ptr<comet::comettests::DummyObject>>(1)};

    queue.push(std::make_unique<comet::comettests::DummyObject>(42));
    queue.front();
    queue.pop();
    auto is_exception{false};

    try {
      queue.front();
      queue.pop();
    } catch (const comet::empty_error& error) {
      is_exception = true;
    }

    REQUIRE(is_exception);
    REQUIRE(queue.size() == 0);
  }

  SECTION("Order is respected.") {
    auto queue{comet::concurrent_ring_queue<
        std::unique_ptr<comet::comettests::DummyObject>>(3)};

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
    auto queue{comet::concurrent_ring_queue<
        std::unique_ptr<comet::comettests::DummyObject>>(0)};

    auto is_exception{false};

    try {
      queue.front();
      queue.pop();
    } catch (const comet::empty_error& error) {
      is_exception = true;
    }

    REQUIRE(is_exception);
  }
}

TEST_CASE("Thread-safe ring queue push operations in multiple threads",
          "[comet::utils::structure]") {
  const comet::uindex capacity{15};
  SECTION("Usage in threads: push operations.") {
    comet::uindex thread_number{5};
    auto queue{comet::concurrent_ring_queue<
        std::unique_ptr<comet::comettests::DummyObject>>(5)};

    std::vector<std::thread> threads;

    auto is_empty_error{false};
    auto is_max_capacity_error{false};
    auto is_generic_error{false};

    for (comet::uindex index{0}; index < thread_number; index++) {
      threads.push_back(
          std::thread([&queue, &is_empty_error, &is_max_capacity_error,
                       &is_generic_error]() {
            try {
              queue.push(std::make_unique<comet::comettests::DummyObject>(0));
            } catch (const comet::empty_error& error) {
              is_empty_error = true;
            } catch (const comet::maximum_capacity_reached_error& error) {
              is_max_capacity_error = true;
            } catch (...) {
              is_generic_error = true;
            }
          }));
    }

    std::for_each(threads.begin(), threads.end(),
                  [](std::thread& thread) { thread.join(); });

    REQUIRE(queue.size() == thread_number);
    REQUIRE(!is_empty_error);
    REQUIRE(!is_max_capacity_error);
    REQUIRE(!is_generic_error);
  }
}

TEST_CASE("Thread-safe ring queue front & pop operations in multiple threads",
          "[comet::utils::structure]") {
  SECTION("Usage in threads: front/pop operations.") {
    comet::uindex thread_number{5};
    auto queue{comet::concurrent_ring_queue<
        std::unique_ptr<comet::comettests::DummyObject>>(5)};

    std::vector<std::thread> threads;

    for (comet::uindex index{0}; index < thread_number; index++) {
      queue.push(std::make_unique<comet::comettests::DummyObject>(0));
    }

    auto is_empty_error{false};
    auto is_max_capacity_error{false};
    auto is_generic_error{false};

    for (comet::uindex index{0}; index < thread_number; index++) {
      threads.push_back(
          std::thread([&queue, &is_empty_error, &is_max_capacity_error,
                       &is_generic_error]() {
            try {
              queue.front();
              queue.pop();
            } catch (const comet::empty_error& error) {
              is_empty_error = true;
            } catch (const comet::maximum_capacity_reached_error& error) {
              is_max_capacity_error = true;
            } catch (...) {
              is_generic_error = true;
            }
          }));
    }

    std::for_each(threads.begin(), threads.end(),
                  [](std::thread& thread) { thread.join(); });

    REQUIRE(queue.size() == 0);
    REQUIRE(!is_empty_error);
    REQUIRE(!is_max_capacity_error);
    REQUIRE(!is_generic_error);
  }

  SECTION("Usage in threads: waiting for front operations.") {
    comet::uindex getting_front_thread_number{5};
    comet::uindex pushing_threads_number{10};

    auto queue{comet::concurrent_ring_queue<
        std::unique_ptr<comet::comettests::DummyObject>>(
        pushing_threads_number)};

    std::vector<std::thread> threads;

    auto is_empty_error{false};
    auto is_max_capacity_error{false};
    auto is_generic_error{false};

    for (comet::uindex index{0}; index < getting_front_thread_number; index++) {
      threads.push_back(
          std::thread([&queue, &is_empty_error, &is_max_capacity_error,
                       &is_generic_error]() {
            try {
              queue.wait_for_front();
            } catch (const comet::empty_error& error) {
              is_empty_error = true;
            } catch (const comet::maximum_capacity_reached_error& error) {
              is_max_capacity_error = true;
            } catch (...) {
              is_generic_error = true;
            }
          }));
    }

    for (comet::uindex index{0}; index < pushing_threads_number; index++) {
      threads.push_back(
          std::thread([&queue, &is_empty_error, &is_max_capacity_error,
                       &is_generic_error]() {
            try {
              std::this_thread::sleep_for(std::chrono::seconds(1));
              queue.push(std::make_unique<comet::comettests::DummyObject>(0));
            } catch (const comet::empty_error& error) {
              is_empty_error = true;
            } catch (const comet::maximum_capacity_reached_error& error) {
              is_max_capacity_error = true;
            } catch (...) {
              is_generic_error = true;
            }
          }));
    }

    std::for_each(threads.begin(), threads.end(),
                  [](std::thread& thread) { thread.join(); });

    REQUIRE(queue.size() == pushing_threads_number);
    REQUIRE(!is_empty_error);
    REQUIRE(!is_max_capacity_error);
    REQUIRE(!is_generic_error);
  }

  SECTION("Usage in threads: waiting and popping front operations.") {
    comet::uindex popping_front_thread_number{5};
    comet::uindex pushing_threads_number{10};

    auto queue{comet::concurrent_ring_queue<
        std::unique_ptr<comet::comettests::DummyObject>>(
        pushing_threads_number)};

    std::vector<std::thread> threads;

    auto is_empty_error{false};
    auto is_max_capacity_error{false};
    auto is_generic_error{false};

    for (comet::uindex index{0}; index < popping_front_thread_number; index++) {
      threads.push_back(
          std::thread([&queue, &is_empty_error, &is_max_capacity_error,
                       &is_generic_error]() {
            try {
              queue.wait_and_pop_front();
            } catch (const comet::empty_error& error) {
              is_empty_error = true;
            } catch (const comet::maximum_capacity_reached_error& error) {
              is_max_capacity_error = true;
            } catch (...) {
              is_generic_error = true;
            }
          }));
    }

    for (comet::uindex index{0}; index < pushing_threads_number; index++) {
      threads.push_back(
          std::thread([&queue, &is_empty_error, &is_max_capacity_error,
                       &is_generic_error]() {
            try {
              std::this_thread::sleep_for(std::chrono::seconds(1));
              queue.push(std::make_unique<comet::comettests::DummyObject>(0));
            } catch (const comet::empty_error& error) {
              is_empty_error = true;
            } catch (const comet::maximum_capacity_reached_error& error) {
              is_max_capacity_error = true;
            } catch (...) {
              is_generic_error = true;
            }
          }));
    }

    std::for_each(threads.begin(), threads.end(),
                  [](std::thread& thread) { thread.join(); });

    REQUIRE(queue.size() ==
            (pushing_threads_number - popping_front_thread_number));
    REQUIRE(!is_empty_error);
    REQUIRE(!is_max_capacity_error);
    REQUIRE(!is_generic_error);
  }

  SECTION("Usage in threads: waiting for data.") {
    comet::uindex waiting_thread_number{5};
    comet::uindex pushing_threads_number{10};

    auto queue{comet::concurrent_ring_queue<
        std::unique_ptr<comet::comettests::DummyObject>>(
        pushing_threads_number)};

    std::vector<std::thread> threads;

    auto is_empty_error{false};
    auto is_max_capacity_error{false};
    auto is_generic_error{false};

    for (comet::uindex index{0}; index < waiting_thread_number; index++) {
      threads.push_back(
          std::thread([&queue, &is_empty_error, &is_max_capacity_error,
                       &is_generic_error]() {
            try {
              queue.wait_for_data();
            } catch (const comet::empty_error& error) {
              is_empty_error = true;
            } catch (const comet::maximum_capacity_reached_error& error) {
              is_max_capacity_error = true;
            } catch (...) {
              is_generic_error = true;
            }
          }));
    }

    for (comet::uindex index{0}; index < pushing_threads_number; index++) {
      threads.push_back(
          std::thread([&queue, &is_empty_error, &is_max_capacity_error,
                       &is_generic_error]() {
            try {
              std::this_thread::sleep_for(std::chrono::seconds(1));
              queue.push(std::make_unique<comet::comettests::DummyObject>(0));
            } catch (const comet::empty_error& error) {
              is_empty_error = true;
            } catch (const comet::maximum_capacity_reached_error& error) {
              is_max_capacity_error = true;
            } catch (...) {
              is_generic_error = true;
            }
          }));
    }

    std::for_each(threads.begin(), threads.end(),
                  [](std::thread& thread) { thread.join(); });

    REQUIRE(queue.size() == pushing_threads_number);
    REQUIRE(!is_empty_error);
    REQUIRE(!is_max_capacity_error);
    REQUIRE(!is_generic_error);
  }

  SECTION("Usage in threads: waiting for space.") {
    comet::uindex waiting_thread_number{10};
    comet::uindex popping_thread_number{5};
    comet::uindex capacity{popping_thread_number + 5};

    auto queue{comet::concurrent_ring_queue<
        std::unique_ptr<comet::comettests::DummyObject>>(capacity)};

    std::vector<std::thread> threads;

    for (comet::uindex index{0}; index < capacity; index++) {
      queue.push(std::make_unique<comet::comettests::DummyObject>(0));
    }

    auto is_empty_error{false};
    auto is_max_capacity_error{false};
    auto is_generic_error{false};

    for (comet::uindex index{0}; index < waiting_thread_number; index++) {
      threads.push_back(
          std::thread([&queue, &is_empty_error, &is_max_capacity_error,
                       &is_generic_error]() {
            try {
              queue.wait_for_data();
            } catch (const comet::empty_error& error) {
              is_empty_error = true;
            } catch (const comet::maximum_capacity_reached_error& error) {
              is_max_capacity_error = true;
            } catch (...) {
              is_generic_error = true;
            }
          }));
    }

    for (comet::uindex index{0}; index < popping_thread_number; index++) {
      threads.push_back(
          std::thread([&queue, &is_empty_error, &is_max_capacity_error,
                       &is_generic_error]() {
            try {
              std::this_thread::sleep_for(std::chrono::seconds(1));
              queue.pop();
            } catch (const comet::empty_error& error) {
              is_empty_error = true;
            } catch (const comet::maximum_capacity_reached_error& error) {
              is_max_capacity_error = true;
            } catch (...) {
              is_generic_error = true;
            }
          }));
    }

    std::for_each(threads.begin(), threads.end(),
                  [](std::thread& thread) { thread.join(); });

    REQUIRE(queue.size() == (capacity - popping_thread_number));
    REQUIRE(!is_empty_error);
    REQUIRE(!is_max_capacity_error);
    REQUIRE(!is_generic_error);
  }
}
