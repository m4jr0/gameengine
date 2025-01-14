// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_COMPRESSION_H_
#define COMET_COMET_CORE_COMPRESSION_H_

#include "lz4.h"

#include "comet/core/essentials.h"
#include "comet/core/memory/allocator/platform_allocator.h"
#include "comet/core/memory/memory.h"
#include "comet/core/type/array.h"

namespace comet {
void CompressLz4(const Array<u8>& src, Array<u8>& dst);
void CompressLz4(const u8* src, usize src_size, Array<u8>& dst);
void DecompressLz4(const u8* src, usize src_size, usize size, u8* dst);
void DecompressLz4(const Array<u8>& src, usize size, Array<u8>& dst);
void DecompressLz4(const u8* src, usize src_size, usize size, Array<u8>& dst);
}  // namespace comet

#endif  // COMET_COMET_CORE_COMPRESSION_H_
