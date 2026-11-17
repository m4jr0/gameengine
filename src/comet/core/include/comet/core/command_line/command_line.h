// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_CORE_COMMAND_LINE_COMMAND_LINE_H_
#define COMET_CORE_COMMAND_LINE_COMMAND_LINE_H_

#include "comet/core/essentials.h"

namespace comet {
namespace command_line {
// Option names omit the leading dashes. A short name is a single character.
// Parsed values point into argv and remain owned by the caller.
struct Option {
  const schar* name{nullptr};
  bool takes_value{false};
  bool is_present{false};
  const schar* value{nullptr};
  schar short_name{'\0'};
};

enum class ParseResult : u8 {
  Success,
  UnknownOption,
  MissingValue,
  UnexpectedValue,
  DuplicateOption,
  InvalidArgument,
};

// Parses long (--name) and short (-n) options without allocating memory.
// String options accept --name=value, --name value, -n=value and -n value.
// Combined short flags and positional arguments are not supported.
// On failure, error_argument points to the offending argv entry.
ParseResult Parse(int argc, const schar* const* argv, Option* options,
                  usize option_count,
                  const schar** error_argument = nullptr);

}  // namespace command_line
}  // namespace comet

#endif  // COMET_CORE_COMMAND_LINE_COMMAND_LINE_H_
