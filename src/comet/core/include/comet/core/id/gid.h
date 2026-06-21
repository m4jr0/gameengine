// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_CORE_ID_GID_H_
#define COMET_CORE_ID_GID_H_

#include "comet/core/essentials.h"

namespace comet {
namespace gid {
using Gid = u32;

constexpr u32 kGenerationBits{8};
constexpr u32 kIndexBits{sizeof(Gid) * 8 - kGenerationBits};
constexpr Gid kGenerationMask{((Gid{1} << kGenerationBits) - 1) << kIndexBits};
constexpr Gid kIndexMask{(Gid{1} << kIndexBits) - 1};
constexpr Gid kIdMask{Gid{static_cast<u32>(-1)}};
constexpr auto kInvalidId{kIdMask};
constexpr u32 kMinFreeIndices{1024};

using IdGeneration =
    std::conditional_t<kGenerationBits <= 16,
                       std::conditional_t<kGenerationBits <= 8, u8, u16>, u32>;

static_assert(sizeof(IdGeneration) * kCharBit >= kGenerationBits,
              "IdGeneration capacity >= required generation bits");
static_assert(sizeof(Gid) > sizeof(IdGeneration),
              "Gid contains more data than IdGeneration");

constexpr bool IsValid(Gid id) noexcept { return id != kInvalidId; }

constexpr Gid GetIndex(Gid id) noexcept { return id & kIndexMask; }

constexpr Gid GetGeneration(Gid id) noexcept {
  return (id & kGenerationMask) >> kIndexBits;
}

constexpr Gid Generate(Gid index, Gid generation) noexcept {
  return (generation << kIndexBits) | index;
}

Gid GenerateNewGeneration(Gid id) noexcept;
}  // namespace gid
}  // namespace comet

#endif  // COMET_CORE_ID_GID_H_