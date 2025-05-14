// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "math_compression.h"

#include "comet/core/compression.h"
#include "comet/math/math_commons.h"
#include "comet_pch.h"

namespace comet {
namespace math {
namespace internal {
void CleanDecompressed(Vec2& vec) {
  if (Abs(vec.x) < kFloatEpsilon) {
    vec.x = 0.0f;
  }

  if (Abs(vec.y) < kFloatEpsilon) {
    vec.y = 0.0f;
  }
}

void CleanDecompressed(Vec3& vec) {
  if (Abs(vec.x) < kFloatEpsilon) {
    vec.x = 0.0f;
  }

  if (Abs(vec.y) < kFloatEpsilon) {
    vec.y = 0.0f;
  }

  if (Abs(vec.z) < kFloatEpsilon) {
    vec.z = 0.0f;
  }
}

void CleanDecompressed(Vec4& vec) {
  if (Abs(vec.x) < kFloatEpsilon) {
    vec.x = 0.0f;
  }

  if (Abs(vec.y) < kFloatEpsilon) {
    vec.y = 0.0f;
  }

  if (Abs(vec.z) < kFloatEpsilon) {
    vec.z = 0.0f;
  }

  if (Abs(vec.w) < kFloatEpsilon) {
    vec.w = 0.0f;
  }
}
}  // namespace internal

Vec2 DecompressVecRl(u16 x, u16 y, u32 bit_count) {
  Vec2 vec{};
  vec.x = DecompressF32Rl(x, bit_count);
  vec.y = DecompressF32Rl(y, bit_count);
  internal::CleanDecompressed(vec);
  return vec;
}

Vec2 DecompressVecRl(u16 x, u16 y, f32 min, f32 max, u32 bit_count) {
  Vec2 vec{};
  vec.x = DecompressF32Rl(x, min, max, bit_count);
  vec.y = DecompressF32Rl(y, min, max, bit_count);
  internal::CleanDecompressed(vec);
  return vec;
}

Vec3 DecompressVecRl(u16 x, u16 y, u16 z, u32 bit_count) {
  Vec3 vec{};
  vec.x = DecompressF32Rl(x, bit_count);
  vec.y = DecompressF32Rl(y, bit_count);
  vec.z = DecompressF32Rl(z, bit_count);
  internal::CleanDecompressed(vec);
  return vec;
}

Vec3 DecompressVecRl(u16 x, u16 y, u16 z, f32 min, f32 max, u32 bit_count) {
  Vec3 vec{};
  vec.x = DecompressF32Rl(x, min, max, bit_count);
  vec.y = DecompressF32Rl(y, min, max, bit_count);
  vec.z = DecompressF32Rl(z, min, max, bit_count);
  internal::CleanDecompressed(vec);
  return vec;
}

Vec4 DecompressVecRl(u16 x, u16 y, u16 z, u16 w, u32 bit_count) {
  Vec4 vec{};
  vec.x = DecompressF32Rl(x, bit_count);
  vec.y = DecompressF32Rl(y, bit_count);
  vec.z = DecompressF32Rl(z, bit_count);
  vec.w = DecompressF32Rl(w, bit_count);
  internal::CleanDecompressed(vec);
  return vec;
}

Vec4 DecompressVecRl(u16 x, u16 y, u16 z, u16 w, f32 min, f32 max, u32 bit_count) {
  Vec4 vec{};
  vec.x = DecompressF32Rl(x, min, max, bit_count);
  vec.y = DecompressF32Rl(y, min, max, bit_count);
  vec.z = DecompressF32Rl(z, min, max, bit_count);
  vec.w = DecompressF32Rl(w, min, max, bit_count);
  internal::CleanDecompressed(vec);
  return vec;
}
}  // namespace math
}  // namespace comet
