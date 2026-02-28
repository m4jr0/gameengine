// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_TYPE_PRIMITIVE_H_
#define COMET_COMET_CORE_TYPE_PRIMITIVE_H_

// External. ///////////////////////////////////////////////////////////////////
#include <float.h>

#include <climits>
#include <cstddef>
#include <cstdint>
#include <limits>
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/compiler.h"
#include "comet/core/define.h"
#include "comet/core/os.h"

#ifdef COMET_ARCH_X86
#include <emmintrin.h>
#include <xmmintrin.h>
#endif  // COMET_ARCH_X86

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

using usize = std::size_t;
using ssize = std::ptrdiff_t;
using sstreamsize = std::streamsize;

using uptr = std::uintptr_t;
using sptrdiff = std::ptrdiff_t;

using schar = char;
using uchar = unsigned char;
using wchar = wchar_t;

using b8 = s8;
using b32 = s32;

constexpr auto kU8Min{std::numeric_limits<u8>::min()};
constexpr auto kU8Max{std::numeric_limits<u8>::max()};
constexpr auto kU16Min{std::numeric_limits<u16>::min()};
constexpr auto kU16Max{std::numeric_limits<u16>::max()};
constexpr auto kU32Min{std::numeric_limits<u32>::min()};
constexpr auto kU32Max{std::numeric_limits<u32>::max()};
constexpr auto kU64Min{std::numeric_limits<u64>::min()};
constexpr auto kU64Max{std::numeric_limits<u64>::max()};
constexpr auto kS8Min{std::numeric_limits<s8>::min()};
constexpr auto kS8Max{std::numeric_limits<s8>::max()};
constexpr auto kS16Min{std::numeric_limits<s16>::min()};
constexpr auto kS16Max{std::numeric_limits<s16>::max()};
constexpr auto kS32Min{std::numeric_limits<s32>::min()};
constexpr auto kS32Max{std::numeric_limits<s32>::max()};
constexpr auto kS64Min{std::numeric_limits<s64>::min()};
constexpr auto kS64Max{std::numeric_limits<s64>::max()};
constexpr auto kUSizeMin{std::numeric_limits<usize>::min()};
constexpr auto kUSizeMax{std::numeric_limits<usize>::max()};
constexpr auto kSSizeMin{std::numeric_limits<ssize>::min()};
constexpr auto kSSizeMax{std::numeric_limits<ssize>::max()};
constexpr auto kSStreamSizeMin{std::numeric_limits<sstreamsize>::min()};
constexpr auto kSStreamSizeMax{std::numeric_limits<sstreamsize>::max()};
constexpr auto kUPtrMin{std::numeric_limits<uptr>::min()};
constexpr auto kUPtrMax{std::numeric_limits<uptr>::max()};
constexpr auto kSptrDiffMin{std::numeric_limits<sptrdiff>::min()};
constexpr auto kSptrDiffMax{std::numeric_limits<sptrdiff>::max()};
constexpr auto kSCharMin{std::numeric_limits<schar>::min()};
constexpr auto kSCharMax{std::numeric_limits<schar>::max()};
constexpr auto kUCharMin{std::numeric_limits<uchar>::min()};
constexpr auto kUCharMax{std::numeric_limits<uchar>::max()};
constexpr auto kWCharMin{std::numeric_limits<wchar>::min()};
constexpr auto kWCharMax{std::numeric_limits<wchar>::max()};
constexpr auto kF32Min{std::numeric_limits<f32>::min()};
constexpr auto kF32Max{std::numeric_limits<f32>::max()};
constexpr auto kF64Min{std::numeric_limits<f64>::min()};
constexpr auto kF64Max{std::numeric_limits<f64>::max()};
constexpr auto kB8Min{std::numeric_limits<b8>::min()};
constexpr auto kB8Max{std::numeric_limits<b8>::max()};
constexpr auto kB32Min{std::numeric_limits<b32>::min()};
constexpr auto kB32Max{std::numeric_limits<b32>::max()};

#ifdef COMET_WIDE_TCHAR
using tchar = wchar;
constexpr auto kTCharMin{kWCharMin};
constexpr auto kTCharMax{kWCharMax};
#else
using tchar = schar;
constexpr auto kTCharMin{kSCharMin};
constexpr auto kTCharMax{kSCharMax};
#endif  // COMET_WIDE_TCHAR

#ifdef COMET_WIDE_TCHAR
#define COMET_TCHAR(str) L##str
#else
#define COMET_TCHAR(str) str
#endif  // COMET_WIDE_TCHAR

#ifdef COMET_64
using ux = u64;
using sx = s64;
using fx = f64;

constexpr auto kUXMin{kU64Min};
constexpr auto kUXMax{kU64Max};
constexpr auto kSXMin{kS64Min};
constexpr auto kSXMax{kS64Max};
constexpr auto kFXMin{kF64Min};
constexpr auto kFXMax{kF64Max};
#else
using ux = u32;
using sx = s32;
using fx = f32;

constexpr auto kUXMin{kU32Min};
constexpr auto kUXMax{kU32Max};
constexpr auto kSXMin{kS32Min};
constexpr auto kSXMax{kS32Max};
constexpr auto kFXMin{kF32Min};
constexpr auto kFXMax{kF32Max};
#endif  // COMET_64

template <typename T>
struct is_char_pointer : std::false_type {};

template <>
struct is_char_pointer<schar*> : std::true_type {};

template <>
struct is_char_pointer<wchar*> : std::true_type {};

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
static_assert(sizeof(ux) * kCharBit == 32,
              "ux is not 32 bits on this architecture.");
static_assert(sizeof(sx) * kCharBit == 32,
              "sx is not 32 bits on this architecture.");
static_assert(sizeof(fx) * kCharBit == 32,
              "fx is not 32 bits on this architecture.");
#endif  // COMET_64

constexpr auto kInvalidSize{static_cast<usize>(-1)};
constexpr auto kInvalidIndex{kInvalidSize};

#ifdef COMET_ARCH_X86
using ms128 = __m128i;
#endif  // COMET_ARCH_X86
}  // namespace comet

#endif  // COMET_COMET_CORE_TYPE_PRIMITIVE_H_