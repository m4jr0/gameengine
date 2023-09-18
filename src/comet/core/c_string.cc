// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "c_string.h"

#include <inttypes.h>

#include <cwctype>

namespace comet {
s32 Compare(const schar* str1, const schar* str2) {
  return std::strcmp(str1, str2);
}

s32 Compare(const wchar* str1, const wchar* str2) {
  return std::wcscmp(str1, str2);
}

s32 Compare(const schar* str1, const schar* str2, uindex length) {
  return std::strncmp(str1, str2, length);
}

s32 Compare(const wchar* str1, const wchar* str2, uindex length) {
  return std::wcsncmp(str1, str2, length);
}

bool AreStringsEqual(const schar* str1, const schar* str2) {
  if (str1 == nullptr && str2 == nullptr) {
    return true;
  }

  if (str1 == nullptr || str2 == nullptr) {
    return false;
  }

  return Compare(str1, str2) == 0;
}

bool AreStringsEqual(const wchar* str1, const wchar* str2) {
  if (str1 == nullptr && str2 == nullptr) {
    return true;
  }

  if (str1 == nullptr || str2 == nullptr) {
    return false;
  }

  return Compare(str1, str2) == 0;
}

bool AreStringsEqual(const schar* str1, uindex str1_len, const schar* str2,
                     uindex str2_len) {
  if (str1_len != str2_len) {
    return false;
  }

  if (str1 == nullptr && str2 == nullptr) {
    return true;
  }

  if (str1 == nullptr || str2 == nullptr) {
    return false;
  }

  return Compare(str1, str2, str1_len) == 0;
}

bool AreStringsEqual(const wchar* str1, uindex str1_len, const wchar* str2,
                     uindex str2_len) {
  if (str1_len != str2_len) {
    return false;
  }

  if (str1 == nullptr && str2 == nullptr) {
    return true;
  }

  if (str1 == nullptr || str2 == nullptr) {
    return false;
  }

  return Compare(str1, str2, str1_len) == 0;
}

s32 CompareInsensitive(const schar* str1, const schar* str2) {
#ifdef COMET_GCC
  return strcasecmp(str1, str2);
#elif defined(COMET_MSVC)
  return _stricmp(str1, str2);
#endif  // COMET_GCC
}

s32 CompareInsensitive(const wchar* str1, const wchar* str2) {
#ifdef COMET_GCC
  return wcscasecmp(str1, str2);
#elif defined(COMET_MSVC)
  return _wcsicmp(str1, str2);
#endif  // COMET_GCC
}

s32 CompareInsensitive(const schar* str1, const schar* str2, uindex length) {
#ifdef COMET_GCC
  return strncasecmp(str1, str2, length);
#elif defined(COMET_MSVC)
  return _strnicmp(str1, str2, length);
#endif  // COMET_GCC
}

s32 CompareInsensitive(const wchar* str1, const wchar* str2, uindex length) {
#ifdef COMET_GCC
  return wcsncasecmp(str1, str2, length);
#elif defined(COMET_MSVC)
  return _wcsnicmp(str1, str2, length);
#endif  // COMET_GCC
}

bool AreStringsEqualInsensitive(const schar* str1, const schar* str2) {
  if (str1 == nullptr && str2 == nullptr) {
    return true;
  }

  if (str1 == nullptr || str2 == nullptr) {
    return false;
  }

  return CompareInsensitive(str1, str2) == 0;
}

bool AreStringsEqualInsensitive(const wchar* str1, const wchar* str2) {
  if (str1 == nullptr && str2 == nullptr) {
    return true;
  }

  if (str1 == nullptr || str2 == nullptr) {
    return false;
  }

  return CompareInsensitive(str1, str2) == 0;
}

bool AreStringsEqualInsensitive(const schar* str1, uindex str1_len,
                                const schar* str2, uindex str2_len) {
  if (str1_len != str2_len) {
    return false;
  }

  if (str1 == nullptr && str2 == nullptr) {
    return true;
  }

  if (str1 == nullptr || str2 == nullptr) {
    return false;
  }

  return CompareInsensitive(str1, str2, str1_len) == 0;
}

bool AreStringsEqualInsensitive(const wchar* str1, uindex str1_len,
                                const wchar* str2, uindex str2_len) {
  if (str1 == nullptr && str2 == nullptr) {
    return true;
  }

  if (str1 == nullptr || str2 == nullptr) {
    return false;
  }

  return CompareInsensitive(str1, str2, str1_len) == 0;
}

bool IsSpace(schar c) { return std::isspace(c); }

bool IsSpace(wchar c) { return std::iswspace(c); }

bool IsAlpha(schar c) { return std::isalpha(c); }

bool IsAlpha(wchar c) { return std::iswalpha(c); }

bool IsEmpty(const schar* str, uindex str_len) {
  COMET_ASSERT(str != nullptr, "String provided is null!");
  return str_len == 0 || str[0] == '\0';
}

bool IsEmpty(const wchar* str, uindex str_len) {
  COMET_ASSERT(str != nullptr, "String provided is null!");
  return str_len == 0 || str[0] == L'\0';
}

schar* Copy(schar* dst, const schar* src, uindex length, uindex dst_offset,
            uindex src_offset) {
  COMET_ASSERT(src != nullptr, "Source string provided is null!");
  COMET_ASSERT(dst != nullptr, "Destination string provided is null!");
  return std::strncpy(dst + dst_offset, src + src_offset, length);
}

wchar* Copy(wchar* dst, const schar* src, uindex length, uindex dst_offset,
            uindex src_offset) {
  COMET_ASSERT(src != nullptr, "Source string provided is null!");
  COMET_ASSERT(dst != nullptr, "Destination string provided is null!");
  std::mbstowcs(dst + dst_offset, src + src_offset, length);
  return dst;
}

schar* Copy(schar* dst, const wchar* src, uindex length, uindex dst_offset,
            uindex src_offset) {
  COMET_ASSERT(src != nullptr, "Source string provided is null!");
  COMET_ASSERT(dst != nullptr, "Destination string provided is null!");
  std::wcstombs(dst + dst_offset, src + src_offset, length);
  return dst;
}

wchar* Copy(wchar* dst, const wchar* src, uindex length, uindex dst_offset,
            uindex src_offset) {
  COMET_ASSERT(src != nullptr, "Source string provided is null!");
  COMET_ASSERT(dst != nullptr, "Destination string provided is null!");
  std::wcsncpy(dst + dst_offset, src + src_offset, length);
  return dst;
}

schar* TrimLeft(schar* str, uindex length, uindex* new_length) {
  return internal::TrimLeft(str, length, new_length);
}

wchar* TrimLeft(wchar* str, uindex length, uindex* new_length) {
  return internal::TrimLeft(str, length, new_length);
}

schar* TrimRight(schar* str, uindex length, uindex* new_length) {
  return internal::TrimRight(str, length, new_length);
}

wchar* TrimRight(wchar* str, uindex length, uindex* new_length) {
  return internal::TrimRight(str, length, new_length);
}

schar* Trim(schar* str, uindex length, uindex* new_length) {
  return internal::Trim(str, length, new_length);
}

wchar* Trim(wchar* str, uindex length, uindex* new_length) {
  return internal::Trim(str, length, new_length);
}

void GetSubString(schar* dst, const schar* src, uindex src_length,
                  uindex offset, uindex length) {
  return internal::GetSubString(dst, src, src_length, offset, length);
}

void GetSubString(wchar* dst, const wchar* src, uindex src_length,
                  uindex offset, uindex length) {
  return internal::GetSubString(dst, src, src_length, offset, length);
}

void FillWith(schar* str, uindex str_length, schar c, uindex offset,
              uindex length) {
  internal ::FillWith(str, str_length, c, offset, length);
}

void FillWith(wchar* str, uindex str_length, wchar c, uindex offset,
              uindex length) {
  internal ::FillWith(str, str_length, c, offset, length);
}

schar ToUpper(schar c) { return std::toupper(c); }

wchar ToUpper(wchar c) { return std::towupper(c); }

schar ToLower(schar c) { return std::tolower(c); }

wchar ToLower(wchar c) { return std::towlower(c); }

void Clear(schar* str, uindex str_len) { FillWith(str, str_len, '\0'); }

void Clear(wchar* str, uindex str_len) { FillWith(str, str_len, L'\0'); }

uindex GetIndexOf(const schar* str, char c, uindex length, uindex offset) {
  return internal::GetIndexOf(str, c, length, offset);
}

uindex GetIndexOf(const wchar* str, char c, uindex length, uindex offset) {
  return internal::GetIndexOf(str, c, length, offset);
}

uindex GetLastIndexOf(const schar* str, uindex length, schar c, uindex offset) {
  return internal::GetLastIndexOf(str, length, c, offset);
}

uindex GetLastIndexOf(const wchar* str, uindex length, wchar c, uindex offset) {
  return internal::GetLastIndexOf(str, length, c, offset);
}

uindex GetNthToLastIndexOf(const schar* str, uindex str_length, schar to_search,
                           uindex count, uindex offset) {
  return internal::GetNthToLastIndexOf(str, str_length, to_search, count,
                                       offset);
}

uindex GetNthToLastIndexOf(const wchar* str, uindex str_length, wchar to_search,
                           uindex count, uindex offset) {
  return internal::GetNthToLastIndexOf(str, str_length, to_search, count,
                                       offset);
}

uindex GetFirstNonWhiteSpaceIndex(const schar* str, uindex str_len) {
  return internal::GetFirstNonWhiteSpaceIndex(str, str_len);
}

uindex GetFirstNonWhiteSpaceIndex(const wchar* str, uindex str_len) {
  return internal::GetFirstNonWhiteSpaceIndex(str, str_len);
}

uindex GetFirstDifferentCharacterIndex(const schar* str, uindex str_len,
                                       schar c) {
  return internal::GetFirstDifferentCharacterIndex(str, str_len, c);
}

uindex GetFirstDifferentCharacterIndex(const wchar* str, uindex str_len,
                                       wchar c) {
  return internal::GetFirstDifferentCharacterIndex(str, str_len, c);
}

uindex GetLastNonWhiteSpaceIndex(const schar* str, uindex str_len) {
  return internal::GetLastNonWhiteSpaceIndex(str, str_len);
}

uindex GetLastNonWhiteSpaceIndex(const wchar* str, uindex str_len) {
  return internal::GetLastNonWhiteSpaceIndex(str, str_len);
}

uindex GetLastNonCharacterIndex(const schar* str, uindex str_len, schar c) {
  return internal::GetLastNonCharacterIndex(str, str_len, c);
}

uindex GetLastNonCharacterIndex(const wchar* str, uindex str_len, wchar c) {
  return internal::GetLastNonCharacterIndex(str, str_len, c);
}

u8 ParseU8(const schar* str) { return static_cast<u8>(std::stoul(str)); }

u8 ParseU8(const wchar* str) {
  wchar* p{nullptr};
  return static_cast<u8>(std::wcstoul(str, &p, 10));
}

u16 ParseU16(const schar* str) { return static_cast<u16>(std::stoul(str)); }

u16 ParseU16(const wchar* str) {
  wchar* p{nullptr};
  return static_cast<u16>(std::wcstoul(str, &p, 10));
}

u32 ParseU32(const schar* str) { return static_cast<u32>(std::stoul(str)); }

u32 ParseU32(const wchar* str) {
  wchar* p{nullptr};
  return static_cast<u32>(std::wcstoul(str, &p, 10));
}

u64 ParseU64(const schar* str) { return static_cast<u64>(std::stoull(str)); }

u64 ParseU64(const wchar* str) {
  wchar* p{nullptr};
  return static_cast<u64>(std::wcstoull(str, &p, 10));
}

s8 ParseS8(const schar* str) { return static_cast<s8>(std::stoi(str)); }

s8 ParseS8(const wchar* str) {
#ifdef COMET_MSVC
  return static_cast<s8>(_wtoi(str));
#else
  wchar* p{nullptr};
  return static_cast<s8>(std::wcstol(str, &p, 10));
#endif  // COMET_MSVC
}

s16 ParseS16(const schar* str) { return static_cast<s16>(std::stoi(str)); }

s16 ParseS16(const wchar* str) {
#ifdef COMET_MSVC
  return static_cast<s16>(_wtoi(str));
#else
  wchar* p{nullptr};
  return static_cast<s16>(std::wcstol(str, &p, 10));
#endif  // COMET_MSVC
}

s32 ParseS32(const schar* str) { return static_cast<s32>(std::stoi(str)); }

s32 ParseS32(const wchar* str) {
#ifdef COMET_MSVC
  return static_cast<s32>(_wtoi(str));
#else
  wchar* p{nullptr};
  return static_cast<s32>(std::wcstol(str, &p, 10));
#endif  // COMET_MSVC
}

s64 ParseS64(const schar* str) { return static_cast<s64>(std::stol(str)); }

s64 ParseS64(const wchar* str) {
  wchar* p{nullptr};
  return static_cast<s32>(std::wcstol(str, &p, 10));
}

f32 ParseF32(const schar* str) { return static_cast<f32>(std::stod(str)); }

f32 ParseF32(const wchar* str) {
  wchar* p{nullptr};
  return static_cast<f32>(std::wcstod(str, &p));
}

f64 ParseF64(const schar* str) { return static_cast<f32>(std::stold(str)); }

f64 ParseF64(const wchar* str) {
  wchar* p{nullptr};
  return static_cast<f32>(std::wcstold(str, &p));
}

uindex ParseIndex(const schar* str) {
  return static_cast<uindex>(std::stoull(str));
}

uindex ParseIndex(const wchar* str) {
  wchar* p{nullptr};
  return static_cast<uindex>(std::wcstoull(str, &p, 10));
}

ux ParseUx(const schar* str) {
#ifdef COMET_64
  return static_cast<ux>(std::stoull(str));
#else
  return static_cast<ux>(std::stoul(str));
#endif  // COMET_64
}

ux ParseUx(const wchar* str) {
  wchar* p{nullptr};
#ifdef COMET_64
  return static_cast<ux>(std::wcstoull(str, &p, 10));
#else
  return static_cast<ux>(std::wcstoul(str, &p, 10));
#endif  // COMET_64
}

sx ParseSx(const schar* str) {
#ifdef COMET_64
  return static_cast<sx>(std::stol(str));
#else
  return static_cast<sx>(std::stoi(str));
#endif  // COMET_64
}

sx ParseSx(const wchar* str) {
#ifdef COMET_64
  wchar* p{nullptr};
  return static_cast<sx>(std::wcstol(str, &p, 10));
#else
#ifdef COMET_MSVC
  return static_cast<sx>(_wtoi(str));
#else
  wchar* p{nullptr};
  return static_cast<sx>(std::wcstol(str, &p, 10));
#endif  // COMET_MSVC
#endif  // COMET_64
}

fx ParseFx(const schar* str) {
#ifdef COMET_64
  return static_cast<fx>(std::stold(str));
#else
  return static_cast<fx>(std::stod(str));
#endif  // COMET_64
}

fx ParseFx(const wchar* str) {
  wchar* p{nullptr};
#ifdef COMET_64
  return static_cast<fx>(std::wcstold(str, &p));
#else
  return static_cast<fx>(std::wcstod(str, &p));
#endif  // COMET_64
}

bool ParseBool(const schar* str) { return internal::ParseBool(str); }

bool ParseBool(const wchar* str) { return internal::ParseBool(str); }

void ConvertToStr(u8 number, schar* buffer, uindex buffer_len,
                  uindex* out_len) {
  // TODO(m4jr0): Optimize function.
  auto len{std::snprintf(buffer, buffer_len - 1, "%" PRIu8, number)};

  if (out_len != nullptr) {
    *out_len = len >= 0 ? len : kInvalidIndex;
  }
}

void ConvertToStr(u8 number, wchar* buffer, uindex buffer_len,
                  uindex* out_len) {
  // TODO(m4jr0): Optimize function.
  auto len{std::swprintf(buffer, buffer_len - 1, L"%" PRIu8, number)};

  if (out_len != nullptr) {
    *out_len = len >= 0 ? len : kInvalidIndex;
  }
}

void ConvertToStr(u16 number, schar* buffer, uindex buffer_len,
                  uindex* out_len) {
  // TODO(m4jr0): Optimize function.
  auto len{std::snprintf(buffer, buffer_len - 1, "%" PRIu16, number)};

  if (out_len != nullptr) {
    *out_len = len >= 0 ? len : kInvalidIndex;
  }
}

void ConvertToStr(u16 number, wchar* buffer, uindex buffer_len,
                  uindex* out_len) {
  // TODO(m4jr0): Optimize function.
  auto len{std::swprintf(buffer, buffer_len - 1, L"%" PRIu16, number)};

  if (out_len != nullptr) {
    *out_len = len >= 0 ? len : kInvalidIndex;
  }
}

void ConvertToStr(u32 number, schar* buffer, uindex buffer_len,
                  uindex* out_len) {
  // TODO(m4jr0): Optimize function.
  auto len{std::snprintf(buffer, buffer_len - 1, "%" PRIu32, number)};

  if (out_len != nullptr) {
    *out_len = len >= 0 ? len : kInvalidIndex;
  }
}

void ConvertToStr(u32 number, wchar* buffer, uindex buffer_len,
                  uindex* out_len) {
  // TODO(m4jr0): Optimize function.
  auto len{std::swprintf(buffer, buffer_len - 1, L"%" PRIu32, number)};

  if (out_len != nullptr) {
    *out_len = len >= 0 ? len : kInvalidIndex;
  }
}

void ConvertToStr(u64 number, schar* buffer, uindex buffer_len,
                  uindex* out_len) {
  // TODO(m4jr0): Optimize function.
  auto len{std::snprintf(buffer, buffer_len - 1, "%" PRIu64, number)};

  if (out_len != nullptr) {
    *out_len = len >= 0 ? len : kInvalidIndex;
  }
}

void ConvertToStr(u64 number, wchar* buffer, uindex buffer_len,
                  uindex* out_len) {
  // TODO(m4jr0): Optimize function.
  auto len{std::swprintf(buffer, buffer_len - 1, L"%" PRIu64, number)};

  if (out_len != nullptr) {
    *out_len = len >= 0 ? len : kInvalidIndex;
  }
}

void ConvertToStr(s8 number, schar* buffer, uindex buffer_len,
                  uindex* out_len) {
  // TODO(m4jr0): Optimize function.
  auto len{std::snprintf(buffer, buffer_len - 1, "%" PRId8, number)};

  if (out_len != nullptr) {
    *out_len = len >= 0 ? len : kInvalidIndex;
  }
}

void ConvertToStr(s8 number, wchar* buffer, uindex buffer_len,
                  uindex* out_len) {
  // TODO(m4jr0): Optimize function.
  auto len{std::swprintf(buffer, buffer_len - 1, L"%" PRId8, number)};

  if (out_len != nullptr) {
    *out_len = len >= 0 ? len : kInvalidIndex;
  }
}

void ConvertToStr(s16 number, schar* buffer, uindex buffer_len,
                  uindex* out_len) {
  // TODO(m4jr0): Optimize function.
  auto len{std::snprintf(buffer, buffer_len - 1, "%" PRId16, number)};

  if (out_len != nullptr) {
    *out_len = len >= 0 ? len : kInvalidIndex;
  }
}

void ConvertToStr(s16 number, wchar* buffer, uindex buffer_len,
                  uindex* out_len) {
  // TODO(m4jr0): Optimize function.
  auto len{std::swprintf(buffer, buffer_len - 1, L"%" PRId16, number)};

  if (out_len != nullptr) {
    *out_len = len >= 0 ? len : kInvalidIndex;
  }
}

void ConvertToStr(s32 number, schar* buffer, uindex buffer_len,
                  uindex* out_len) {
  // TODO(m4jr0): Optimize function.
  auto len{std::snprintf(buffer, buffer_len - 1, "%" PRId32, number)};

  if (out_len != nullptr) {
    *out_len = len >= 0 ? len : kInvalidIndex;
  }
}

void ConvertToStr(s32 number, wchar* buffer, uindex buffer_len,
                  uindex* out_len) {
  // TODO(m4jr0): Optimize function.
  auto len{std::swprintf(buffer, buffer_len - 1, L"%" PRId32, number)};

  if (out_len != nullptr) {
    *out_len = len >= 0 ? len : kInvalidIndex;
  }
}

void ConvertToStr(s64 number, schar* buffer, uindex buffer_len,
                  uindex* out_len) {
  // TODO(m4jr0): Optimize function.
  auto len{std::snprintf(buffer, buffer_len - 1, "%" PRId64, number)};

  if (out_len != nullptr) {
    *out_len = len >= 0 ? len : kInvalidIndex;
  }
}

void ConvertToStr(s64 number, wchar* buffer, uindex buffer_len,
                  uindex* out_len) {
  // TODO(m4jr0): Optimize function.
  auto len{std::swprintf(buffer, buffer_len - 1, L"%" PRId64, number)};

  if (out_len != nullptr) {
    *out_len = len >= 0 ? len : kInvalidIndex;
  }
}
}  // namespace comet
