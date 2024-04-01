// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "compression.h"

#include <vector>

namespace comet {
void CompressLz4(const std::vector<u8>& src, std::vector<u8>& dst) {
  dst = {};

  if (src.size() <= 0) {
    return;
  }

  const auto compress_staging{LZ4_compressBound(static_cast<s32>(src.size()))};
  dst.resize(compress_staging);

  const auto compressed_size{
      LZ4_compress_default(reinterpret_cast<const schar*>(src.data()),
                           reinterpret_cast<schar*>(dst.data()),
                           static_cast<s32>(src.size()), compress_staging)};
  dst.resize(compressed_size);
}

void CompressLz4(const u8* src, usize src_size, std::vector<u8>& dst) {
  dst = {};

  if (src_size <= 0) {
    return;
  }

  const auto compress_staging{LZ4_compressBound(static_cast<s32>(src_size))};
  dst.resize(compress_staging);

  const auto compressed_size{LZ4_compress_default(
      reinterpret_cast<const schar*>(src), reinterpret_cast<schar*>(dst.data()),
      static_cast<s32>(src_size), compress_staging)};
  dst.resize(compressed_size);
}

void DecompressLz4(const std::vector<u8>& src, usize size,
                   std::vector<u8>& dst) {
  if (src.size() <= 0) {
    return;
  }

  dst.resize(size);

  LZ4_decompress_safe(reinterpret_cast<const schar*>(src.data()),
                      reinterpret_cast<schar*>(dst.data()),
                      sizeof(src[0]) * static_cast<s32>(src.size()),
                      static_cast<s32>(size));
}

void DecompressLz4(const u8* src, usize src_size, usize size,
                   std::vector<u8>& dst) {
  if (src_size <= 0) {
    return;
  }

  dst.resize(size);

  LZ4_decompress_safe(reinterpret_cast<const schar*>(src),
                      reinterpret_cast<schar*>(dst.data()),
                      static_cast<s32>(src_size), static_cast<s32>(size));
}
}  // namespace comet
