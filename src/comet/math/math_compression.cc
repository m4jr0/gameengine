// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "math_compression.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/compression.h"
#include "comet/math/math_common.h"

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

void CompressVec2Rl(const Vec2& vec, u32 bit_count, u32& out_x, u32& out_y) {
  out_x = CompressF32Rl(vec.x, bit_count);
  out_y = CompressF32Rl(vec.y, bit_count);
}

void CompressVec2Rl(const Vec2& vec, f32 min, f32 max, u32 bit_count,
                    u32& out_x, u32& out_y) {
  out_x = CompressF32Rl(vec.x, min, max, bit_count);
  out_y = CompressF32Rl(vec.y, min, max, bit_count);
}

void CompressVec3Rl(const Vec3& vec, u32 bit_count, u32& out_x, u32& out_y,
                    u32& out_z) {
  out_x = CompressF32Rl(vec.x, bit_count);
  out_y = CompressF32Rl(vec.y, bit_count);
  out_z = CompressF32Rl(vec.z, bit_count);
}

void CompressVec3Rl(const Vec3& vec, f32 min, f32 max, u32 bit_count,
                    u32& out_x, u32& out_y, u32& out_z) {
  out_x = CompressF32Rl(vec.x, min, max, bit_count);
  out_y = CompressF32Rl(vec.y, min, max, bit_count);
  out_z = CompressF32Rl(vec.z, min, max, bit_count);
}

void CompressVec4Rl(const Vec4& vec, u32 bit_count, u32& out_x, u32& out_y,
                    u32& out_z, u32& out_w) {
  out_x = CompressF32Rl(vec.x, bit_count);
  out_y = CompressF32Rl(vec.y, bit_count);
  out_z = CompressF32Rl(vec.z, bit_count);
  out_w = CompressF32Rl(vec.w, bit_count);
}

void CompressVec4Rl(const Vec4& vec, f32 min, f32 max, u32 bit_count,
                    u32& out_x, u32& out_y, u32& out_z, u32& out_w) {
  out_x = CompressF32Rl(vec.x, min, max, bit_count);
  out_y = CompressF32Rl(vec.y, min, max, bit_count);
  out_z = CompressF32Rl(vec.z, min, max, bit_count);
  out_w = CompressF32Rl(vec.w, min, max, bit_count);
}

Vec2 DecompressVec2Rl(u16 x, u16 y, u32 bit_count) {
  Vec2 vec{};
  vec.x = DecompressF32Rl(x, bit_count);
  vec.y = DecompressF32Rl(y, bit_count);
  internal::CleanDecompressed(vec);
  return vec;
}

Vec2 DecompressVec2Rl(u16 x, u16 y, f32 min, f32 max, u32 bit_count) {
  Vec2 vec{};
  vec.x = DecompressF32Rl(x, min, max, bit_count);
  vec.y = DecompressF32Rl(y, min, max, bit_count);
  internal::CleanDecompressed(vec);
  return vec;
}

Vec3 DecompressVec3Rl(u16 x, u16 y, u16 z, u32 bit_count) {
  Vec3 vec{};
  vec.x = DecompressF32Rl(x, bit_count);
  vec.y = DecompressF32Rl(y, bit_count);
  vec.z = DecompressF32Rl(z, bit_count);
  internal::CleanDecompressed(vec);
  return vec;
}

Vec3 DecompressVec3Rl(u16 x, u16 y, u16 z, f32 min, f32 max, u32 bit_count) {
  Vec3 vec{};
  vec.x = DecompressF32Rl(x, min, max, bit_count);
  vec.y = DecompressF32Rl(y, min, max, bit_count);
  vec.z = DecompressF32Rl(z, min, max, bit_count);
  internal::CleanDecompressed(vec);
  return vec;
}

Vec4 DecompressVec4Rl(u16 x, u16 y, u16 z, u16 w, u32 bit_count) {
  Vec4 vec{};
  vec.x = DecompressF32Rl(x, bit_count);
  vec.y = DecompressF32Rl(y, bit_count);
  vec.z = DecompressF32Rl(z, bit_count);
  vec.w = DecompressF32Rl(w, bit_count);
  internal::CleanDecompressed(vec);
  return vec;
}

Vec4 DecompressVec4Rl(u16 x, u16 y, u16 z, u16 w, f32 min, f32 max,
                      u32 bit_count) {
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
