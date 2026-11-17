// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include <string_view>

#include "comet/core/command_line/command_line.h"
#include "catch.hpp"

TEST_CASE("Command line parses flags and values", "[comet][command_line]") {
  const char* argv[]{"tool", "--force", "--assets", "some assets",
                     "--resources=compiled"};
  comet::command_line::Option options[]{
      {"force", false}, {"assets", true}, {"resources", true}};
  const char* error{nullptr};
  REQUIRE(comet::command_line::Parse(5, argv, options, 3, &error) ==
          comet::command_line::ParseResult::Success);
  REQUIRE(error == nullptr);
  REQUIRE(options[0].is_present);
  REQUIRE(options[1].is_present);
  REQUIRE(std::string_view{options[1].value} == "some assets");
  REQUIRE(std::string_view{options[2].value} == "compiled");
  REQUIRE(comet::command_line::Parse(1, argv, options, 3) ==
          comet::command_line::ParseResult::Success);
  REQUIRE_FALSE(options[0].is_present);
  REQUIRE_FALSE(options[1].is_present);
  REQUIRE_FALSE(options[2].is_present);
}

TEST_CASE("Command line rejects malformed options", "[comet][command_line]") {
  using comet::command_line::Option;
  using comet::command_line::Parse;
  using comet::command_line::ParseResult;
  Option options[]{{"force", false}, {"assets", true}};
  const char* unknown[]{"tool", "--unknown"};
  REQUIRE(Parse(2, unknown, options, 2) == ParseResult::UnknownOption);
  const char* missing[]{"tool", "--assets"};
  REQUIRE(Parse(2, missing, options, 2) == ParseResult::MissingValue);
  const char* unexpected[]{"tool", "--force=yes"};
  REQUIRE(Parse(2, unexpected, options, 2) == ParseResult::UnexpectedValue);
  const char* duplicate[]{"tool", "--force", "--force"};
  REQUIRE(Parse(3, duplicate, options, 2) == ParseResult::DuplicateOption);
  const char* empty[]{"tool", "--assets="};
  REQUIRE(Parse(2, empty, options, 2) == ParseResult::MissingValue);
}

TEST_CASE("Command line parses short options", "[comet][command_line]") {
  using comet::command_line::Option;
  using comet::command_line::Parse;
  using comet::command_line::ParseResult;

  Option options[]{
      {"help", false, false, nullptr, 'h'},
      {"force", false, false, nullptr, 'f'},
      {"assets", true, false, nullptr, 'a'},
      {"resources", true, false, nullptr, 'r'},
  };

  const char* argv[]{"tool", "-h", "-f", "-a", "asset folder",
                     "-r=resource folder"};
  REQUIRE(Parse(6, argv, options, 4) == ParseResult::Success);
  REQUIRE(options[0].is_present);
  REQUIRE(options[1].is_present);
  REQUIRE(std::string_view{options[2].value} == "asset folder");
  REQUIRE(std::string_view{options[3].value} == "resource folder");

  const char* repeated[]{"tool", "-f", "--force"};
  REQUIRE(Parse(3, repeated, options, 4) == ParseResult::DuplicateOption);

  const char* combined[]{"tool", "-hf"};
  REQUIRE(Parse(2, combined, options, 4) == ParseResult::InvalidArgument);

  const char* missing[]{"tool", "-a"};
  REQUIRE(Parse(2, missing, options, 4) == ParseResult::MissingValue);

  const char* unexpected[]{"tool", "-f=yes"};
  REQUIRE(Parse(2, unexpected, options, 4) == ParseResult::UnexpectedValue);
}
