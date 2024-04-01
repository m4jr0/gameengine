// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_COMPRESSION_H_
#define COMET_COMET_CORE_COMPRESSION_H_

#include <vector>
#include "lz4.h"

#include "comet/core/essentials.h"

namespace comet {
void CompressLz4(const std::vector<u8>& src, std::vector<u8>& dst);
void CompressLz4(const u8* src, usize src_size, std::vector<u8>& dst);
void DecompressLz4(const std::vector<u8>& src, usize size,
                   std::vector<u8>& dst);
void DecompressLz4(const u8* src, usize src_size, usize size,
                   std::vector<u8>& dst);
}  // namespace comet

#endif  // COMET_COMET_CORE_COMPRESSION_H_
