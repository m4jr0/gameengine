// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "string.h"

namespace comet {
namespace utils {
namespace string {
uindex GetLastNthPos(std::string_view str, std::string_view to_search,
                     uindex count) {
  if (str.size() == 0 || count == 0) {
    return kInvalidIndex;
  }

  uindex occurrence_counter{0};
  uindex nth_pos{str.size()};

  while (nth_pos != std::string::npos && occurrence_counter < count) {
    nth_pos = str.find_last_of(to_search, nth_pos - 1);
    ++occurrence_counter;
  }

  if (nth_pos == std::string::npos || occurrence_counter != count) {
    return kInvalidIndex;
  }

  return nth_pos;
}

uindex GetLastNthPos(std::string_view str, schar to_search, uindex count) {
  return GetLastNthPos(str, std::string_view{&to_search, 1}, count);
}

std::string& TrimLeft(std::string& str) {
  str.erase(str.begin(), std::find_if(str.begin(), str.end(), [](schar c) {
              return !std::isspace(c);
            }));

  return str;
}

std::string& TrimRight(std::string& str) {
  str.erase(std::find_if(str.rbegin(), str.rend(),
                         [](schar c) { return !std::isspace(c); })
                .base(),
            str.end());

  return str;
}

std::string& Trim(std::string& str) {
  TrimLeft(str);
  TrimRight(str);
  return str;
}

std::string GetLeftTrimmedCopy(std::string str) {
  TrimLeft(str);
  return str;
}

std::string GetRightTrimmedCopy(std::string str) {
  TrimRight(str);
  return str;
}

std::string GetTrimmedCopy(std::string str) {
  Trim(str);
  return str;
}

uindex GetFirstNonWhiteSpaceIndex(std::string_view str) {
  auto str_len{str.size()};

  if (str_len == 0) {
    return kInvalidIndex;
  }

  uindex index{0};

  while (index < str_len && std::isspace(str[index])) {
    ++index;
  }

  return index == str_len ? kInvalidIndex : index;
}

uindex GetFirstDifferentCharacterIndex(std::string_view str, schar c) {
  auto str_len{str.size()};

  if (str_len == 0) {
    return kInvalidIndex;
  }

  uindex index{0};

  while (index < str_len && str[index] == c) {
    ++index;
  }

  return index == str_len ? kInvalidIndex : index;
}

uindex GetLastNonWhiteSpaceIndex(std::string_view str) {
  auto index{str.size()};

  if (index == 0) {
    return 0;
  }

  --index;

  while (index >= 0 && std::isspace(str[index])) {
    --index;
  }

  return index == kInvalidIndex ? kInvalidIndex : index;
}

uindex GetLastNonCharacterIndex(std::string_view str, schar c) {
  auto index{str.size()};

  if (index == 0) {
    return 0;
  }

  --index;

  while (index >= 0 && str[index] == c) {
    --index;
  }

  return index == kInvalidIndex ? kInvalidIndex : index;
}

u8 ParseU8(const std::string& str) { return ParseU8(str.c_str()); }

u8 ParseU8(const schar* str) { return static_cast<u8>(std::stoul(str)); }

u16 ParseU16(const std::string& str) { return ParseU16(str.c_str()); }

u16 ParseU16(const schar* str) { return static_cast<u16>(std::stoul(str)); }

u32 ParseU32(const std::string& str) { return ParseU32(str.c_str()); }

u32 ParseU32(const schar* str) { return static_cast<u32>(std::stoul(str)); }

u64 ParseU64(const std::string& str) { return ParseU64(str.c_str()); }

u64 ParseU64(const schar* str) { return static_cast<u64>(std::stoull(str)); }

s8 ParseS8(const std::string& str) { return ParseS8(str.c_str()); }

s8 ParseS8(const schar* str) { return static_cast<s8>(std::stoi(str)); }

s16 ParseS16(const std::string& str) { return ParseS16(str.c_str()); }

s16 ParseS16(const schar* str) { return static_cast<s16>(std::stoi(str)); }

s32 ParseS32(const std::string& str) { return ParseS32(str.c_str()); }

s32 ParseS32(const schar* str) { return static_cast<s32>(std::stoi(str)); }

s64 ParseS64(const std::string& str) { return ParseS64(str.c_str()); }

s64 ParseS64(const schar* str) { return static_cast<s64>(std::stol(str)); }

f32 ParseF32(const std::string& str) { return ParseF32(str.c_str()); }

f32 ParseF32(const schar* str) { return static_cast<f32>(std::stod(str)); }

f64 ParseF64(const std::string& str) { return ParseF64(str.c_str()); }

f64 ParseF64(const schar* str) { return static_cast<f32>(std::stold(str)); }

uindex ParseIndex(const std::string& str) { return ParseIndex(str.c_str()); }

uindex ParseIndex(const schar* str) {
  return static_cast<uindex>(std::stoull(str));
}

ux ParseUx(const std::string& str) { return ParseUx(str.c_str()); }

ux ParseUx(const schar* str) {
#ifdef COMET_64
  return static_cast<u64>(std::stoull(str));
#else
  return static_cast<u32>(std::stoul(str));
#endif  // COMET_64
}

sx ParseSx(const std::string& str) { return ParseSx(str.c_str()); }

sx ParseSx(const schar* str) {
#ifdef COMET_64
  return static_cast<s64>(std::stol(str));
#else
  return static_cast<s32>(std::stoi(str));
#endif  // COMET_64
}

fx ParseFx(const std::string& str) { return ParseFx(str.c_str()); }

fx ParseFx(const schar* str) {
#ifdef COMET_64
  return static_cast<f32>(std::stold(str));
#else
  return static_cast<f32>(std::stod(str));
#endif  // COMET_64
}

bool ParseBool(const std::string& str) { return ParseBool(str.c_str()); }

bool ParseBool(const schar* str) {
  if (str == nullptr) {
    return false;
  }

  const auto length{strlen(str)};

  if (length == 0) {
    return false;
  }

  schar c{str[0]};
  uindex i{1};

  while (i < length && std::isspace(c)) {
    c = str[i];
    ++i;
  }

  if (c != '1') {
    return false;
  }

  while (i < length) {
    if (!std::isspace(c)) {
      return false;
    }

    ++i;
  }

  return true;
}
}  // namespace string
}  // namespace utils
}  // namespace comet
