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
u32 HashCrC32(const std::string& string);

std::string HashSha256(std::ifstream& stream);
std::string HashSha256(const u8* data, const u8* end);

template <typename Container>
std::string HashSha256(const Container& data) {
  return HashSha256(data.data(), data.data() + data.size());
}

constexpr std::size_t HashCombine(std::size_t lhs, std::size_t rhs) {
  lhs ^= rhs + internal::kMagicNumber + (lhs << 6) + (lhs >> 2);
  return lhs;
}
}  // namespace comet

#endif  // COMET_COMET_CORE_HASH_H_
