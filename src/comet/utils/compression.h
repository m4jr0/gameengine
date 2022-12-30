// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_UTILS_COMPRESSION_H_
#define COMET_COMET_UTILS_COMPRESSION_H_

#include "comet_precompile.h"

#include "lz4.h"

namespace comet {
namespace utils {
namespace compression {
template <typename T>
void CompressLz4(const T& src, uindex size, std::vector<char>& dst) {
  const auto compress_staging{LZ4_compressBound(size)};

  dst.resize(compress_staging);

  const auto compressed_size{LZ4_compress_default(
      reinterpret_cast<const char*>(&src), dst.data(), size, compress_staging)};

  dst.resize(compressed_size);
}

template <typename T>
void CompressLz4(const std::vector<T>& src, uindex size,
                 std::vector<char>& dst) {
  dst = {};

  if (src.size() <= 0) {
    return;
  }

  const auto compress_staging{LZ4_compressBound(sizeof(src[0]) * src.size())};
  dst.resize(compress_staging);

  const auto compressed_size{
      LZ4_compress_default(reinterpret_cast<const char*>(src.data()),
                           dst.data(), size, compress_staging)};
  dst.resize(compressed_size);
}

template <typename T>
void DecompressLz4(const std::vector<char>& src, uindex size, T& dst) {
  if (src.size() <= 0) {
    return;
  }

  LZ4_decompress_safe(src.data(), reinterpret_cast<char*>(&dst),
                      sizeof(src[0]) * src.size(), size);
}

template <typename T>
void DecompressLz4(const std::vector<char>& src, uindex size,
                   std::vector<T>& dst) {
  if (src.size() <= 0) {
    return;
  }

  dst.resize(size);

  LZ4_decompress_safe(src.data(), reinterpret_cast<char*>(dst.data()),
                      sizeof(src[0]) * src.size(), size);
}
}  // namespace compression
}  // namespace utils
}  // namespace comet

#endif  // COMET_COMET_UTILS_COMPRESSION_H_
