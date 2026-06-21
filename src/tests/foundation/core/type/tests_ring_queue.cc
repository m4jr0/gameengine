// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "tests_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Tested. /////////////////////////////////////////////////////////////////////
#include "comet/core/container/ring_queue.h"
////////////////////////////////////////////////////////////////////////////////

// External. ///////////////////////////////////////////////////////////////////
#include <atomic>
#include <memory>
#include <thread>
#include <vector>

#include "catch.hpp"
////////////////////////////////////////////////////////////////////////////////

#include "comet/runtime/memory/allocator/platform_allocator.h"
#include "comet/core/memory/memory.h"
#include "comet/runtime/memory/memory_tag.h"
#include "tests/dummies/dummy_object.h"

namespace comet {
namespace comettests {
namespace memory {
enum TestsMemoryTag : comet::memory::MemoryTag {
  kTestsMemoryTagGeneral = kEngineMemoryTagUserBase + 1
};
}  // namespace memory

namespace {
comet::memory::PlatformAllocator allocator{memory::kTestsMemoryTagGeneral};

comet::usize GetStressThreadCount() {
  const auto hw{std::thread::hardware_concurrency()};
  return hw == 0 ? 8 : static_cast<comet::usize>(hw);
}
}  // namespace
}  // namespace comettests
}  // namespace comet

TEST_CASE("Ring queue creation with specific capacity", "[comet][ring_queue]") {
  constexpr comet::usize kCapacity{15};

  const auto queue{comet::RingQueue<
      comet::memory::UniquePtr<comet::comettests::DummyObject>>{
      &comet::comettests::allocator, kCapacity}};

  REQUIRE(queue.GetCapacity() == kCapacity);
  REQUIRE(queue.GetSize() == 0);
  REQUIRE(queue.IsEmpty());
  REQUIRE(queue.TryGet() == nullptr);
}

TEST_CASE("Ring queue try push and capacity handling", "[comet][ring_queue]") {
  auto queue{comet::RingQueue<
      comet::memory::UniquePtr<comet::comettests::DummyObject>>{
      &comet::comettests::allocator, 2}};

  REQUIRE(queue.TryPush(std::make_unique<comet::comettests::DummyObject>(1)));
  REQUIRE(queue.TryPush(std::make_unique<comet::comettests::DummyObject>(2)));
  REQUIRE(!queue.TryPush(std::make_unique<comet::comettests::DummyObject>(3)));

  REQUIRE(queue.GetSize() == 2);
  REQUIRE(!queue.IsEmpty());
  REQUIRE(queue.TryGet() != nullptr);
  REQUIRE(queue.Get()->GetValue() == 1);
}

TEST_CASE("Ring queue zero capacity rejects pushes", "[comet][ring_queue]") {
  auto queue{comet::RingQueue<
      comet::memory::UniquePtr<comet::comettests::DummyObject>>{
      &comet::comettests::allocator, 0}};

  REQUIRE(queue.GetCapacity() == 0);
  REQUIRE(queue.GetSize() == 0);
  REQUIRE(queue.IsEmpty());
  REQUIRE(!queue.TryPush(std::make_unique<comet::comettests::DummyObject>(1)));
  REQUIRE(queue.TryGet() == nullptr);
}

TEST_CASE("Ring queue front pop and order", "[comet][ring_queue]") {
  auto queue{comet::RingQueue<
      comet::memory::UniquePtr<comet::comettests::DummyObject>>{
      &comet::comettests::allocator, 3}};

  REQUIRE(queue.TryPush(std::make_unique<comet::comettests::DummyObject>(1)));
  REQUIRE(queue.TryPush(std::make_unique<comet::comettests::DummyObject>(2)));
  REQUIRE(queue.TryPush(std::make_unique<comet::comettests::DummyObject>(3)));

  REQUIRE(queue.GetSize() == 3);

  REQUIRE(queue.Get()->GetValue() == 1);
  queue.TryPop();

  REQUIRE(queue.Get()->GetValue() == 2);
  queue.TryPop();

  REQUIRE(queue.Get()->GetValue() == 3);
  queue.TryPop();

  REQUIRE(queue.GetSize() == 0);
  REQUIRE(queue.IsEmpty());
  REQUIRE(queue.TryGet() == nullptr);

  queue.TryPop();
  REQUIRE(queue.GetSize() == 0);
}

TEST_CASE("Ring queue wraps around", "[comet][ring_queue]") {
  auto queue{comet::RingQueue<comet::s32>{&comet::comettests::allocator, 3}};

  REQUIRE(queue.TryPush(1));
  REQUIRE(queue.TryPush(2));
  REQUIRE(queue.TryPush(3));
  REQUIRE(!queue.TryPush(4));

  REQUIRE(queue.Get() == 1);
  queue.TryPop();

  REQUIRE(queue.TryPush(4));

  REQUIRE(queue.Get() == 2);
  queue.TryPop();
  REQUIRE(queue.Get() == 3);
  queue.TryPop();
  REQUIRE(queue.Get() == 4);
  queue.TryPop();

  REQUIRE(queue.IsEmpty());
}

TEST_CASE("Ring queue clear", "[comet][ring_queue]") {
  auto queue{comet::RingQueue<comet::s32>{&comet::comettests::allocator, 4}};

  REQUIRE(queue.TryPush(1));
  REQUIRE(queue.TryPush(2));

  queue.Clear();

  REQUIRE(queue.IsEmpty());
  REQUIRE(queue.GetSize() == 0);
  REQUIRE(queue.TryGet() == nullptr);

  REQUIRE(queue.TryPush(3));
  REQUIRE(queue.Get() == 3);
}

TEST_CASE("Ring queue copy performs deep copy", "[comet][ring_queue]") {
  auto queue{comet::RingQueue<comet::s32>{&comet::comettests::allocator, 4}};

  REQUIRE(queue.TryPush(1));
  REQUIRE(queue.TryPush(2));
  queue.TryPop();
  REQUIRE(queue.TryPush(3));

  auto copy{queue};

  REQUIRE(copy.GetSize() == 2);
  REQUIRE(copy.Get() == 2);
  copy.TryPop();
  REQUIRE(copy.Get() == 3);

  REQUIRE(queue.GetSize() == 2);
  REQUIRE(queue.Get() == 2);
}

TEST_CASE("Ring queue move preserves entries", "[comet][ring_queue]") {
  auto queue{comet::RingQueue<comet::s32>{&comet::comettests::allocator, 4}};

  REQUIRE(queue.TryPush(1));
  REQUIRE(queue.TryPush(2));

  auto moved{std::move(queue)};

  REQUIRE(moved.GetCapacity() == 4);
  REQUIRE(moved.GetSize() == 2);
  REQUIRE(moved.Get() == 1);
  moved.TryPop();
  REQUIRE(moved.Get() == 2);

  REQUIRE(queue.GetCapacity() == 0);
  REQUIRE(queue.GetSize() == 0);
}

TEST_CASE("LockFreeMPSC basic single-thread behavior", "[comet][ring_queue]") {
  comet::LockFreeMPSCRingQueue<comet::s32> queue{&comet::comettests::allocator,
                                                 4};

  REQUIRE(queue.GetCapacity() == 4);

  REQUIRE(queue.TryPush(1));
  REQUIRE(queue.TryPush(2));

  comet::s32 value{0};

  REQUIRE(queue.TryPop(value));
  REQUIRE(value == 1);

  REQUIRE(queue.TryPop(value));
  REQUIRE(value == 2);

  REQUIRE(!queue.TryPop(value));
}

TEST_CASE("LockFreeMPSC reports full", "[comet][ring_queue]") {
  comet::LockFreeMPSCRingQueue<comet::s32> queue{&comet::comettests::allocator,
                                                 2};

  REQUIRE(queue.TryPush(1));
  REQUIRE(queue.TryPush(2));
  REQUIRE(!queue.TryPush(3));

  comet::s32 value{0};

  REQUIRE(queue.TryPop(value));
  REQUIRE(value == 1);

  REQUIRE(queue.TryPush(3));

  REQUIRE(queue.TryPop(value));
  REQUIRE(value == 2);

  REQUIRE(queue.TryPop(value));
  REQUIRE(value == 3);

  REQUIRE(!queue.TryPop(value));
}

TEST_CASE("LockFreeMPSC clear", "[comet][ring_queue]") {
  comet::LockFreeMPSCRingQueue<comet::s32> queue{&comet::comettests::allocator,
                                                 4};

  REQUIRE(queue.TryPush(1));
  REQUIRE(queue.TryPush(2));

  queue.Clear();

  comet::s32 value{0};
  REQUIRE(!queue.TryPop(value));

  REQUIRE(queue.TryPush(3));
  REQUIRE(queue.TryPop(value));
  REQUIRE(value == 3);
}

TEST_CASE("LockFreeMPMC basic single-thread behavior", "[comet][ring_queue]") {
  comet::LockFreeMPMCRingQueue<comet::s32> queue{&comet::comettests::allocator,
                                                 4};

  REQUIRE(queue.GetCapacity() == 4);

  REQUIRE(queue.TryPush(1));
  REQUIRE(queue.TryPush(2));

  auto value{queue.TryPop()};
  REQUIRE(value.has_value());
  REQUIRE(*value == 1);

  value = queue.TryPop();
  REQUIRE(value.has_value());
  REQUIRE(*value == 2);

  REQUIRE(!queue.TryPop().has_value());
}

TEST_CASE("LockFreeMPMC reports full", "[comet][ring_queue]") {
  comet::LockFreeMPMCRingQueue<comet::s32> queue{&comet::comettests::allocator,
                                                 4};

  REQUIRE(queue.TryPush(1));
  REQUIRE(queue.TryPush(2));
  REQUIRE(queue.TryPush(3));
  REQUIRE(queue.TryPush(4));
  REQUIRE(!queue.TryPush(5));

  auto value{queue.TryPop()};
  REQUIRE(value.has_value());
  REQUIRE(*value == 1);

  REQUIRE(queue.TryPush(5));
}

TEST_CASE("LockFreeMPMC clear", "[comet][ring_queue]") {
  comet::LockFreeMPMCRingQueue<comet::s32> queue{&comet::comettests::allocator,
                                                 4};

  REQUIRE(queue.TryPush(1));
  REQUIRE(queue.TryPush(2));

  queue.Clear();

  REQUIRE(!queue.TryPop().has_value());

  REQUIRE(queue.TryPush(3));

  auto value{queue.TryPop()};
  REQUIRE(value.has_value());
  REQUIRE(*value == 3);
}

TEST_CASE("LockFreeMPSC pressure multiple producers single consumer",
          "[comet][ring_queue][stress][.]") {
  const auto kProducerCount{comet::comettests::GetStressThreadCount()};
  constexpr comet::usize kItemsPerProducer{4096};
  constexpr comet::usize kQueueCapacity{1024};
  const auto kTotalCount{kProducerCount * kItemsPerProducer};

  comet::LockFreeMPSCRingQueue<comet::usize> queue{
      &comet::comettests::allocator, kQueueCapacity};

  std::atomic<comet::usize> produced{0};
  std::atomic<comet::usize> consumed{0};
  std::atomic<comet::usize> duplicate_count{0};
  std::atomic<comet::usize> invalid_count{0};
  std::atomic<bool> done{false};

  std::unique_ptr<std::atomic<bool>[]> seen{new std::atomic<bool>[kTotalCount]};

  for (comet::usize i{0}; i < kTotalCount; ++i) {
    seen[i].store(false, std::memory_order_relaxed);
  }

  std::vector<std::thread> producers;

  for (comet::usize producer{0}; producer < kProducerCount; ++producer) {
    producers.emplace_back([&, producer] {
      for (comet::usize i{0}; i < kItemsPerProducer; ++i) {
        const auto value{producer * kItemsPerProducer + i};

        while (!queue.TryPush(value)) {
          std::this_thread::yield();
        }

        produced.fetch_add(1, std::memory_order_relaxed);
      }
    });
  }

  std::thread consumer{[&] {
    comet::usize value{0};

    while (!done.load(std::memory_order_acquire) ||
           consumed.load(std::memory_order_relaxed) < kTotalCount) {
      if (!queue.TryPop(value)) {
        std::this_thread::yield();
        continue;
      }

      if (value >= kTotalCount) {
        invalid_count.fetch_add(1, std::memory_order_relaxed);
        consumed.fetch_add(1, std::memory_order_relaxed);
        continue;
      }

      bool expected{false};

      if (!seen[value].compare_exchange_strong(expected, true,
                                               std::memory_order_acq_rel,
                                               std::memory_order_relaxed)) {
        duplicate_count.fetch_add(1, std::memory_order_relaxed);
      }

      consumed.fetch_add(1, std::memory_order_relaxed);
    }
  }};

  for (auto& producer : producers) {
    producer.join();
  }

  done.store(true, std::memory_order_release);
  consumer.join();

  REQUIRE(invalid_count.load(std::memory_order_relaxed) == 0);
  REQUIRE(duplicate_count.load(std::memory_order_relaxed) == 0);
  REQUIRE(produced.load(std::memory_order_relaxed) == kTotalCount);
  REQUIRE(consumed.load(std::memory_order_relaxed) == kTotalCount);
}

TEST_CASE("LockFreeMPSC blowup tiny capacity maximum contention",
          "[comet][ring_queue][stress][blowup][.]") {
  const auto hw{std::thread::hardware_concurrency()};
  const comet::usize kProducerCount{hw == 0 ? 8
                                            : static_cast<comet::usize>(hw)};
  constexpr comet::usize kItemsPerProducer{2'000'000};
  constexpr comet::usize kQueueCapacity{2};
  const auto kTotalCount{kProducerCount * kItemsPerProducer};

  comet::LockFreeMPSCRingQueue<comet::usize> queue{
      &comet::comettests::allocator, kQueueCapacity};

  std::atomic<comet::usize> produced{0};
  std::atomic<comet::usize> consumed{0};
  std::atomic<comet::usize> duplicate_count{0};
  std::atomic<comet::usize> invalid_count{0};
  std::atomic<bool> done{false};

  std::unique_ptr<std::atomic<bool>[]> seen{new std::atomic<bool>[kTotalCount]};

  for (comet::usize i{0}; i < kTotalCount; ++i) {
    seen[i].store(false, std::memory_order_relaxed);
  }

  std::vector<std::thread> producers;

  for (comet::usize producer{0}; producer < kProducerCount; ++producer) {
    producers.emplace_back([&, producer] {
      for (comet::usize i{0}; i < kItemsPerProducer; ++i) {
        const auto value{producer * kItemsPerProducer + i};

        while (!queue.TryPush(value)) {
          std::this_thread::yield();
        }

        produced.fetch_add(1, std::memory_order_relaxed);
      }
    });
  }

  std::thread consumer{[&] {
    comet::usize value{0};

    while (!done.load(std::memory_order_acquire) ||
           consumed.load(std::memory_order_relaxed) < kTotalCount) {
      if (!queue.TryPop(value)) {
        std::this_thread::yield();
        continue;
      }

      if (value >= kTotalCount) {
        invalid_count.fetch_add(1, std::memory_order_relaxed);
        consumed.fetch_add(1, std::memory_order_relaxed);
        continue;
      }

      bool expected{false};

      if (!seen[value].compare_exchange_strong(expected, true,
                                               std::memory_order_acq_rel,
                                               std::memory_order_relaxed)) {
        duplicate_count.fetch_add(1, std::memory_order_relaxed);
      }

      consumed.fetch_add(1, std::memory_order_relaxed);
    }
  }};

  for (auto& producer : producers) {
    producer.join();
  }

  done.store(true, std::memory_order_release);
  consumer.join();

  REQUIRE(invalid_count.load(std::memory_order_relaxed) == 0);
  REQUIRE(duplicate_count.load(std::memory_order_relaxed) == 0);
  REQUIRE(produced.load(std::memory_order_relaxed) == kTotalCount);
  REQUIRE(consumed.load(std::memory_order_relaxed) == kTotalCount);
}

TEST_CASE("LockFreeMPMC pressure multiple producers multiple consumers",
          "[comet][ring_queue][stress][.]") {
  const auto kThreadCount{comet::comettests::GetStressThreadCount()};
  const auto kProducerCount{kThreadCount};
  const auto kConsumerCount{kThreadCount};
  constexpr comet::usize kItemsPerProducer{4096};
  constexpr comet::usize kQueueCapacity{1024};
  const auto kTotalCount{kProducerCount * kItemsPerProducer};

  comet::LockFreeMPMCRingQueue<comet::usize> queue{
      &comet::comettests::allocator, kQueueCapacity};

  std::atomic<comet::usize> produced{0};
  std::atomic<comet::usize> consumed{0};
  std::atomic<comet::usize> duplicate_count{0};
  std::atomic<comet::usize> invalid_count{0};
  std::atomic<bool> producers_done{false};

  std::unique_ptr<std::atomic<bool>[]> seen{new std::atomic<bool>[kTotalCount]};

  for (comet::usize i{0}; i < kTotalCount; ++i) {
    seen[i].store(false, std::memory_order_relaxed);
  }

  std::vector<std::thread> producers;
  std::vector<std::thread> consumers;

  for (comet::usize producer{0}; producer < kProducerCount; ++producer) {
    producers.emplace_back([&, producer] {
      for (comet::usize i{0}; i < kItemsPerProducer; ++i) {
        const auto value{producer * kItemsPerProducer + i};

        while (!queue.TryPush(value)) {
          std::this_thread::yield();
        }

        produced.fetch_add(1, std::memory_order_relaxed);
      }
    });
  }

  for (comet::usize consumer{0}; consumer < kConsumerCount; ++consumer) {
    consumers.emplace_back([&] {
      while (!producers_done.load(std::memory_order_acquire) ||
             consumed.load(std::memory_order_relaxed) < kTotalCount) {
        auto value{queue.TryPop()};

        if (!value.has_value()) {
          std::this_thread::yield();
          continue;
        }

        if (*value >= kTotalCount) {
          invalid_count.fetch_add(1, std::memory_order_relaxed);
          consumed.fetch_add(1, std::memory_order_relaxed);
          continue;
        }

        bool expected{false};

        if (!seen[*value].compare_exchange_strong(expected, true,
                                                  std::memory_order_acq_rel,
                                                  std::memory_order_relaxed)) {
          duplicate_count.fetch_add(1, std::memory_order_relaxed);
        }

        consumed.fetch_add(1, std::memory_order_relaxed);
      }
    });
  }

  for (auto& producer : producers) {
    producer.join();
  }

  producers_done.store(true, std::memory_order_release);

  for (auto& consumer : consumers) {
    consumer.join();
  }

  REQUIRE(invalid_count.load(std::memory_order_relaxed) == 0);
  REQUIRE(duplicate_count.load(std::memory_order_relaxed) == 0);
  REQUIRE(produced.load(std::memory_order_relaxed) == kTotalCount);
  REQUIRE(consumed.load(std::memory_order_relaxed) == kTotalCount);
}

TEST_CASE("LockFreeMPMC soak tiny capacity high contention",
          "[comet][ring_queue][stress][soak][.]") {
  const auto kThreadCount{comet::comettests::GetStressThreadCount()};
  const auto kProducerCount{kThreadCount};
  const auto kConsumerCount{kThreadCount};
  constexpr comet::usize kItemsPerProducer{32768};
  constexpr comet::usize kQueueCapacity{8};
  const auto kTotalCount{kProducerCount * kItemsPerProducer};

  comet::LockFreeMPMCRingQueue<comet::usize> queue{
      &comet::comettests::allocator, kQueueCapacity};

  std::atomic<comet::usize> produced{0};
  std::atomic<comet::usize> consumed{0};
  std::atomic<comet::usize> duplicate_count{0};
  std::atomic<comet::usize> invalid_count{0};
  std::atomic<bool> producers_done{false};

  std::unique_ptr<std::atomic<bool>[]> seen{new std::atomic<bool>[kTotalCount]};

  for (comet::usize i{0}; i < kTotalCount; ++i) {
    seen[i].store(false, std::memory_order_relaxed);
  }

  std::vector<std::thread> producers;
  std::vector<std::thread> consumers;

  for (comet::usize producer{0}; producer < kProducerCount; ++producer) {
    producers.emplace_back([&, producer] {
      for (comet::usize i{0}; i < kItemsPerProducer; ++i) {
        const auto value{producer * kItemsPerProducer + i};

        while (!queue.TryPush(value)) {
          std::this_thread::yield();
        }

        produced.fetch_add(1, std::memory_order_relaxed);
      }
    });
  }

  for (comet::usize consumer{0}; consumer < kConsumerCount; ++consumer) {
    consumers.emplace_back([&] {
      while (!producers_done.load(std::memory_order_acquire) ||
             consumed.load(std::memory_order_relaxed) < kTotalCount) {
        auto value{queue.TryPop()};

        if (!value.has_value()) {
          std::this_thread::yield();
          continue;
        }

        if (*value >= kTotalCount) {
          invalid_count.fetch_add(1, std::memory_order_relaxed);
          consumed.fetch_add(1, std::memory_order_relaxed);
          continue;
        }

        bool expected{false};

        if (!seen[*value].compare_exchange_strong(expected, true,
                                                  std::memory_order_acq_rel,
                                                  std::memory_order_relaxed)) {
          duplicate_count.fetch_add(1, std::memory_order_relaxed);
        }

        consumed.fetch_add(1, std::memory_order_relaxed);
      }
    });
  }

  for (auto& producer : producers) {
    producer.join();
  }

  producers_done.store(true, std::memory_order_release);

  for (auto& consumer : consumers) {
    consumer.join();
  }

  REQUIRE(invalid_count.load(std::memory_order_relaxed) == 0);
  REQUIRE(duplicate_count.load(std::memory_order_relaxed) == 0);
  REQUIRE(produced.load(std::memory_order_relaxed) == kTotalCount);
  REQUIRE(consumed.load(std::memory_order_relaxed) == kTotalCount);
}

TEST_CASE("LockFreeMPMC blowup tiny capacity maximum contention",
          "[comet][ring_queue][stress][blowup][.]") {
  const auto hw{std::thread::hardware_concurrency()};
  const comet::usize kThreadCount{hw == 0 ? 8 : static_cast<comet::usize>(hw)};
  const auto kProducerCount{kThreadCount};
  const auto kConsumerCount{kThreadCount};
  constexpr comet::usize kItemsPerProducer{2'000'000};
  constexpr comet::usize kQueueCapacity{2};
  const auto kTotalCount{kProducerCount * kItemsPerProducer};

  comet::LockFreeMPMCRingQueue<comet::usize> queue{
      &comet::comettests::allocator, kQueueCapacity};

  std::atomic<comet::usize> produced{0};
  std::atomic<comet::usize> consumed{0};
  std::atomic<comet::usize> duplicate_count{0};
  std::atomic<comet::usize> invalid_count{0};
  std::atomic<bool> producers_done{false};

  std::unique_ptr<std::atomic<bool>[]> seen{new std::atomic<bool>[kTotalCount]};

  for (comet::usize i{0}; i < kTotalCount; ++i) {
    seen[i].store(false, std::memory_order_relaxed);
  }

  std::vector<std::thread> producers;
  std::vector<std::thread> consumers;

  for (comet::usize producer{0}; producer < kProducerCount; ++producer) {
    producers.emplace_back([&, producer] {
      for (comet::usize i{0}; i < kItemsPerProducer; ++i) {
        const auto value{producer * kItemsPerProducer + i};

        while (!queue.TryPush(value)) {
          std::this_thread::yield();
        }

        produced.fetch_add(1, std::memory_order_relaxed);
      }
    });
  }

  for (comet::usize consumer{0}; consumer < kConsumerCount; ++consumer) {
    consumers.emplace_back([&] {
      while (!producers_done.load(std::memory_order_acquire) ||
             consumed.load(std::memory_order_relaxed) < kTotalCount) {
        auto value{queue.TryPop()};

        if (!value.has_value()) {
          std::this_thread::yield();
          continue;
        }

        if (*value >= kTotalCount) {
          invalid_count.fetch_add(1, std::memory_order_relaxed);
          consumed.fetch_add(1, std::memory_order_relaxed);
          continue;
        }

        bool expected{false};

        if (!seen[*value].compare_exchange_strong(expected, true,
                                                  std::memory_order_acq_rel,
                                                  std::memory_order_relaxed)) {
          duplicate_count.fetch_add(1, std::memory_order_relaxed);
        }

        consumed.fetch_add(1, std::memory_order_relaxed);
      }
    });
  }

  for (auto& producer : producers) {
    producer.join();
  }

  producers_done.store(true, std::memory_order_release);

  for (auto& consumer : consumers) {
    consumer.join();
  }

  REQUIRE(invalid_count.load(std::memory_order_relaxed) == 0);
  REQUIRE(duplicate_count.load(std::memory_order_relaxed) == 0);
  REQUIRE(produced.load(std::memory_order_relaxed) == kTotalCount);
  REQUIRE(consumed.load(std::memory_order_relaxed) == kTotalCount);
}