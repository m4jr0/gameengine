// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Tested. /////////////////////////////////////////////////////////////////////
#include "comet/core/type/hash_set.h"
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
  kTestsMemoryTagHashSet = comet::memory::kEngineMemoryTagUserBase + 3
};
}  // namespace memory

namespace internal {
comet::memory::PlatformAllocator hash_set_allocator{
    memory::kTestsMemoryTagHashSet};
}  // namespace internal
}  // namespace comettests
}  // namespace comet

TEST_CASE("HashSet creation", "[comet]") {
  comet::HashSet<comet::s32> set{
      &comet::comettests::internal::hash_set_allocator};

  REQUIRE(set.IsEmpty());
  REQUIRE(set.GetEntryCount() == 0);
}

TEST_CASE("HashSet add and contains", "[comet]") {
  comet::HashSet<comet::s32> set{
      &comet::comettests::internal::hash_set_allocator};

  set.Add(1);
  set.Add(2);
  set.Add(2);

  REQUIRE(set.GetEntryCount() == 2);
  REQUIRE(set.IsContained(1));
  REQUIRE(set.IsContained(2));
  REQUIRE(!set.IsContained(3));
}

TEST_CASE("HashSet set replaces existing value", "[comet]") {
  comet::HashSet<comet::s32> set{
      &comet::comettests::internal::hash_set_allocator};

  set.Set(1);
  set.Set(1);

  REQUIRE(set.GetEntryCount() == 1);
  REQUIRE(set.IsContained(1));
}

TEST_CASE("HashSet remove", "[comet]") {
  comet::HashSet<comet::s32> set{
      &comet::comettests::internal::hash_set_allocator};

  set.Add(1);
  set.Add(2);

  REQUIRE(set.Remove(1));
  REQUIRE(!set.IsContained(1));
  REQUIRE(set.IsContained(2));
  REQUIRE(set.GetEntryCount() == 1);

  REQUIRE(!set.Remove(42));
}

TEST_CASE("HashSet find", "[comet]") {
  comet::HashSet<comet::s32> set{
      &comet::comettests::internal::hash_set_allocator};

  set.Add(42);

  auto* value{set.Find(42)};

  REQUIRE(value != nullptr);
  REQUIRE(*value == 42);
  REQUIRE(set.Find(7) == nullptr);
}

TEST_CASE("HashSet pop", "[comet]") {
  comet::HashSet<comet::s32> set{
      &comet::comettests::internal::hash_set_allocator};

  set.Add(42);

  const auto value{set.Pop(42)};

  REQUIRE(value == 42);
  REQUIRE(set.IsEmpty());
}

TEST_CASE("HashSet clear", "[comet]") {
  comet::HashSet<comet::s32> set{
      &comet::comettests::internal::hash_set_allocator};

  set.Add(1);
  set.Add(2);
  set.Clear();

  REQUIRE(set.IsEmpty());
  REQUIRE(set.GetEntryCount() == 0);
}

TEST_CASE("HashSet copy performs deep copy", "[comet]") {
  comet::HashSet<comet::s32> set{
      &comet::comettests::internal::hash_set_allocator};

  set.Add(1);
  set.Add(2);

  auto copy{set};
  copy.Remove(1);
  copy.Add(3);

  REQUIRE(set.IsContained(1));
  REQUIRE(!set.IsContained(3));

  REQUIRE(!copy.IsContained(1));
  REQUIRE(copy.IsContained(3));
}

TEST_CASE("HashSet move preserves entries", "[comet]") {
  comet::HashSet<comet::s32> set{
      &comet::comettests::internal::hash_set_allocator};

  set.Add(1);
  set.Add(2);

  auto moved{std::move(set)};

  REQUIRE(moved.GetEntryCount() == 2);
  REQUIRE(moved.IsContained(1));
  REQUIRE(moved.IsContained(2));
}

TEST_CASE("HashSet grows and preserves entries", "[comet]") {
  comet::HashSet<comet::s32> set{
      &comet::comettests::internal::hash_set_allocator};

  constexpr comet::usize kCount{1024};

  for (comet::s32 i{0}; i < static_cast<comet::s32>(kCount); ++i) {
    set.Add(i);
  }

  REQUIRE(set.GetEntryCount() == kCount);

  for (comet::s32 i{0}; i < static_cast<comet::s32>(kCount); ++i) {
    REQUIRE(set.IsContained(i));
  }
}

TEST_CASE("HashSet clear then reuse", "[comet]") {
  comet::HashSet<comet::s32> set{
      &comet::comettests::internal::hash_set_allocator};

  set.Add(1);
  set.Add(2);
  set.Clear();

  set.Add(3);

  REQUIRE(set.GetEntryCount() == 1);
  REQUIRE(!set.IsContained(1));
  REQUIRE(!set.IsContained(2));
  REQUIRE(set.IsContained(3));
}

TEST_CASE("HashSet remove many then add many", "[comet]") {
  comet::HashSet<comet::s32> set{
      &comet::comettests::internal::hash_set_allocator};

  constexpr comet::s32 kCount{256};

  for (comet::s32 i{0}; i < kCount; ++i) {
    set.Add(i);
  }

  for (comet::s32 i{0}; i < kCount; i += 2) {
    REQUIRE(set.Remove(i));
  }

  for (comet::s32 i{kCount}; i < kCount * 2; ++i) {
    set.Add(i);
  }

  for (comet::s32 i{0}; i < kCount; ++i) {
    REQUIRE(set.IsContained(i) == (i % 2 != 0));
  }

  for (comet::s32 i{kCount}; i < kCount * 2; ++i) {
    REQUIRE(set.IsContained(i));
  }
}

TEST_CASE("HashSet iteration visits each entry once", "[comet]") {
  comet::HashSet<comet::s32> set{
      &comet::comettests::internal::hash_set_allocator};

  set.Add(1);
  set.Add(2);
  set.Add(3);

  comet::usize count{0};
  comet::s32 sum{0};

  for (const auto value : set) {
    ++count;
    sum += value;
  }

  REQUIRE(count == 3);
  REQUIRE(sum == 6);
}

TEST_CASE("HashSet copy assignment performs deep copy", "[comet]") {
  comet::HashSet<comet::s32> set{
      &comet::comettests::internal::hash_set_allocator};
  comet::HashSet<comet::s32> copy{
      &comet::comettests::internal::hash_set_allocator};

  set.Add(1);
  set.Add(2);

  copy = set;
  copy.Remove(1);
  copy.Add(3);

  REQUIRE(set.IsContained(1));
  REQUIRE(!set.IsContained(3));

  REQUIRE(!copy.IsContained(1));
  REQUIRE(copy.IsContained(3));
}

TEST_CASE("HashSet move assignment preserves entries", "[comet]") {
  comet::HashSet<comet::s32> set{
      &comet::comettests::internal::hash_set_allocator};
  comet::HashSet<comet::s32> moved{
      &comet::comettests::internal::hash_set_allocator};

  set.Add(1);
  set.Add(2);

  moved = std::move(set);

  REQUIRE(moved.GetEntryCount() == 2);
  REQUIRE(moved.IsContained(1));
  REQUIRE(moved.IsContained(2));
}