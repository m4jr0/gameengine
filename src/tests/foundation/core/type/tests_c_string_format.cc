// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "comet/core/string/c_string.h"
#include "catch2/catch_test_macros.hpp"

TEST_CASE("Core formatting writes caller-owned buffers", "[comet][string]") {
  comet::schar buffer[32]{};

  const auto required{comet::FormatToBuffer(buffer, sizeof(buffer),
                                            "%s %d", "asset", 42)};

  REQUIRE(required == 8);
  REQUIRE(comet::AreStringsEqual(buffer, "asset 42"));
}

TEST_CASE("Core formatting reports truncation", "[comet][string]") {
  comet::schar buffer[5]{};

  const auto required{comet::FormatToBuffer(buffer, sizeof(buffer),
                                            "%s", "abcdef")};

  REQUIRE(required == 6);
  REQUIRE(comet::AreStringsEqual(buffer, "abcd"));
}

TEST_CASE("Core formatting rejects invalid buffers", "[comet][string]") {
  comet::schar buffer[4]{};

  REQUIRE(comet::FormatToBuffer(buffer, 0, "%s", "abc") < 0);
  REQUIRE(comet::FormatToBuffer(nullptr, 4, "%s", "abc") < 0);
  REQUIRE(comet::FormatToBuffer(buffer, sizeof(buffer), nullptr) < 0);
}
