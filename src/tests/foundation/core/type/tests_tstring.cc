// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "tests_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Tested. /////////////////////////////////////////////////////////////////////
#include "comet/core/string/tstring.h"
////////////////////////////////////////////////////////////////////////////////

// External. ///////////////////////////////////////////////////////////////////
#include "catch.hpp"
////////////////////////////////////////////////////////////////////////////////

TEST_CASE("TString creation from literal", "[comet]") {
  comet::TString str{COMET_TCHAR("hello")};

  REQUIRE(str.GetLength() == 5);
  REQUIRE(str.GetLengthWithNullTerminator() == 6);
  REQUIRE(str == COMET_TCHAR("hello"));
}

TEST_CASE("TString creation from char", "[comet]") {
  comet::TString str{COMET_TCHAR('a')};

  REQUIRE(str.GetLength() == 1);
  REQUIRE(str[0] == COMET_TCHAR('a'));
}

TEST_CASE("TString append", "[comet]") {
  comet::TString str{COMET_TCHAR("hello")};

  str.Append(COMET_TCHAR(" world"));

  REQUIRE(str == COMET_TCHAR("hello world"));
}

TEST_CASE("TString operator plus", "[comet]") {
  comet::TString lhs{COMET_TCHAR("hello")};
  comet::TString rhs{COMET_TCHAR(" world")};

  const auto result{lhs + rhs};

  REQUIRE(result == COMET_TCHAR("hello world"));
  REQUIRE(lhs == COMET_TCHAR("hello"));
  REQUIRE(rhs == COMET_TCHAR(" world"));
}

TEST_CASE("TString copy constructor performs deep copy", "[comet]") {
  comet::TString str{COMET_TCHAR("hello")};
  auto copy{str};

  copy[0] = COMET_TCHAR('H');

  REQUIRE(str == COMET_TCHAR("hello"));
  REQUIRE(copy == COMET_TCHAR("Hello"));
}

TEST_CASE("TString move constructor preserves value", "[comet]") {
  comet::TString str{COMET_TCHAR("hello")};

  auto moved{std::move(str)};

  REQUIRE(moved == COMET_TCHAR("hello"));
  REQUIRE(str.IsEmpty());
}

TEST_CASE("TString assignment from aliased CTStringView", "[comet]") {
  comet::TString str{COMET_TCHAR("hello world")};
  comet::CTStringView view{str.GetCTStr() + 6, 5};

  str = view;

  REQUIRE(str == COMET_TCHAR("world"));
}

TEST_CASE("TString substring", "[comet]") {
  comet::TString str{COMET_TCHAR("hello world")};

  const auto sub{str.GenerateSubString(6, 5)};

  REQUIRE(sub == COMET_TCHAR("world"));
  REQUIRE(sub.GetLength() == 5);
}

TEST_CASE("TString resize", "[comet]") {
  comet::TString str{COMET_TCHAR("hello")};

  str.Resize(2);

  REQUIRE(str == COMET_TCHAR("he"));
  REQUIRE(str.GetLength() == 2);

  str.Resize(5);

  REQUIRE(str.GetLength() == 5);
}

TEST_CASE("TString clear", "[comet]") {
  comet::TString str{COMET_TCHAR("hello")};

  str.Clear();

  REQUIRE(str.IsEmpty());
  REQUIRE(str.GetLength() == 0);
  REQUIRE(str.GetCTStr()[0] == COMET_TCHAR('\0'));
}

TEST_CASE("TString comparisons", "[comet]") {
  comet::TString a{COMET_TCHAR("abc")};
  comet::TString b{COMET_TCHAR("abd")};
  comet::TString c{COMET_TCHAR("abc")};

  REQUIRE(a == c);
  REQUIRE(a != b);
  REQUIRE(a < b);
  REQUIRE(b > a);
  REQUIRE(a <= c);
  REQUIRE(a >= c);
}

TEST_CASE("TString empty creation", "[comet]") {
  comet::TString str{};

  REQUIRE(str.IsEmpty());
  REQUIRE(str.GetLength() == 0);
  REQUIRE(str.GetLengthWithNullTerminator() == 1);
  REQUIRE(str.GetCTStr()[0] == COMET_TCHAR('\0'));
}

TEST_CASE("TString append empty keeps value", "[comet]") {
  comet::TString str{COMET_TCHAR("hello")};

  str.Append(COMET_TCHAR(""));

  REQUIRE(str == COMET_TCHAR("hello"));
  REQUIRE(str.GetLength() == 5);
}

TEST_CASE("TString append char", "[comet]") {
  comet::TString str{COMET_TCHAR("hell")};

  str.Append(1, COMET_TCHAR('o'));

  REQUIRE(str == COMET_TCHAR("hello"));
}

TEST_CASE("TString self assignment preserves value", "[comet]") {
  comet::TString str{COMET_TCHAR("hello")};

  str = str;

  REQUIRE(str == COMET_TCHAR("hello"));
}

TEST_CASE("TString copy assignment performs deep copy", "[comet]") {
  comet::TString str{COMET_TCHAR("hello")};
  comet::TString copy{};

  copy = str;
  copy[0] = COMET_TCHAR('H');

  REQUIRE(str == COMET_TCHAR("hello"));
  REQUIRE(copy == COMET_TCHAR("Hello"));
}

TEST_CASE("TString move assignment preserves value", "[comet]") {
  comet::TString str{COMET_TCHAR("hello")};
  comet::TString moved{};

  moved = std::move(str);

  REQUIRE(moved == COMET_TCHAR("hello"));
  REQUIRE(str.IsEmpty());
}

TEST_CASE("TString chained append grows correctly", "[comet]") {
  comet::TString str{COMET_TCHAR("a")};

  str.Append(COMET_TCHAR("b"));
  str.Append(COMET_TCHAR("c"));
  str.Append(COMET_TCHAR("def"));

  REQUIRE(str == COMET_TCHAR("abcdef"));
  REQUIRE(str.GetLength() == 6);
}

TEST_CASE("TString substring from start and full range", "[comet]") {
  comet::TString str{COMET_TCHAR("hello")};

  REQUIRE(str.GenerateSubString(0, 2) == COMET_TCHAR("he"));
  REQUIRE(str.GenerateSubString(0, str.GetLength()) == COMET_TCHAR("hello"));
}

TEST_CASE("TString resize grow null terminates", "[comet]") {
  comet::TString str{COMET_TCHAR("hi")};

  str.Resize(8);

  REQUIRE(str.GetLength() == 8);
  REQUIRE(str.GetCTStr()[8] == COMET_TCHAR('\0'));
}

TEST_CASE("TString mutable indexing changes value", "[comet]") {
  comet::TString str{COMET_TCHAR("hello")};

  str[1] = COMET_TCHAR('a');

  REQUIRE(str == COMET_TCHAR("hallo"));
}

TEST_CASE("TString plus with empty strings", "[comet]") {
  comet::TString empty{};
  comet::TString hello{COMET_TCHAR("hello")};

  REQUIRE((empty + hello) == COMET_TCHAR("hello"));
  REQUIRE((hello + empty) == COMET_TCHAR("hello"));
  REQUIRE((empty + empty).IsEmpty());
}

TEST_CASE("TString equality with literal and empty literal", "[comet]") {
  comet::TString str{COMET_TCHAR("hello")};
  comet::TString empty{};

  REQUIRE(str == COMET_TCHAR("hello"));
  REQUIRE(str != COMET_TCHAR("hell"));
  REQUIRE(empty == COMET_TCHAR(""));
}