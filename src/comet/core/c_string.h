// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_C_STRING_H_
#define COMET_COMET_CORE_C_STRING_H_

#include "comet/core/essentials.h"
#include "comet/math/math_commons.h"

namespace comet {
constexpr auto kU8MaxCharCountDigits10{3};
constexpr auto kU16MaxCharCountDigits10{5};
constexpr auto kU32MaxCharCountDigits10{10};
constexpr auto kU64MaxCharCountDigits10{20};
constexpr auto kS8MaxCharCountDigits10{4};
constexpr auto kS16MaxCharCountDigits10{6};
constexpr auto kS32MaxCharCountDigits10{11};
constexpr auto kS64MaxCharCountDigits10{20};
constexpr auto kUIndexMaxCharCountDigits10{
#ifdef COMET_32
    kU32MaxCharCountDigits10
#else
    kU64MaxCharCountDigits10
#endif  // COMET_32
};
constexpr auto kUPtrMaxCharCountDigits10{
#ifdef COMET_32
    kU32MaxCharCountDigits10
#else
    kU64MaxCharCountDigits10
#endif  // COMET_32
};
constexpr auto kSptrDiffMaxCharCountDigits10{
#ifdef COMET_32
    kU32MaxCharCountDigits10
#else
    kU64MaxCharCountDigits10
#endif  // COMET_32
};
constexpr auto kSCharMaxCharCountDigits10{kS8MaxCharCountDigits10};
constexpr auto kUCharMaxCharCountDigits10{kU8MaxCharCountDigits10};
constexpr auto kWCharMaxCharCountDigits10{kS8MaxCharCountDigits10};
constexpr auto kF32MaxCharCountDigits10{16};
constexpr auto kF64MaxCharCountDigits10{24};
constexpr auto kBoolMaxCharCountDigits10{5};  // true or false.
constexpr auto kB8MaxCharCountDigits10{kBoolMaxCharCountDigits10};
constexpr auto kB32MaxCharCountDigits10{kBoolMaxCharCountDigits10};

s32 Compare(const schar* str1, const schar* str2);
s32 Compare(const wchar* str1, const wchar* str2);
s32 Compare(const schar* str1, const schar* str2, usize length);
s32 Compare(const wchar* str1, const wchar* str2, usize length);
bool AreStringsEqual(const schar* str1, const schar* str2);
bool AreStringsEqual(const wchar* str1, const wchar* str2);
bool AreStringsEqual(const schar* str1, usize str1_len, const schar* str2,
                     usize str2_len);
bool AreStringsEqual(const wchar* str1, usize str1_len, const wchar* str2,
                     usize str2_len);
s32 CompareInsensitive(const schar* str1, const schar* str2);
s32 CompareInsensitive(const wchar* str1, const wchar* str2);
s32 CompareInsensitive(const schar* str1, const schar* str2, usize length);
s32 CompareInsensitive(const wchar* str1, const wchar* str2, usize length);
bool AreStringsEqualInsensitive(const schar* str1, const schar* str2);
bool AreStringsEqualInsensitive(const wchar* str1, const wchar* str2);
bool AreStringsEqualInsensitive(const schar* str1, usize str1_len,
                                const schar* str2, usize str2_len);
bool AreStringsEqualInsensitive(const wchar* str1, usize str1_len,
                                const wchar* str2, usize str2_len);
bool IsSpace(schar c);
bool IsSpace(wchar c);
bool IsAlpha(schar c);
bool IsAlpha(wchar c);
bool IsEmpty(const schar* str, usize str_len);
bool IsEmpty(const wchar* str, usize str_len);
schar* Copy(schar* dst, const schar* src, usize length, usize dst_offset = 0,
            usize src_offset = 0);
wchar* Copy(wchar* dst, const schar* src, usize length, usize dst_offset = 0,
            usize src_offset = 0);
schar* Copy(schar* dst, const wchar* src, usize length, usize dst_offset = 0,
            usize src_offset = 0);
wchar* Copy(wchar* dst, const wchar* src, usize length, usize dst_offset = 0,
            usize src_offset = 0);
schar* TrimLeft(schar* str, usize length, usize* new_length = nullptr);
wchar* TrimLeft(wchar* str, usize length, usize* new_length = nullptr);
schar* TrimRight(schar* str, usize length, usize* new_length = nullptr);
wchar* TrimRight(wchar* str, usize length, usize* new_length = nullptr);
schar* Trim(schar* str, usize length, usize* new_length = nullptr);
wchar* Trim(wchar* str, usize length, usize* new_length = nullptr);
void GetSubString(schar* dst, const schar* src, usize src_length, usize offset,
                  usize length = kInvalidIndex);
void GetSubString(wchar* dst, const wchar* src, usize src_length, usize offset,
                  usize length = kInvalidIndex);

constexpr usize GetLength(const schar* str) noexcept {
  return std::char_traits<schar>::length(str);
}

constexpr usize GetLengthWithNullTerminator(const schar* str) noexcept {
  return GetLength(str) + 1;
}

constexpr usize GetLength(const wchar* str) noexcept {
  return std::char_traits<wchar>::length(str);
}

constexpr usize GetLengthWithNullTerminator(const wchar* str) noexcept {
  return GetLength(str) + 1;
}

void FillWith(schar* str, usize str_length, schar c, usize offset = 0,
              usize length = kInvalidIndex);
void FillWith(wchar* str, usize str_length, wchar c, usize offset = 0,
              usize length = kInvalidIndex);
schar ToUpper(schar c);
wchar ToUpper(wchar c);
schar ToLower(schar c);
wchar ToLower(wchar c);
void Clear(schar* str, usize str_len);
void Clear(wchar* str, usize str_len);
usize GetIndexOf(const schar* str, char c, usize length,
                 usize offset = kInvalidIndex);
usize GetIndexOf(const wchar* str, char c, usize length,
                 usize offset = kInvalidIndex);
usize GetLastIndexOf(const schar* str, usize length, schar c,
                     usize offset = kInvalidIndex);
usize GetLastIndexOf(const wchar* str, usize length, wchar c,
                     usize offset = kInvalidIndex);
usize GetNthToLastIndexOf(const schar* str, usize str_length, schar to_search,
                          usize count, usize offset = kInvalidIndex);
usize GetNthToLastIndexOf(const wchar* str, usize str_length, wchar to_search,
                          usize count, usize offset = kInvalidIndex);
usize GetFirstNonWhiteSpaceIndex(const schar* str, usize str_len);
usize GetFirstNonWhiteSpaceIndex(const wchar* str, usize str_len);
usize GetFirstDifferentCharacterIndex(const schar* str, usize str_len, schar c);
usize GetFirstDifferentCharacterIndex(const wchar* str, usize str_len, wchar c);
usize GetLastNonWhiteSpaceIndex(const schar* str, usize str_len);
usize GetLastNonWhiteSpaceIndex(const wchar* str, usize str_len);
usize GetLastNonCharacterIndex(const schar* str, usize str_len, schar c);
usize GetLastNonCharacterIndex(const wchar* str, usize str_len, wchar c);
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
usize ParseIndex(const schar* str);
usize ParseIndex(const wchar* str);
ux ParseUx(const schar* str);
ux ParseUx(const wchar* str);
sx ParseSx(const schar* str);
sx ParseSx(const wchar* str);
fx ParseFx(const schar* str);
fx ParseFx(const wchar* str);
bool ParseBool(const schar* str);
bool ParseBool(const wchar* str);
void ConvertToStr(u8 number, schar* buffer, usize buffer_len,
                  usize* out_len = nullptr);
void ConvertToStr(u8 number, wchar* buffer, usize buffer_len,
                  usize* out_len = nullptr);
void ConvertToStr(u16 number, schar* buffer, usize buffer_len,
                  usize* out_len = nullptr);
void ConvertToStr(u16 number, wchar* buffer, usize buffer_len,
                  usize* out_len = nullptr);
void ConvertToStr(u32 number, schar* buffer, usize buffer_len,
                  usize* out_len = nullptr);
void ConvertToStr(u32 number, wchar* buffer, usize buffer_len,
                  usize* out_len = nullptr);
void ConvertToStr(u64 number, schar* buffer, usize buffer_len,
                  usize* out_len = nullptr);
void ConvertToStr(u64 number, wchar* buffer, usize buffer_len,
                  usize* out_len = nullptr);
void ConvertToStr(s8 number, schar* buffer, usize buffer_len,
                  usize* out_len = nullptr);
void ConvertToStr(s8 number, wchar* buffer, usize buffer_len,
                  usize* out_len = nullptr);
void ConvertToStr(s16 number, schar* buffer, usize buffer_len,
                  usize* out_len = nullptr);
void ConvertToStr(s16 number, wchar* buffer, usize buffer_len,
                  usize* out_len = nullptr);
void ConvertToStr(s32 number, schar* buffer, usize buffer_len,
                  usize* out_len = nullptr);
void ConvertToStr(s32 number, wchar* buffer, usize buffer_len,
                  usize* out_len = nullptr);
void ConvertToStr(s64 number, schar* buffer, usize buffer_len,
                  usize* out_len = nullptr);
void ConvertToStr(s64 number, wchar* buffer, usize buffer_len,
                  usize* out_len = nullptr);
void ConvertToStr(bool boolean, schar* buffer, usize buffer_len,
                  usize* out_len = nullptr);
void ConvertToStr(bool boolean, wchar* buffer, usize buffer_len,
                  usize* out_len = nullptr);
usize GetCharCount(u8);
usize GetCharCount(u16);
usize GetCharCount(u32);
usize GetCharCount(u64);
usize GetCharCount(s8);
usize GetCharCount(s16);
usize GetCharCount(s32);
usize GetCharCount(s64);
usize GetCharCount(f32);
usize GetCharCount(f64);
usize GetCharCount(bool);

template <typename Float,
          typename std::enable_if_t<std::is_floating_point_v<Float>>* = nullptr>
void ConvertToStr(Float number, u8 precision, schar* buffer, usize buffer_len,
                  usize* out_len = nullptr) {
  // TODO(m4jr0): Optimize function.
  schar format[10];
  std::snprintf(format, 10, "%%.%df", precision);
  std::snprintf(buffer, buffer_len - 1, format, number);

  if (out_len != nullptr) {
    *out_len = GetLength(buffer);
  }
}

template <typename Float,
          typename std::enable_if_t<std::is_floating_point_v<Float>>* = nullptr>
void ConvertToStr(Float number, schar* buffer, usize buffer_len,
                  usize* out_len = nullptr) {
  ConvertToStr(number, 3, buffer, buffer_len, out_len);
}

template <typename Float,
          typename std::enable_if_t<std::is_floating_point_v<Float>>* = nullptr>
void ConvertToStr(Float number, u8 precision, wchar* buffer, usize buffer_len,
                  usize* out_len = nullptr) {
  // TODO(m4jr0): Optimize function.
  schar format[10];
  std::snprintf(format, 10, "%%.%df", precision);
  const auto len{std::swprintf(buffer, buffer_len - 1, format, number)};

  if (out_len != nullptr) {
    *out_len = GetLength(buffer);
  }
}

namespace internal {
template <typename Char>
Char* TrimLeft(Char* str, usize length, usize* new_length = nullptr) {
  COMET_ASSERT(str != nullptr, "String provided is null!");

  if (length == 0) {
    return str;
  }

  usize anchor{0};

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
Char* TrimRight(Char* str, usize length, usize* new_length = nullptr) {
  COMET_ASSERT(str != nullptr, "String provided is null!");

  if (length == 0) {
    return str;
  }

  usize i{length - 1};

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
Char* Trim(Char* str, usize length, usize* new_length = nullptr) {
  COMET_ASSERT(str != nullptr, "String provided is null!");

  if (length == 0) {
    return str;
  }

  usize anchor{0};

  while (IsSpace(str[anchor]) && anchor < length) {
    ++anchor;
  }

  usize cursor{0};
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
void GetSubString(Char* dst, const Char* src, usize src_length, usize offset,
                  usize length) {
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
      math::Min(src_length - offset + 1, static_cast<usize>(length))};

  for (usize i{0}; i < max_len; ++i) {
    dst[i] = src[i + offset];
  }

  dst[length] = COMET_TCHAR('\0');
}

template <typename Char>
void FillWith(Char* str, usize str_length, Char c, usize offset, usize length) {
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

  for (usize i{offset}; i < max; ++i) {
    str[i] = c;
  }
}

template <typename Char>
usize GetIndexOf(const Char* str, char c, usize length, usize offset) {
  if (length == 0 || (offset != kInvalidIndex && offset >= length)) {
    return kInvalidIndex;
  }

  if (offset == kInvalidIndex) {
    offset = 0;
  }

  for (usize i{offset}; i < length; ++i) {
    if (str[i] == c) {
      return i;
    }
  }

  return kInvalidIndex;
}

template <typename Char>
usize GetLastIndexOf(const Char* str, usize length, Char c,
                     usize offset = kInvalidIndex) {
  if (offset == kInvalidIndex) {
    offset = 0;
  }

  if (length == 0 || offset >= length) {
    return kInvalidIndex;
  }

  for (usize i{length - 1 - offset}; i > -0; --i) {
    if (str[i] == c) {
      return i;
    }
  }

  return kInvalidIndex;
}

template <typename Char>
usize GetNthToLastIndexOf(const Char* str, usize str_length, Char to_search,
                          usize count, usize offset = kInvalidIndex) {
  if (offset == kInvalidIndex) {
    offset = 0;
  }

  if (str_length == 0 || count == 0 || offset >= str_length) {
    return kInvalidIndex;
  }

  usize occurrence_counter{0};
  usize nth_index{str_length - offset};

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
usize GetFirstNonWhiteSpaceIndex(const Char* str, usize str_len) {
  if (str_len == 0) {
    return kInvalidIndex;
  }

  usize index{0};

  while (index < str_len && IsSpace(str[index])) {
    ++index;
  }

  return index == str_len ? kInvalidIndex : index;
}

template <typename Char>
usize GetFirstDifferentCharacterIndex(const Char* str, usize str_len, Char c) {
  if (str_len == 0) {
    return kInvalidIndex;
  }

  usize index{0};

  while (index < str_len && str[index] == c) {
    ++index;
  }

  return index == str_len ? kInvalidIndex : index;
}

template <typename Char>
usize GetLastNonWhiteSpaceIndex(const Char* str, usize str_len) {
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
usize GetLastNonCharacterIndex(const Char* str, usize str_len, Char c) {
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
