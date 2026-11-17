// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_CORE_COMMAND_LINE_COMMAND_LINE_H_
#define COMET_CORE_COMMAND_LINE_COMMAND_LINE_H_

#include "comet/core/essentials.h"

namespace comet {
namespace command_line {
// Option names omit the leading "--". Values point into argv and are not owned.
struct Option {
  const schar* name{nullptr};
  bool takes_value{false};
  bool is_present{false};
  const schar* value{nullptr};
};

enum class ParseResult : u8 {
  Success,
  UnknownOption,
  MissingValue,
  UnexpectedValue,
  DuplicateOption,
  InvalidArgument,
};

// No allocations. The caller owns options and argv for the duration of use.
// On failure, error_argument points to the offending argv entry.
inline ParseResult Parse(int argc, const schar* const* argv, Option* options,
                         usize option_count,
                         const schar** error_argument = nullptr) {
  if (error_argument != nullptr) {
    *error_argument = nullptr;
  }
  if (argc < 0 || (argc > 0 && argv == nullptr) ||
      (option_count > 0 && options == nullptr)) {
    return ParseResult::InvalidArgument;
  }
  for (usize i{0}; i < option_count; ++i) {
    if (options[i].name == nullptr) {
      return ParseResult::InvalidArgument;
    }
    options[i].is_present = false;
    options[i].value = nullptr;
  }
  for (int i{1}; i < argc; ++i) {
    const schar* arg{argv[i]};
    if (arg == nullptr || arg[0] != '-' || arg[1] != '-') {
      if (error_argument != nullptr) *error_argument = arg;
      return ParseResult::InvalidArgument;
    }
    const schar* name{arg + 2};
    const schar* separator{name};
    while (*separator != '\0' && *separator != '=') ++separator;
    const usize name_len{static_cast<usize>(separator - name)};
    Option* match{nullptr};
    for (usize j{0}; j < option_count; ++j) {
      const schar* candidate{options[j].name};
      usize length{0};
      while (candidate[length] != '\0') ++length;
      if (length != name_len) continue;
      bool equal{true};
      for (usize k{0}; k < name_len; ++k) {
        if (candidate[k] != name[k]) { equal = false; break; }
      }
      if (equal) { match = &options[j]; break; }
    }
    if (error_argument != nullptr) *error_argument = arg;
    if (match == nullptr) return ParseResult::UnknownOption;
    if (match->is_present) return ParseResult::DuplicateOption;
    const bool has_inline_value{*separator == '='};
    if (!match->takes_value && has_inline_value)
      return ParseResult::UnexpectedValue;
    if (match->takes_value) {
      if (has_inline_value) {
        match->value = separator + 1;
      } else if (i + 1 < argc && argv[i + 1] != nullptr &&
                 !(argv[i + 1][0] == '-' && argv[i + 1][1] == '-')) {
        match->value = argv[++i];
      } else {
        return ParseResult::MissingValue;
      }
      if (match->value[0] == '\0') return ParseResult::MissingValue;
    }
    match->is_present = true;
  }
  if (error_argument != nullptr) *error_argument = nullptr;
  return ParseResult::Success;
}
}  // namespace command_line
}  // namespace comet

#endif  // COMET_CORE_COMMAND_LINE_COMMAND_LINE_H_
