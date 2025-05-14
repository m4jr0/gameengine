// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_MATH_COMPRESSION_H_
#define COMET_COMET_MATH_COMPRESSION_H_

#include "comet/core/essentials.h"
#include "comet/math/vector.h"

namespace comet {
namespace math {
namespace internal {
constexpr auto kFloatEpsilon{1e-5f};
void CleanDecompressed(Vec2& vec);
void CleanDecompressed(Vec3& vec);
void CleanDecompressed(Vec4& vec);
}  // namespace internal

Vec2 DecompressVecRl(u16 x, u16 y, u32 bit_count);
Vec2 DecompressVecRl(u16 x, u16 y, f32 min, f32 max, u32 bit_count);
Vec3 DecompressVecRl(u16 x, u16 y, u16 z, u32 bit_count);
Vec3 DecompressVecRl(u16 x, u16 y, u16 z, f32 min, f32 max, u32 bit_count);
Vec4 DecompressVecRl(u16 x, u16 y, u16 z, u16 w, u32 bit_count);
Vec4 DecompressVecRl(u16 x, u16 y, u16 z, u16 w, f32 min, f32 max,
                     u32 bit_count);
}  // namespace math
}  // namespace comet

#endif  // COMET_COMET_MATH_COMPRESSION_H_
