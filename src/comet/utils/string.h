// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_UTILS_STRING_H_
#define COMET_COMET_UTILS_STRING_H_

#include "comet_precompile.h"

namespace comet {
namespace utils {
namespace string {
uindex GetLastNthPos(std::string_view str, std::string_view to_search,
                     uindex count);
uindex GetLastNthPos(std::string_view str, schar to_search, uindex count);
std::string& TrimLeft(std::string& str);
std::string& TrimRight(std::string& str);
std::string& Trim(std::string& str);
std::string GetLeftTrimmedCopy(std::string str);
std::string GetRightTrimmedCopy(std::string str);
std::string GetTrimmedCopy(std::string str);
uindex GetFirstNonWhiteSpaceIndex(std::string_view str);
uindex GetFirstDifferentCharacterIndex(std::string_view str, schar c);
uindex GetLastNonWhiteSpaceIndex(std::string_view str);
uindex GetLastNonCharacterIndex(std::string_view str, schar c);

u8 ParseU8(const std::string& str);
u8 ParseU8(const schar* str);
u16 ParseU16(const std::string& str);
u16 ParseU16(const schar* str);
u32 ParseU32(const std::string& str);
u32 ParseU32(const schar* str);
u64 ParseU64(const std::string& str);
u64 ParseU64(const schar* str);
s8 ParseS8(const std::string& str);
s8 ParseS8(const schar* str);
s16 ParseS16(const std::string& str);
s16 ParseS16(const schar* str);
s32 ParseS32(const std::string& str);
s32 ParseS32(const schar* str);
s64 ParseS64(const std::string& str);
s64 ParseS64(const schar* str);
f32 ParseF32(const std::string& str);
f32 ParseF32(const schar* str);
f64 ParseF64(const std::string& str);
f64 ParseF64(const schar* str);
uindex ParseIndex(const std::string& str);
uindex ParseIndex(const schar* str);
ux ParseUx(const std::string& str);
ux ParseUx(const schar* str);
sx ParseSx(const std::string& str);
sx ParseSx(const schar* str);
fx ParseFx(const std::string& str);
fx ParseFx(const schar* str);
bool ParseBool(const std::string& str);
bool ParseBool(const schar* str);
}  // namespace string
}  // namespace utils
}  // namespace comet

#endif  // COMET_COMET_UTILS_STRING_H_
