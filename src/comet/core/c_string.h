// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_C_STRING_H_
#define COMET_COMET_CORE_C_STRING_H_

#include "comet_precompile.h"

#include "comet/math/math_commons.h"

namespace comet {
s32 Compare(const schar* str1, const schar* str2);
s32 Compare(const wchar* str1, const wchar* str2);
s32 Compare(const schar* str1, const schar* str2, uindex length);
s32 Compare(const wchar* str1, const wchar* str2, uindex length);
bool AreStringsEqual(const schar* str1, const schar* str2);
bool AreStringsEqual(const wchar* str1, const wchar* str2);
bool AreStringsEqual(const schar* str1, uindex str1_len, const schar* str2,
                     uindex str2_len);
bool AreStringsEqual(const wchar* str1, uindex str1_len, const wchar* str2,
                     uindex str2_len);
s32 CompareInsensitive(const schar* str1, const schar* str2);
s32 CompareInsensitive(const wchar* str1, const wchar* str2);
s32 CompareInsensitive(const schar* str1, const schar* str2, uindex length);
s32 CompareInsensitive(const wchar* str1, const wchar* str2, uindex length);
bool AreStringsEqualInsensitive(const schar* str1, const schar* str2);
bool AreStringsEqualInsensitive(const wchar* str1, const wchar* str2);
bool AreStringsEqualInsensitive(const schar* str1, uindex str1_len,
                                const schar* str2, uindex str2_len);
bool AreStringsEqualInsensitive(const wchar* str1, uindex str1_len,
                                const wchar* str2, uindex str2_len);
bool IsSpace(schar c);
bool IsSpace(wchar c);
bool IsAlpha(schar c);
bool IsAlpha(wchar c);
bool IsEmpty(const schar* str, uindex str_len);
bool IsEmpty(const wchar* str, uindex str_len);
schar* Copy(schar* dst, const schar* src, uindex length, uindex dst_offset = 0,
            uindex src_offset = 0);
wchar* Copy(wchar* dst, const schar* src, uindex length, uindex dst_offset = 0,
            uindex src_offset = 0);
schar* Copy(schar* dst, const wchar* src, uindex length, uindex dst_offset = 0,
            uindex src_offset = 0);
wchar* Copy(wchar* dst, const wchar* src, uindex length, uindex dst_offset = 0,
            uindex src_offset = 0);
schar* TrimLeft(schar* str, uindex length, uindex* new_length = nullptr);
wchar* TrimLeft(wchar* str, uindex length, uindex* new_length = nullptr);
schar* TrimRight(schar* str, uindex length, uindex* new_length = nullptr);
wchar* TrimRight(wchar* str, uindex length, uindex* new_length = nullptr);
schar* Trim(schar* str, uindex length, uindex* new_length = nullptr);
wchar* Trim(wchar* str, uindex length, uindex* new_length = nullptr);
void GetSubString(schar* dst, const schar* src, uindex src_length,
                  uindex offset, uindex length = kInvalidIndex);
void GetSubString(wchar* dst, const wchar* src, uindex src_length,
                  uindex offset, uindex length = kInvalidIndex);

constexpr uindex GetLength(const schar* str) noexcept {
  return std::char_traits<schar>::length(str);
}

constexpr uindex GetLengthWithNullTerminator(const schar* str) noexcept {
  return GetLength(str) + 1;
}

constexpr uindex GetLength(const wchar* str) noexcept {
  return std::char_traits<wchar>::length(str);
}

constexpr uindex GetLengthWithNullTerminator(const wchar* str) noexcept {
  return GetLength(str) + 1;
}

void FillWith(schar* str, uindex str_length, schar c, uindex offset = 0,
              uindex length = kInvalidIndex);
void FillWith(wchar* str, uindex str_length, wchar c, uindex offset = 0,
              uindex length = kInvalidIndex);
schar ToUpper(schar c);
wchar ToUpper(wchar c);
schar ToLower(schar c);
wchar ToLower(wchar c);
void Clear(schar* str, uindex str_len);
void Clear(wchar* str, uindex str_len);
uindex GetIndexOf(const schar* str, char c, uindex length,
                  uindex offset = kInvalidIndex);
uindex GetIndexOf(const wchar* str, char c, uindex length,
                  uindex offset = kInvalidIndex);
uindex GetLastIndexOf(const schar* str, uindex length, schar c,
                      uindex offset = kInvalidIndex);
uindex GetLastIndexOf(const wchar* str, uindex length, wchar c,
                      uindex offset = kInvalidIndex);
uindex GetNthToLastIndexOf(const schar* str, uindex str_length, schar to_search,
                           uindex count, uindex offset = kInvalidIndex);
uindex GetNthToLastIndexOf(const wchar* str, uindex str_length, wchar to_search,
                           uindex count, uindex offset = kInvalidIndex);
uindex GetFirstNonWhiteSpaceIndex(const schar* str, uindex str_len);
uindex GetFirstNonWhiteSpaceIndex(const wchar* str, uindex str_len);
uindex GetFirstDifferentCharacterIndex(const schar* str, uindex str_len,
                                       schar c);
uindex GetFirstDifferentCharacterIndex(const wchar* str, uindex str_len,
                                       wchar c);
uindex GetLastNonWhiteSpaceIndex(const schar* str, uindex str_len);
uindex GetLastNonWhiteSpaceIndex(const wchar* str, uindex str_len);
uindex GetLastNonCharacterIndex(const schar* str, uindex str_len, schar c);
uindex GetLastNonCharacterIndex(const wchar* str, uindex str_len, wchar c);
u8 ParseU8(const schar* str);
u8 ParseU8(const wchar* str);
u16 ParseU16(const schar* str);
u16 ParseU16(const wchar* str);
u32 ParseU32(const schar* str);
u32 ParseU32(const wchar* str);
u64 ParseU64(const schar* str);
u64 ParseU64(const wchar* str);
s8 ParseS8(const schar* str);
s8 ParseS8(const wchar* str);
s16 ParseS16(const schar* str);
s16 ParseS16(const wchar* str);
s32 ParseS32(const schar* str);
s32 ParseS32(const wchar* str);
s64 ParseS64(const schar* str);
s64 ParseS64(const wchar* str);
f32 ParseF32(const schar* str);
f32 ParseF32(const wchar* str);
f64 ParseF64(const schar* str);
f64 ParseF64(const wchar* str);
uindex ParseIndex(const schar* str);
uindex ParseIndex(const wchar* str);
ux ParseUx(const schar* str);
ux ParseUx(const wchar* str);
sx ParseSx(const schar* str);
sx ParseSx(const wchar* str);
fx ParseFx(const schar* str);
fx ParseFx(const wchar* str);
bool ParseBool(const schar* str);
bool ParseBool(const wchar* str);
void ConvertToStr(u8 number, schar* buffer, uindex buffer_len,
                  uindex* out_len = nullptr);
void ConvertToStr(u8 number, wchar* buffer, uindex buffer_len,
                  uindex* out_len = nullptr);
void ConvertToStr(u16 number, schar* buffer, uindex buffer_len,
                  uindex* out_len = nullptr);
void ConvertToStr(u16 number, wchar* buffer, uindex buffer_len,
                  uindex* out_len = nullptr);
void ConvertToStr(u32 number, schar* buffer, uindex buffer_len,
                  uindex* out_len = nullptr);
void ConvertToStr(u32 number, wchar* buffer, uindex buffer_len,
                  uindex* out_len = nullptr);
void ConvertToStr(u64 number, schar* buffer, uindex buffer_len,
                  uindex* out_len = nullptr);
void ConvertToStr(u64 number, wchar* buffer, uindex buffer_len,
                  uindex* out_len = nullptr);
void ConvertToStr(s8 number, schar* buffer, uindex buffer_len,
                  uindex* out_len = nullptr);
void ConvertToStr(s8 number, wchar* buffer, uindex buffer_len,
                  uindex* out_len = nullptr);
void ConvertToStr(s16 number, schar* buffer, uindex buffer_len,
                  uindex* out_len = nullptr);
void ConvertToStr(s16 number, wchar* buffer, uindex buffer_len,
                  uindex* out_len = nullptr);
void ConvertToStr(s32 number, schar* buffer, uindex buffer_len,
                  uindex* out_len = nullptr);
void ConvertToStr(s32 number, wchar* buffer, uindex buffer_len,
                  uindex* out_len = nullptr);
void ConvertToStr(s64 number, schar* buffer, uindex buffer_len,
                  uindex* out_len = nullptr);
void ConvertToStr(s64 number, wchar* buffer, uindex buffer_len,
                  uindex* out_len = nullptr);
void ConvertToStr(bool boolean, schar* buffer, uindex buffer_len,
                  uindex* out_len = nullptr);
void ConvertToStr(bool boolean, wchar* buffer, uindex buffer_len,
                  uindex* out_len = nullptr);

template <typename Float,
          typename std::enable_if_t<std::is_floating_point_v<Float>>* = nullptr>
void ConvertToStr(Float number, u8 precision, schar* buffer, uindex buffer_len,
                  uindex* out_len = nullptr) {
  // TODO(m4jr0): Optimize function.
  schar format[10];
  std::snprintf(format, 10, "%%.%df", precision);
  const auto len{std::snprintf(buffer, buffer_len - 1, format, number)};

  if (out_len != nullptr) {
    *out_len = len >= 0 ? len : kInvalidIndex;
  }
}

template <typename Float,
          typename std::enable_if_t<std::is_floating_point_v<Float>>* = nullptr>
void ConvertToStr(Float number, u8 precision, wchar* buffer, uindex buffer_len,
                  uindex* out_len = nullptr) {
  // TODO(m4jr0): Optimize function.
  schar format[10];
  std::snprintf(format, 10, "%%.%df", precision);
  const auto len{std::swprintf(buffer, buffer_len - 1, format, number)};

  if (out_len != nullptr) {
    *out_len = len >= 0 ? len : kInvalidIndex;
  }
}

namespace internal {
template <typename Char>
Char* TrimLeft(Char* str, uindex length, uindex* new_length = nullptr) {
  COMET_ASSERT(str != nullptr, "String provided is null!");

  if (length == 0) {
    return str;
  }

  uindex anchor{0};

  while (IsSpace(str[anchor]) && anchor < length) {
    ++anchor;
  }

  auto cursor{0};

  while (anchor < length) {
    auto c{str[anchor]};
    str[cursor] = c;
    ++cursor;
    ++anchor;
  }

  str[cursor] = '\0';

  if (new_length != nullptr) {
    *new_length = cursor - 1;
  }

  return str;
}

template <typename Char>
Char* TrimRight(Char* str, uindex length, uindex* new_length = nullptr) {
  COMET_ASSERT(str != nullptr, "String provided is null!");

  if (length == 0) {
    return str;
  }

  uindex i{length - 1};

  while (i != kInvalidIndex && (IsSpace(str[i]) || str[i] == '\0')) {
    --i;
  }

  if (i != kInvalidIndex && i < length - 1) {
    str[i + 1] = '\0';
  }

  if (new_length != nullptr) {
    *new_length = i;
  }

  return str;
}

template <typename Char>
Char* Trim(Char* str, uindex length, uindex* new_length = nullptr) {
  COMET_ASSERT(str != nullptr, "String provided is null!");

  if (length == 0) {
    return str;
  }

  uindex anchor{0};

  while (IsSpace(str[anchor]) && anchor < length) {
    ++anchor;
  }

  uindex cursor{0};
  auto last_index{cursor};

  while (anchor <= length) {
    auto c{str[anchor]};
    str[cursor] = c;

    if (c == '\0') {
      break;
    }

    if (!IsSpace(c)) {
      last_index = cursor;
    }

    ++cursor;
    ++anchor;
  }

  ++last_index;
  str[last_index] = '\0';

  if (new_length != nullptr) {
    *new_length = last_index;
  }

  return str;
}

template <typename Char>
void GetSubString(Char* dst, const Char* src, uindex src_length, uindex offset,
                  uindex length) {
  COMET_ASSERT(src != nullptr, "Source string provided is null!");
  COMET_ASSERT(dst != nullptr, "Destination string provided is null!");

  if (length == kInvalidIndex) {
    length = src_length - offset + 1;
  }

  if (length == 0) {
    return;
  }

  if (offset >= src_length) {
    dst[0] = COMET_TCHAR('\0');
    return;
  }

  const auto max_len{
      math::Min(src_length - offset + 1, static_cast<uindex>(length))};

  for (uindex i{0}; i < max_len; ++i) {
    dst[i] = src[i + offset];
  }

  dst[length] = COMET_TCHAR('\0');
}

template <typename Char>
void FillWith(Char* str, uindex str_length, Char c, uindex offset,
              uindex length) {
  COMET_ASSERT(str != nullptr, "String provided is null!");

  if (str_length == 0) {
    return;
  }

  COMET_ASSERT(offset < str_length, "Offset provided is too big!");
  COMET_ASSERT(length == kInvalidIndex || offset + length < str_length,
               "Length provided is too big!");

  if (length == kInvalidIndex) {
    length = str_length - offset;
  }

  auto max{offset + length};

  for (uindex i{offset}; i < max; ++i) {
    str[i] = c;
  }
}

template <typename Char>
uindex GetIndexOf(const Char* str, char c, uindex length, uindex offset) {
  if (length == 0 || (offset != kInvalidIndex && offset >= length)) {
    return kInvalidIndex;
  }

  if (offset == kInvalidIndex) {
    offset = 0;
  }

  for (uindex i{offset}; i < length; ++i) {
    if (str[i] == c) {
      return i;
    }
  }

  return kInvalidIndex;
}

template <typename Char>
uindex GetLastIndexOf(const Char* str, uindex length, Char c,
                      uindex offset = kInvalidIndex) {
  if (offset == kInvalidIndex) {
    offset = 0;
  }

  if (length == 0 || offset >= length) {
    return kInvalidIndex;
  }

  for (uindex i{length - 1 - offset}; i > -0; --i) {
    if (str[i] == c) {
      return i;
    }
  }

  return kInvalidIndex;
}

template <typename Char>
uindex GetNthToLastIndexOf(const Char* str, uindex str_length, Char to_search,
                           uindex count, uindex offset = kInvalidIndex) {
  if (offset == kInvalidIndex) {
    offset = 0;
  }

  if (str_length == 0 || count == 0 || offset >= str_length) {
    return kInvalidIndex;
  }

  uindex occurrence_counter{0};
  uindex nth_index{str_length - offset};

  while (nth_index != kInvalidIndex && occurrence_counter < count) {
    nth_index = GetLastIndexOf(str, nth_index, to_search, 1);
    ++occurrence_counter;
  }

  if (nth_index == kInvalidIndex || occurrence_counter != count) {
    return kInvalidIndex;
  }

  return nth_index;
}

template <typename Char>
uindex GetFirstNonWhiteSpaceIndex(const Char* str, uindex str_len) {
  if (str_len == 0) {
    return kInvalidIndex;
  }

  uindex index{0};

  while (index < str_len && IsSpace(str[index])) {
    ++index;
  }

  return index == str_len ? kInvalidIndex : index;
}

template <typename Char>
uindex GetFirstDifferentCharacterIndex(const Char* str, uindex str_len,
                                       Char c) {
  if (str_len == 0) {
    return kInvalidIndex;
  }

  uindex index{0};

  while (index < str_len && str[index] == c) {
    ++index;
  }

  return index == str_len ? kInvalidIndex : index;
}

template <typename Char>
uindex GetLastNonWhiteSpaceIndex(const Char* str, uindex str_len) {
  auto index{str_len};

  if (index == 0) {
    return 0;
  }

  --index;

  while (index > 0 && IsSpace(str[index])) {
    --index;
  }

  return index == kInvalidIndex ? kInvalidIndex : index;
}

template <typename Char>
uindex GetLastNonCharacterIndex(const Char* str, uindex str_len, Char c) {
  auto index{str_len};

  if (index == 0) {
    return 0;
  }

  --index;

  while (index > 0 && str[index] == c) {
    --index;
  }

  return index == kInvalidIndex ? kInvalidIndex : index;
}

template <typename Char>
bool ParseBool(const Char* str) {
  if (str == nullptr) {
    return false;
  }

  const auto length{GetLength(str)};

  if (length == 0) {
    return false;
  }

  return length != 1 || str[0] != '0';
}
}  // namespace internal
}  // namespace comet

#endif  // COMET_COMET_CORE_C_STRING_H_
