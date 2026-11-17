// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include <cstdint>

#include "catch2/catch_test_macros.hpp"
#include "comet/core/memory/allocator/system_allocator.h"

TEST_CASE("System allocator supports all engine alignments",
          "[comet][memory]") {
  comet::memory::SystemAllocator allocator{};
  for (comet::memory::Alignment align{1};
       align <= comet::memory::kMaxAlignment; align *= 2) {
    auto* ptr{allocator.AllocateAligned(97, align)};
    REQUIRE(ptr != nullptr);
    REQUIRE(reinterpret_cast<std::uintptr_t>(ptr) % align == 0);
    allocator.Deallocate(ptr);
  }
}

TEST_CASE("System allocator supports multiple live allocations",
          "[comet][memory]") {
  comet::memory::SystemAllocator allocator{};
  auto* first{allocator.AllocateAligned(64, 16)};
  auto* second{allocator.AllocateAligned(1024, 256)};
  REQUIRE(first != nullptr);
  REQUIRE(second != nullptr);
  REQUIRE(first != second);
  allocator.Deallocate(second);
  allocator.Deallocate(first);
  allocator.Deallocate(nullptr);
}
