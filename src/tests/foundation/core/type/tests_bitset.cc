// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "tests_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Tested. /////////////////////////////////////////////////////////////////////
#include "comet/core/container/bitset.h"
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
  kTestsMemoryTagBitset = kEngineMemoryTagUserBase + 2
};
}  // namespace memory

namespace internal {
comet::memory::PlatformAllocator bitset_allocator{
    memory::kTestsMemoryTagBitset};
}  // namespace internal
}  // namespace comettests
}  // namespace comet

TEST_CASE("Bitset creation", "[comet]") {
  auto bitset{comet::Bitset::WithSize(
      &comet::comettests::internal::bitset_allocator, 128)};

  REQUIRE(bitset.GetSize() == 128);
  REQUIRE(bitset.GetWordCount() == 2);
}

TEST_CASE("Bitset set reset and test", "[comet]") {
  auto bitset{comet::Bitset::WithSize(
      &comet::comettests::internal::bitset_allocator, 128)};

  REQUIRE(!bitset.Test(0));
  REQUIRE(!bitset.Test(63));
  REQUIRE(!bitset.Test(64));

  bitset.Set(0);
  bitset.Set(63);
  bitset.Set(64);

  REQUIRE(bitset.Test(0));
  REQUIRE(bitset.Test(63));
  REQUIRE(bitset.Test(64));

  bitset.Reset(63);

  REQUIRE(bitset.Test(0));
  REQUIRE(!bitset.Test(63));
  REQUIRE(bitset.Test(64));
}

TEST_CASE("Bitset reset all", "[comet]") {
  auto bitset{comet::Bitset::WithSize(
      &comet::comettests::internal::bitset_allocator, 128)};

  bitset.Set(1);
  bitset.Set(127);
  bitset.ResetAll();

  REQUIRE(!bitset.Test(1));
  REQUIRE(!bitset.Test(127));
}

TEST_CASE("Bitset resize preserves old values", "[comet]") {
  auto bitset{comet::Bitset::WithSize(
      &comet::comettests::internal::bitset_allocator, 8)};

  bitset.Set(3);
  bitset.Resize(128);

  REQUIRE(bitset.GetSize() == 128);
  REQUIRE(bitset.Test(3));
  REQUIRE(!bitset.Test(127));
}

TEST_CASE("Bitset copy performs deep copy", "[comet]") {
  auto bitset{comet::Bitset::WithSize(
      &comet::comettests::internal::bitset_allocator, 64)};

  bitset.Set(7);

  auto copy{bitset};
  copy.Set(8);
  copy.Reset(7);

  REQUIRE(bitset.Test(7));
  REQUIRE(!bitset.Test(8));

  REQUIRE(!copy.Test(7));
  REQUIRE(copy.Test(8));
}

TEST_CASE("Bitset move steals storage", "[comet]") {
  auto bitset{comet::Bitset::WithSize(
      &comet::comettests::internal::bitset_allocator, 64)};

  bitset.Set(7);

  auto moved{std::move(bitset)};

  REQUIRE(moved.GetSize() == 64);
  REQUIRE(moved.Test(7));
  REQUIRE(bitset.GetSize() == 0);
}

TEST_CASE("Bitset zero size", "[comet]") {
  auto bitset{comet::Bitset::WithSize(
      &comet::comettests::internal::bitset_allocator, 0)};

  REQUIRE(bitset.GetSize() == 0);
  REQUIRE(bitset.GetWordCount() == 0);
}

TEST_CASE("Bitset resize within same word clears new bits", "[comet]") {
  auto bitset{comet::Bitset::WithSize(
      &comet::comettests::internal::bitset_allocator, 3)};

  bitset.Set(2);
  bitset.Resize(5);

  REQUIRE(bitset.Test(2));
  REQUIRE(!bitset.Test(3));
  REQUIRE(!bitset.Test(4));
}

TEST_CASE("Bitset resize across word boundary preserves and clears",
          "[comet]") {
  auto bitset{comet::Bitset::WithSize(
      &comet::comettests::internal::bitset_allocator, 63)};

  bitset.Set(62);
  bitset.Resize(65);

  REQUIRE(bitset.Test(62));
  REQUIRE(!bitset.Test(63));
  REQUIRE(!bitset.Test(64));
  REQUIRE(bitset.GetWordCount() == 2);
}

TEST_CASE("Bitset clear keeps size and clears all words", "[comet]") {
  auto bitset{comet::Bitset::WithSize(
      &comet::comettests::internal::bitset_allocator, 130)};

  bitset.Set(0);
  bitset.Set(64);
  bitset.Set(129);

  bitset.Clear();

  REQUIRE(bitset.GetSize() == 130);
  REQUIRE(!bitset.Test(0));
  REQUIRE(!bitset.Test(64));
  REQUIRE(!bitset.Test(129));
}

TEST_CASE("Bitset resize smaller is no-op", "[comet]") {
  auto bitset{comet::Bitset::WithSize(
      &comet::comettests::internal::bitset_allocator, 128)};

  bitset.Set(100);
  bitset.Resize(64);

  REQUIRE(bitset.GetSize() == 128);
  REQUIRE(bitset.Test(100));
}

TEST_CASE("Bitset assignment copy performs deep copy", "[comet]") {
  auto a{comet::Bitset::WithSize(&comet::comettests::internal::bitset_allocator,
                                 64)};
  auto b{comet::Bitset::WithSize(&comet::comettests::internal::bitset_allocator,
                                 64)};

  a.Set(3);
  b = a;
  b.Reset(3);
  b.Set(4);

  REQUIRE(a.Test(3));
  REQUIRE(!a.Test(4));
  REQUIRE(!b.Test(3));
  REQUIRE(b.Test(4));
}