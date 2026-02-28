// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "compression.h"
////////////////////////////////////////////////////////////////////////////////

// External. ///////////////////////////////////////////////////////////////////
#include "lz4.h"
////////////////////////////////////////////////////////////////////////////////

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

u32 CompressF32Rl(f32 f, u32 bit_count) {
  auto interval_count{static_cast<u32>(1 << bit_count)};
  auto scaled{f * static_cast<f32>(interval_count - 1)};
  auto rounded{static_cast<u32>(scaled + .5f)};

  if (rounded > interval_count - 1) {
    rounded = interval_count - 1;
  }

  return rounded;
}

f32 DecompressF32Rl(u32 quantized, u32 bit_count) {
  auto interval_count{1 << bit_count};
  auto interval_size{1.0f / static_cast<f32>(interval_count - 1)};
  return static_cast<f32>(quantized) * interval_size;
}

u32 CompressF32Rl(f32 f, f32 min, f32 max, u32 bit_count) {
  COMET_ASSERT(min <= f && f <= max,
               "Float value to be compressed is out of bounds: ", f,
               ", min: ", min, "max: ", max, "!");
  f = (f - min) / (max - min);
  auto quantized{CompressF32Rl(f, bit_count)};
  return quantized;
}

f32 DecompressF32Rl(u32 quantized, f32 min, f32 max, u32 bit_count) {
  auto f{DecompressF32Rl(quantized, bit_count)};
  return min + (f * (max - min));
}
}  // namespace comet
