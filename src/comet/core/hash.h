// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_HASH_H_
#define COMET_COMET_CORE_HASH_H_

#include <string>

#include "picosha2.h"

#include "comet/core/type/primitive.h"
#include "comet/math/math_commons.h"

namespace comet {
namespace internal {
constexpr ux kPhi{(1 + math::CtSqrt(ux{5})) / 2};
constexpr ux kMagicNumber{2 ^ kCharBit / kPhi};
}  // namespace internal

u32 HashCrC32(const void* data, uindex length);

constexpr auto kSha256DigestSize{picosha2::k_digest_size};

void HashSha256(std::ifstream& stream, schar* buffer, uindex buffer_len);

constexpr std::size_t HashCombine(std::size_t lhs, std::size_t rhs) {
  lhs ^= rhs + internal::kMagicNumber + (lhs << 6) + (lhs >> 2);
  return lhs;
}
}  // namespace comet

#endif  // COMET_COMET_CORE_HASH_H_
