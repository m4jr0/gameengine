// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_TYPE_PRIMITIVE_H_
#define COMET_COMET_CORE_TYPE_PRIMITIVE_H_

#include <climits>
#include <cstdint>

#include "comet/core/type/os.h"

namespace comet {
constexpr auto kCharBit{CHAR_BIT};

static_assert(sizeof(float) * kCharBit == 32,
              "f32 is not 32 bit on this architecture.");

static_assert(sizeof(double) * kCharBit == 64,
              "f64 is not 64 bit on this architecture.");

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

#ifdef COMET_64
using ux = u64;
using sx = s64;
using fx = f64;
#else
using ux = u32;
using sx = s32;
using fx = f32;
#endif  // COMET_64

constexpr auto kInvalidIndex{static_cast<uindex>(-1)};
}  // namespace comet

#endif  // COMET_COMET_CORE_TYPE_PRIMITIVE_H_