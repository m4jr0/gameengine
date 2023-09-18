// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_TYPE_PRIMITIVE_H_
#define COMET_COMET_CORE_TYPE_PRIMITIVE_H_

#include <float.h>

#include <climits>
#include <cstddef>
#include <cstdint>

#include "comet/core/define.h"
#include "comet/core/os.h"

namespace comet {
constexpr auto kCharBit{CHAR_BIT};

using u8 = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;

using s8 = std::int8_t;
using s16 = std::int16_t;
using s32 = std::int32_t;
using s64 = std::int64_t;

using f32 = float;
using f64 = double;

using uindex = std::size_t;
using uptr = std::uintptr_t;
using sptrdiff = std::ptrdiff_t;

using schar = char;
using uchar = unsigned char;
using wchar = wchar_t;

#ifdef COMET_WIDE_TCHAR
using tchar = wchar;
#else
using tchar = schar;
#endif  // COMET_WIDE_TCHAR

#ifdef COMET_WIDE_TCHAR
#define COMET_TCHAR(str) L##str
#else
#define COMET_TCHAR(str) str
#endif  // COMET_WIDE_TCHAR

using b8 = s8;
using b32 = s32;

#ifdef COMET_64
using ux = u64;
using sx = s64;
using fx = f64;
#else
using ux = u32;
using sx = s32;
using fx = f32;
#endif  // COMET_64

constexpr auto kF32Min{FLT_MIN};
constexpr auto kF32Max{FLT_MAX};
constexpr auto kF64Min{DBL_MIN};
constexpr auto kF64Max{DBL_MAX};

static_assert(sizeof(u8) * kCharBit == 8,
              "u8 is not 8 bits on this architecture.");
static_assert(sizeof(u16) * kCharBit == 16,
              "u16 is not 16 bits on this architecture.");
static_assert(sizeof(u32) * kCharBit == 32,
              "u32 is not 32 bits on this architecture.");
static_assert(sizeof(u64) * kCharBit == 64,
              "u64 is not 64 bits on this architecture.");

static_assert(sizeof(s8) * kCharBit == 8,
              "s8 is not 8 bits on this architecture.");
static_assert(sizeof(s16) * kCharBit == 16,
              "s16 is not 16 bits on this architecture.");
static_assert(sizeof(s32) * kCharBit == 32,
              "s32 is not 32 bits on this architecture.");
static_assert(sizeof(s64) * kCharBit == 64,
              "s64 is not 64 bits on this architecture.");

static_assert(sizeof(f32) * kCharBit == 32,
              "f32 is not 32 bits on this architecture.");
static_assert(sizeof(f64) * kCharBit == 64,
              "f64 is not 64 bits on this architecture.");

static_assert(sizeof(schar) * kCharBit == 8,
              "schar is not 8 bits on this architecture.");
static_assert(sizeof(uchar) * kCharBit == 8,
              "uchar is not 8 bits on this architecture.");

static_assert(sizeof(b8) * kCharBit == 8,
              "b8 is not 8 bits on this architecture.");
static_assert(sizeof(b32) * kCharBit == 32,
              "b32 is not 32 bits on this architecture.");

#ifdef COMET_64
static_assert(sizeof(ux) * kCharBit == 64,
              "ux is not 64 bits on this architecture.");
static_assert(sizeof(sx) * kCharBit == 64,
              "sx is not 64 bits on this architecture.");
static_assert(sizeof(fx) * kCharBit == 64,
              "fx is not 64 bits on this architecture.");
#else
static_assert(sizeof(ux) * kCharBit == 64,
              "ux is not 64 bits on this architecture.");
static_assert(sizeof(sx) * kCharBit == 64,
              "sx is not 64 bits on this architecture.");
static_assert(sizeof(fx) * kCharBit == 64,
              "fx is not 64 bits on this architecture.");
#endif  // COMET_64

constexpr auto kInvalidIndex{static_cast<uindex>(-1)};
}  // namespace comet

#endif  // COMET_COMET_CORE_TYPE_PRIMITIVE_H_