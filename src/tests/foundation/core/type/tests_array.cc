// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "tests_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Tested. /////////////////////////////////////////////////////////////////////
#include "comet/core/container/array.h"
////////////////////////////////////////////////////////////////////////////////

// External. ///////////////////////////////////////////////////////////////////
#include "catch.hpp"
////////////////////////////////////////////////////////////////////////////////

#include "comet/runtime/memory/allocator/platform_allocator.h"
#include "comet/core/memory/memory.h"
#include "comet/runtime/memory/memory_tag.h"

namespace comet {
namespace comettests {
namespace memory {
enum TestsMemoryTag : comet::memory::MemoryTag {
  kTestsMemoryTagArray = kEngineMemoryTagUserBase + 1
};
}  // namespace memory

namespace internal {
comet::memory::PlatformAllocator array_allocator{memory::kTestsMemoryTagArray};

struct TrackedArrayValue {
  inline static s32 alive_count{0};

  s32 value{0};

  TrackedArrayValue() { ++alive_count; }
  explicit TrackedArrayValue(s32 value) : value{value} { ++alive_count; }

  TrackedArrayValue(const TrackedArrayValue& other) : value{other.value} {
    ++alive_count;
  }

  TrackedArrayValue(TrackedArrayValue&& other) noexcept : value{other.value} {
    other.value = -1;
    ++alive_count;
  }

  ~TrackedArrayValue() { --alive_count; }

  TrackedArrayValue& operator=(const TrackedArrayValue& other) {
    value = other.value;
    return *this;
  }

  bool operator==(const TrackedArrayValue& other) const {
    return value == other.value;
  }
};
}  // namespace internal
}  // namespace comettests
}  // namespace comet

TEST_CASE("Array creation", "[comet]") {
  comet::Array<comet::s32> array{&comet::comettests::internal::array_allocator};

  REQUIRE(array.GetSize() == 0);
  REQUIRE(array.GetCapacity() == 0);
  REQUIRE(array.IsEmpty());
  REQUIRE(array.GetAllocator() ==
          &comet::comettests::internal::array_allocator);
}

TEST_CASE("Array creation with capacity", "[comet]") {
  auto array{comet::Array<comet::s32>::WithCapacity(
      &comet::comettests::internal::array_allocator, 8)};

  REQUIRE(array.GetSize() == 0);
  REQUIRE(array.GetCapacity() == 8);
  REQUIRE(array.IsEmpty());
}

TEST_CASE("Array push and access", "[comet]") {
  comet::Array<comet::s32> array{&comet::comettests::internal::array_allocator};

  array.PushLast(1);
  array.PushLast(2);
  array.PushLast(3);

  REQUIRE(array.GetSize() == 3);
  REQUIRE(array[0] == 1);
  REQUIRE(array.Get(1) == 2);
  REQUIRE(array.GetLast() == 3);
}

TEST_CASE("Array set", "[comet]") {
  comet::Array<comet::s32> array{&comet::comettests::internal::array_allocator};

  array.PushLast(1);
  array.PushLast(2);

  auto& value{array.Set(1, 42)};

  REQUIRE(value == 42);
  REQUIRE(array[1] == 42);
}

TEST_CASE("Array resize grows and clears trivial values", "[comet]") {
  auto array{comet::Array<comet::s32>::WithCapacity(
      &comet::comettests::internal::array_allocator, 2)};

  array.Resize(4);

  REQUIRE(array.GetSize() == 4);
  REQUIRE(array.GetCapacity() >= 4);

  for (const auto value : array) {
    REQUIRE(value == 0);
  }
}

TEST_CASE("Array resize shrinks", "[comet]") {
  comet::Array<comet::s32> array{&comet::comettests::internal::array_allocator};

  array.PushLast(1);
  array.PushLast(2);
  array.PushLast(3);

  array.Resize(1);

  REQUIRE(array.GetSize() == 1);
  REQUIRE(array[0] == 1);
}

TEST_CASE("Array remove operations", "[comet]") {
  comet::Array<comet::s32> array{&comet::comettests::internal::array_allocator};

  array.PushLast(1);
  array.PushLast(2);
  array.PushLast(3);

  array.RemoveFromIndex(1);

  REQUIRE(array.GetSize() == 2);
  REQUIRE(array[0] == 1);
  REQUIRE(array[1] == 3);

  array.RemoveFromValue(1);

  REQUIRE(array.GetSize() == 1);
  REQUIRE(array[0] == 3);
}

TEST_CASE("Array copy constructor performs deep copy", "[comet]") {
  comet::Array<comet::s32> array{&comet::comettests::internal::array_allocator};

  array.PushLast(1);
  array.PushLast(2);

  auto copy{array};
  copy.Set(0, 42);

  REQUIRE(array[0] == 1);
  REQUIRE(copy[0] == 42);
}

TEST_CASE("Array move constructor steals storage", "[comet]") {
  comet::Array<comet::s32> array{&comet::comettests::internal::array_allocator};

  array.PushLast(1);
  array.PushLast(2);

  auto moved{std::move(array)};

  REQUIRE(moved.GetSize() == 2);
  REQUIRE(moved[0] == 1);
  REQUIRE(moved[1] == 2);

  REQUIRE(array.GetSize() == 0);
  REQUIRE(array.GetData() == nullptr);
}

TEST_CASE("Array take last", "[comet]") {
  comet::Array<comet::s32> array{&comet::comettests::internal::array_allocator};

  array.PushLast(1);
  array.PushLast(2);

  const auto value{array.TakeLast()};

  REQUIRE(value == 2);
  REQUIRE(array.GetSize() == 1);
  REQUIRE(array.GetLast() == 1);
}

TEST_CASE("Array push from range", "[comet]") {
  comet::s32 source[]{1, 2, 3};

  comet::Array<comet::s32> array{&comet::comettests::internal::array_allocator};
  array.PushFromRange(source, 3);

  REQUIRE(array.GetSize() == 3);
  REQUIRE(array[0] == 1);
  REQUIRE(array[1] == 2);
  REQUIRE(array[2] == 3);
}

TEST_CASE("StaticArray creation and access", "[comet]") {
  comet::StaticArray<comet::s32, 3> array{1, 2, 3};

  REQUIRE(array.GetSize() == 3);
  REQUIRE(!array.IsEmpty());
  REQUIRE(array[0] == 1);
  REQUIRE(array.GetFirst() == 1);
  REQUIRE(array.GetLast() == 3);
}

TEST_CASE("StaticArray set", "[comet]") {
  comet::StaticArray<comet::s32, 3> array{1, 2, 3};

  auto& value{array.Set(1, 42)};

  REQUIRE(value == 42);
  REQUIRE(array[1] == 42);
}

TEST_CASE("StaticArray contains and index", "[comet]") {
  comet::StaticArray<comet::s32, 3> array{1, 2, 3};

  REQUIRE(array.IsContained(2));
  REQUIRE(!array.IsContained(42));
  REQUIRE(array.GetIndex(3) == 2);
  REQUIRE(array.GetIndex(42) == comet::kInvalidIndex);
}

TEST_CASE("StaticArray zero size", "[comet]") {
  comet::StaticArray<comet::s32, 0> array{};

  REQUIRE(array.GetSize() == 0);
  REQUIRE(array.IsEmpty());
  REQUIRE(array.GetData() == nullptr);
}

TEST_CASE("Array reserve does not change size", "[comet]") {
  comet::Array<comet::s32> array{&comet::comettests::internal::array_allocator};

  array.PushLast(1);
  array.Reserve(16);

  REQUIRE(array.GetSize() == 1);
  REQUIRE(array.GetCapacity() >= 16);
  REQUIRE(array[0] == 1);
}

TEST_CASE("Array clear keeps capacity", "[comet]") {
  auto array{comet::Array<comet::s32>::WithCapacity(
      &comet::comettests::internal::array_allocator, 8)};

  array.PushLast(1);
  array.PushLast(2);

  array.Clear();

  REQUIRE(array.GetSize() == 0);
  REQUIRE(array.GetCapacity() == 8);
  REQUIRE(array.IsEmpty());
}

TEST_CASE("Array release frees storage", "[comet]") {
  auto array{comet::Array<comet::s32>::WithCapacity(
      &comet::comettests::internal::array_allocator, 8)};

  array.PushLast(1);
  array.Release();

  REQUIRE(array.GetSize() == 0);
  REQUIRE(array.GetCapacity() == 0);
  REQUIRE(array.GetData() == nullptr);
}

TEST_CASE("Array remove first and last", "[comet]") {
  comet::Array<comet::s32> array{&comet::comettests::internal::array_allocator};

  array.PushLast(1);
  array.PushLast(2);
  array.PushLast(3);

  array.RemoveFromIndex(0);
  REQUIRE(array.GetSize() == 2);
  REQUIRE(array[0] == 2);
  REQUIRE(array[1] == 3);

  array.RemoveFromIndex(array.GetSize() - 1);
  REQUIRE(array.GetSize() == 1);
  REQUIRE(array[0] == 2);
}

TEST_CASE("Array remove missing value is no-op", "[comet]") {
  comet::Array<comet::s32> array{&comet::comettests::internal::array_allocator};

  array.PushLast(1);
  array.PushLast(2);

  array.RemoveFromValue(42);

  REQUIRE(array.GetSize() == 2);
  REQUIRE(array[0] == 1);
  REQUIRE(array[1] == 2);
}

TEST_CASE("Array push range appends to existing values", "[comet]") {
  comet::s32 source[]{2, 3, 4};

  comet::Array<comet::s32> array{&comet::comettests::internal::array_allocator};
  array.PushLast(1);
  array.PushFromRange(source, 3);

  REQUIRE(array.GetSize() == 4);
  REQUIRE(array[0] == 1);
  REQUIRE(array[1] == 2);
  REQUIRE(array[2] == 3);
  REQUIRE(array[3] == 4);
}

TEST_CASE("Array copy assignment performs deep copy", "[comet]") {
  comet::Array<comet::s32> array{&comet::comettests::internal::array_allocator};
  comet::Array<comet::s32> copy{&comet::comettests::internal::array_allocator};

  array.PushLast(1);
  array.PushLast(2);

  copy = array;
  copy.Set(0, 42);

  REQUIRE(array[0] == 1);
  REQUIRE(copy[0] == 42);
  REQUIRE(copy[1] == 2);
}

TEST_CASE("Array move assignment steals storage", "[comet]") {
  comet::Array<comet::s32> array{&comet::comettests::internal::array_allocator};
  comet::Array<comet::s32> moved{&comet::comettests::internal::array_allocator};

  array.PushLast(1);
  array.PushLast(2);

  moved = std::move(array);

  REQUIRE(moved.GetSize() == 2);
  REQUIRE(moved[0] == 1);
  REQUIRE(moved[1] == 2);
  REQUIRE(array.GetSize() == 0);
  REQUIRE(array.GetData() == nullptr);
}

TEST_CASE("Array supports non-trivial values", "[comet]") {
  comet::comettests::internal::TrackedArrayValue::alive_count = 0;

  {
    comet::Array<comet::comettests::internal::TrackedArrayValue> array{
        &comet::comettests::internal::array_allocator};

    array.EmplaceLast(1);
    array.EmplaceLast(2);
    array.Resize(4);

    REQUIRE(array.GetSize() == 4);
    REQUIRE(array[0].value == 1);
    REQUIRE(array[1].value == 2);
    REQUIRE(array[2].value == 0);
    REQUIRE(array[3].value == 0);

    array.RemoveFromIndex(0);
    REQUIRE(array.GetSize() == 3);
    REQUIRE(array[0].value == 2);
    REQUIRE(array[1].value == 0);
    REQUIRE(array[2].value == 0);
  }

  REQUIRE(comet::comettests::internal::TrackedArrayValue::alive_count == 0);
}