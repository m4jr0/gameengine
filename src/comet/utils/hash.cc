// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "hash.h"

#include "boost/crc.hpp"

namespace comet {
namespace utils {
namespace hash {
unsigned int HashCrC32(const std::string& string) {
  boost::crc_32_type result;
  result.process_bytes(string.data(), string.length());

  return result.checksum();
}
}  // namespace hash
}  // namespace utils
}  // namespace comet
