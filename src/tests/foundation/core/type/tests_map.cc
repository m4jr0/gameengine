// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Tested. /////////////////////////////////////////////////////////////////////
#include "comet/core/type/map.h"
////////////////////////////////////////////////////////////////////////////////

// External. ///////////////////////////////////////////////////////////////////
#include "catch.hpp"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/memory/allocator/platform_allocator.h"
#include "comet/core/memory/memory.h"

namespace comet {
namespace comettests {
namespace memory {
enum TestsMemoryTag : comet::memory::MemoryTag {
  kTestsMemoryTagMap = comet::memory::kEngineMemoryTagUserBase + 4
};
}  // namespace memory

namespace internal {
comet::memory::PlatformAllocator map_allocator{memory::kTestsMemoryTagMap};
}  // namespace internal
}  // namespace comettests
}  // namespace comet

TEST_CASE("Map creation", "[comet]") {
  comet::Map<comet::s32, comet::s32> map{
      &comet::comettests::internal::map_allocator};

  REQUIRE(map.IsEmpty());
  REQUIRE(map.GetEntryCount() == 0);
}

TEST_CASE("Map set and get", "[comet]") {
  comet::Map<comet::s32, comet::s32> map{
      &comet::comettests::internal::map_allocator};

  map.Set(1, 42);
  map.Set(2, 84);

  REQUIRE(map.GetEntryCount() == 2);
  REQUIRE(map.Get(1) == 42);
  REQUIRE(map.Get(2) == 84);
}

TEST_CASE("Map set replaces existing value", "[comet]") {
  comet::Map<comet::s32, comet::s32> map{
      &comet::comettests::internal::map_allocator};

  map.Set(1, 42);
  map.Set(1, 84);

  REQUIRE(map.GetEntryCount() == 1);
  REQUIRE(map.Get(1) == 84);
}

TEST_CASE("Map try get", "[comet]") {
  comet::Map<comet::s32, comet::s32> map{
      &comet::comettests::internal::map_allocator};

  map.Set(1, 42);

  auto* value{map.TryGet(1)};

  REQUIRE(value != nullptr);
  REQUIRE(*value == 42);
  REQUIRE(map.TryGet(2) == nullptr);
}

TEST_CASE("Map get or add", "[comet]") {
  comet::Map<comet::s32, comet::s32> map{
      &comet::comettests::internal::map_allocator};

  auto& value{map.GetOrAdd(1)};
  value = 42;

  REQUIRE(map.Get(1) == 42);
  REQUIRE(map.GetEntryCount() == 1);

  auto& same_value{map.GetOrAdd(1)};

  REQUIRE(same_value == 42);
  REQUIRE(map.GetEntryCount() == 1);
}

TEST_CASE("Map remove", "[comet]") {
  comet::Map<comet::s32, comet::s32> map{
      &comet::comettests::internal::map_allocator};

  map.Set(1, 42);
  map.Set(2, 84);

  REQUIRE(map.Remove(1));
  REQUIRE(map.TryGet(1) == nullptr);
  REQUIRE(map.TryGet(2) != nullptr);
  REQUIRE(map.GetEntryCount() == 1);

  REQUIRE(!map.Remove(42));
}

TEST_CASE("Map clear", "[comet]") {
  comet::Map<comet::s32, comet::s32> map{
      &comet::comettests::internal::map_allocator};

  map.Set(1, 42);
  map.Set(2, 84);
  map.Clear();

  REQUIRE(map.IsEmpty());
  REQUIRE(map.GetEntryCount() == 0);
}

TEST_CASE("Map iteration", "[comet]") {
  comet::Map<comet::s32, comet::s32> map{
      &comet::comettests::internal::map_allocator};

  map.Set(1, 10);
  map.Set(2, 20);
  map.Set(3, 30);

  comet::s32 sum{0};

  for (const auto& pair : map) {
    sum += pair.value;
  }

  REQUIRE(sum == 60);
}

TEST_CASE("Map copy performs deep copy", "[comet]") {
  comet::Map<comet::s32, comet::s32> map{
      &comet::comettests::internal::map_allocator};

  map.Set(1, 42);

  auto copy{map};
  copy.Set(1, 84);
  copy.Set(2, 168);

  REQUIRE(map.Get(1) == 42);
  REQUIRE(map.TryGet(2) == nullptr);

  REQUIRE(copy.Get(1) == 84);
  REQUIRE(copy.Get(2) == 168);
}

TEST_CASE("Map move preserves entries", "[comet]") {
  comet::Map<comet::s32, comet::s32> map{
      &comet::comettests::internal::map_allocator};

  map.Set(1, 42);
  map.Set(2, 84);

  auto moved{std::move(map)};

  REQUIRE(moved.GetEntryCount() == 2);
  REQUIRE(moved.Get(1) == 42);
  REQUIRE(moved.Get(2) == 84);
}

TEST_CASE("Map grows and preserves entries", "[comet]") {
  comet::Map<comet::s32, comet::s32> map{
      &comet::comettests::internal::map_allocator};

  constexpr comet::s32 kCount{1024};

  for (comet::s32 i{0}; i < kCount; ++i) {
    map.Set(i, i * 10);
  }

  REQUIRE(map.GetEntryCount() == kCount);

  for (comet::s32 i{0}; i < kCount; ++i) {
    REQUIRE(map.Get(i) == i * 10);
  }
}

TEST_CASE("Map remove many then add many", "[comet]") {
  comet::Map<comet::s32, comet::s32> map{
      &comet::comettests::internal::map_allocator};

  constexpr comet::s32 kCount{256};

  for (comet::s32 i{0}; i < kCount; ++i) {
    map.Set(i, i);
  }

  for (comet::s32 i{0}; i < kCount; i += 2) {
    REQUIRE(map.Remove(i));
  }

  for (comet::s32 i{kCount}; i < kCount * 2; ++i) {
    map.Set(i, i);
  }

  for (comet::s32 i{0}; i < kCount; ++i) {
    if (i % 2 == 0) {
      REQUIRE(map.TryGet(i) == nullptr);
    } else {
      REQUIRE(map.Get(i) == i);
    }
  }

  for (comet::s32 i{kCount}; i < kCount * 2; ++i) {
    REQUIRE(map.Get(i) == i);
  }
}

TEST_CASE("Map clear then reuse", "[comet]") {
  comet::Map<comet::s32, comet::s32> map{
      &comet::comettests::internal::map_allocator};

  map.Set(1, 10);
  map.Set(2, 20);
  map.Clear();

  map.Set(3, 30);

  REQUIRE(map.GetEntryCount() == 1);
  REQUIRE(map.TryGet(1) == nullptr);
  REQUIRE(map.TryGet(2) == nullptr);
  REQUIRE(map.Get(3) == 30);
}

TEST_CASE("Map TryGet allows value mutation", "[comet]") {
  comet::Map<comet::s32, comet::s32> map{
      &comet::comettests::internal::map_allocator};

  map.Set(1, 10);

  auto* value{map.TryGet(1)};
  REQUIRE(value != nullptr);

  *value = 99;

  REQUIRE(map.Get(1) == 99);
}

TEST_CASE("Map GetOrAdd default initializes missing value", "[comet]") {
  comet::Map<comet::s32, comet::s32> map{
      &comet::comettests::internal::map_allocator};

  auto& value{map.GetOrAdd(42)};

  REQUIRE(value == 0);
  REQUIRE(map.GetEntryCount() == 1);
  REQUIRE(map.Get(42) == 0);
}

TEST_CASE("Map iteration visits each entry once", "[comet]") {
  comet::Map<comet::s32, comet::s32> map{
      &comet::comettests::internal::map_allocator};

  map.Set(1, 10);
  map.Set(2, 20);
  map.Set(3, 30);

  comet::usize count{0};
  comet::s32 key_sum{0};
  comet::s32 value_sum{0};

  for (const auto& pair : map) {
    ++count;
    key_sum += pair.key;
    value_sum += pair.value;
  }

  REQUIRE(count == 3);
  REQUIRE(key_sum == 6);
  REQUIRE(value_sum == 60);
}

TEST_CASE("Map copy assignment performs deep copy", "[comet]") {
  comet::Map<comet::s32, comet::s32> map{
      &comet::comettests::internal::map_allocator};
  comet::Map<comet::s32, comet::s32> copy{
      &comet::comettests::internal::map_allocator};

  map.Set(1, 42);

  copy = map;
  copy.Set(1, 84);
  copy.Set(2, 168);

  REQUIRE(map.Get(1) == 42);
  REQUIRE(map.TryGet(2) == nullptr);

  REQUIRE(copy.Get(1) == 84);
  REQUIRE(copy.Get(2) == 168);
}

TEST_CASE("Map move assignment preserves entries", "[comet]") {
  comet::Map<comet::s32, comet::s32> map{
      &comet::comettests::internal::map_allocator};
  comet::Map<comet::s32, comet::s32> moved{
      &comet::comettests::internal::map_allocator};

  map.Set(1, 42);
  map.Set(2, 84);

  moved = std::move(map);

  REQUIRE(moved.GetEntryCount() == 2);
  REQUIRE(moved.Get(1) == 42);
  REQUIRE(moved.Get(2) == 84);
}