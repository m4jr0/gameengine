// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_core_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/core/command_line/command_line.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/string/c_string.h"

namespace comet {
namespace command_line {
namespace internal {
struct ParsedArgument {
  const schar* name{nullptr};
  usize name_length{0};
  const schar* inline_value{nullptr};
  bool has_inline_value{false};
  bool is_short{false};
};

bool DecodeArgument(const schar* arg, ParsedArgument& parsed) {
  if (arg == nullptr || arg[0] != '-' || arg[1] == '\0') {
    return false;
  }

  parsed.is_short = arg[1] != '-';
  parsed.name = arg + (parsed.is_short ? 1 : 2);

  const auto* cursor{parsed.name};

  while (*cursor != '\0' && *cursor != '=') {
    ++cursor;
  }

  parsed.name_length = static_cast<usize>(cursor - parsed.name);
  parsed.has_inline_value = *cursor == '=';
  parsed.inline_value = parsed.has_inline_value ? cursor + 1 : nullptr;

  return parsed.name_length > 0 &&
         (!parsed.is_short || parsed.name_length == 1);
}

Option* FindOption(const ParsedArgument& arg, Option* options,
                   usize option_count) {
  for (usize i{0}; i < option_count; ++i) {
    auto& option{options[i]};

    if (arg.is_short) {
      if (option.short_name == arg.name[0]) {
        return &option;
      }

      continue;
    }

    if (GetLength(option.name) == arg.name_length &&
        Compare(option.name, arg.name, arg.name_length) == 0) {
      return &option;
    }
  }

  return nullptr;
}

ParseResult ReadValue(const ParsedArgument& arg, int argc,
                      const schar* const* argv, int& index, Option& option) {
  if (!option.takes_value) {
    return arg.has_inline_value ? ParseResult::UnexpectedValue
                                : ParseResult::Success;
  }

  if (arg.has_inline_value) {
    option.value = arg.inline_value;
  } else if (index + 1 < argc && argv[index + 1] != nullptr &&
             argv[index + 1][0] != '-') {
    option.value = argv[++index];
  } else {
    return ParseResult::MissingValue;
  }

  return option.value[0] == '\0' ? ParseResult::MissingValue
                                  : ParseResult::Success;
}
}  // namespace internal

ParseResult Parse(int argc, const schar* const* argv, Option* options,
                  usize option_count, const schar** error_argument) {
  if (error_argument != nullptr) {
    *error_argument = nullptr;
  }

  if (argc < 0 || (argc > 0 && argv == nullptr) ||
      (option_count > 0 && options == nullptr)) {
    return ParseResult::InvalidArgument;
  }

  for (usize i{0}; i < option_count; ++i) {
    auto& option{options[i]};

    if (option.name == nullptr || option.name[0] == '\0' ||
        (option.short_name != '\0' && option.short_name == '-')) {
      return ParseResult::InvalidArgument;
    }

    option.is_present = false;
    option.value = nullptr;
  }

  for (int i{1}; i < argc; ++i) {
    const auto* arg{argv[i]};

    if (error_argument != nullptr) {
      *error_argument = arg;
    }

    internal::ParsedArgument parsed{};

    if (!internal::DecodeArgument(arg, parsed)) {
      return ParseResult::InvalidArgument;
    }

    auto* option{internal::FindOption(parsed, options, option_count)};

    if (option == nullptr) {
      return ParseResult::UnknownOption;
    }

    if (option->is_present) {
      return ParseResult::DuplicateOption;
    }

    const auto result{internal::ReadValue(parsed, argc, argv, i, *option)};

    if (result != ParseResult::Success) {
      return result;
    }

    option->is_present = true;
  }

  if (error_argument != nullptr) {
    *error_argument = nullptr;
  }

  return ParseResult::Success;
}
}  // namespace command_line
}  // namespace comet
