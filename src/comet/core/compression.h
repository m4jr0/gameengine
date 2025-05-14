// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_COMPRESSION_H_
#define COMET_COMET_CORE_COMPRESSION_H_

#include "comet/core/essentials.h"
#include "comet/core/type/array.h"

namespace comet {
void CompressLz4(const Array<u8>& src, Array<u8>& dst);
void CompressLz4(const u8* src, usize src_size, Array<u8>& dst);
void DecompressLz4(const u8* src, usize src_size, usize size, u8* dst);
void DecompressLz4(const Array<u8>& src, usize size, Array<u8>& dst);
void DecompressLz4(const u8* src, usize src_size, usize size, Array<u8>& dst);
u32 CompressF32Rl(f32 f, u32 bit_count);
f32 DecompressF32Rl(u32 quantized, u32 bit_count);
u32 CompressF32Rl(f32 f, f32 min, f32 max, u32 bit_count);
f32 DecompressF32Rl(u32 quantized, f32 min, f32 max, u32 bit_count);
}  // namespace comet

#endif  // COMET_COMET_CORE_COMPRESSION_H_
