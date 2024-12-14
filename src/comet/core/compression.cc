// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "compression.h"

namespace comet {
void CompressLz4(const Array<u8>& src, Array<u8>& dst) {
  dst.Clear();

  if (src.GetSize() <= 0) {
    return;
  }

  const auto compress_staging{
      LZ4_compressBound(static_cast<s32>(src.GetSize()))};
  dst.Resize(compress_staging);

  const auto compressed_size{
      LZ4_compress_default(reinterpret_cast<const schar*>(src.GetData()),
                           reinterpret_cast<schar*>(dst.GetData()),
                           static_cast<s32>(src.GetSize()), compress_staging)};
  dst.Resize(compressed_size);
}

void CompressLz4(const u8* src, usize src_size, Array<u8>& dst) {
  dst.Clear();

  if (src_size <= 0) {
    return;
  }

  const auto compress_staging{LZ4_compressBound(static_cast<s32>(src_size))};
  dst.Resize(compress_staging);

  const auto compressed_size{
      LZ4_compress_default(reinterpret_cast<const schar*>(src),
                           reinterpret_cast<schar*>(dst.GetData()),
                           static_cast<s32>(src_size), compress_staging)};
  dst.Resize(compressed_size);
}

void DecompressLz4(const u8* src, usize src_size, usize size, u8* dst) {
  if (src_size <= 0) {
    return;
  }

  LZ4_decompress_safe(reinterpret_cast<const schar*>(src),
                      reinterpret_cast<schar*>(dst), static_cast<s32>(src_size),
                      static_cast<s32>(size));
}

void DecompressLz4(const Array<u8>& src, usize size, Array<u8>& dst) {
  if (src.GetSize() <= 0) {
    return;
  }

  dst.Resize(size);
  DecompressLz4(src.GetData(), src.GetSize(), size, dst.GetData());
}

void DecompressLz4(const u8* src, usize src_size, usize size, Array<u8>& dst) {
  if (src_size <= 0) {
    return;
  }

  dst.Resize(size);

  LZ4_decompress_safe(reinterpret_cast<const schar*>(src),
                      reinterpret_cast<schar*>(dst.GetData()),
                      static_cast<s32>(src_size), static_cast<s32>(size));
}
}  // namespace comet
