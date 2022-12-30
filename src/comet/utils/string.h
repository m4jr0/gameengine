// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_UTILS_STRING_H_
#define COMET_COMET_UTILS_STRING_H_

#include "comet_precompile.h"

namespace comet {
namespace utils {
namespace string {
uindex GetSubStrNthPos(const std::string& str, const char* to_search,
                       uindex count);
uindex GetSubStrNthPos(const std::string& str, const std::string& to_search,
                       uindex count);
std::string& TrimLeft(std::string& str);
std::string& TrimRight(std::string& str);
std::string& Trim(std::string& str);
std::string GetLeftTrimmedCopy(std::string str);
std::string GetRightTrimmedCopy(std::string str);
std::string GetTrimmedCopy(std::string str);

u8 ParseU8(const std::string& str);
u8 ParseU8(const char* str);
u16 ParseU16(const std::string& str);
u16 ParseU16(const char* str);
u32 ParseU32(const std::string& str);
u32 ParseU32(const char* str);
u64 ParseU64(const std::string& str);
u64 ParseU64(const char* str);
s8 ParseS8(const std::string& str);
s8 ParseS8(const char* str);
s16 ParseS16(const std::string& str);
s16 ParseS16(const char* str);
s32 ParseS32(const std::string& str);
s32 ParseS32(const char* str);
s64 ParseS64(const std::string& str);
s64 ParseS64(const char* str);
f32 ParseF32(const std::string& str);
f32 ParseF32(const char* str);
f64 ParseF64(const std::string& str);
f64 ParseF64(const char* str);
uindex ParseIndex(const std::string& str);
uindex ParseIndex(const char* str);
ux ParseUx(const std::string& str);
ux ParseUx(const char* str);
sx ParseSx(const std::string& str);
sx ParseSx(const char* str);
fx ParseFx(const std::string& str);
fx ParseFx(const char* str);
bool ParseBool(const std::string& str);
bool ParseBool(const char* str);
}  // namespace string
}  // namespace utils
}  // namespace comet

#endif  // COMET_COMET_UTILS_STRING_H_
