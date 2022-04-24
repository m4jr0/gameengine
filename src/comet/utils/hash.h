// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_UTILS_HASH_H_
#define COMET_COMET_UTILS_HASH_H_

#include <string>

#include "picosha2.h"

#include "comet/core/type/primitive.h"

namespace comet {
namespace utils {
namespace hash {
u32 HashCrC32(const void* data, uindex length);
u32 HashCrC32(const std::string& string);

std::string HashSha256(std::ifstream& stream);
std::string HashSha256(const s8* data, const s8* end);

template <typename Container>
std::string HashSha256(const Container& data) {
  return HashSha256(data.data(), data.data() + data.size());
}
}  // namespace hash
}  // namespace utils
}  // namespace comet

#endif  // COMET_COMET_UTILS_HASH_H_
