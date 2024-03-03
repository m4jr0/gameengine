// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "vector.h"

#include "comet/math/math_commons.h"

namespace comet {
namespace math {
f32 GetMagnitude(const Vec2& vec) { return Sqrt(Dot(vec, vec)); }

f32 GetMagnitude(const Vec3& vec) { return Sqrt(Dot(vec, vec)); }

f32 GetMagnitude(const Vec4& vec) { return Sqrt(Dot(vec, vec)); }

f32 GetSquaredMagnitude(const Vec2& vec) { return Dot(vec, vec); }

f32 GetSquaredMagnitude(const Vec3& vec) { return Dot(vec, vec); }

f32 GetSquaredMagnitude(const Vec4& vec) { return Dot(vec, vec); }

Vec2& Normalize(Vec2& vector) {
  vector /= Sqrt(Dot(vector, vector));
  return vector;
}

Vec3& Normalize(Vec3& vector) {
  vector /= Sqrt(Dot(vector, vector));
  return vector;
}

Vec4& Normalize(Vec4& vector) {
  vector /= Sqrt(Dot(vector, vector));
  return vector;
}

Vec2 GetNormalizedCopy(const Vec2& vector) {
  auto copy{vector};
  Normalize(copy);
  return copy;
}

Vec3 GetNormalizedCopy(const Vec3& vector) {
  auto copy{vector};
  Normalize(copy);
  return copy;
}

Vec4 GetNormalizedCopy(const Vec4& vector) {
  auto copy{vector};
  Normalize(copy);
  return copy;
}

Vec2& NormalizeFast(Vec2& vector) {
  vector *= FastInvSqrt(Dot(vector, vector));
  return vector;
}

Vec3& NormalizeFast(Vec3& vector) {
  vector *= FastInvSqrt(Dot(vector, vector));
  return vector;
}

Vec4& NormalizeFast(Vec4& vector) {
  vector *= FastInvSqrt(Dot(vector, vector));
  return vector;
}

Vec2 GetNormalizedCopyFast(const Vec2& vector) {
  auto copy{vector};
  Normalize(copy);
  return copy;
}

Vec3 GetNormalizedCopyFast(const Vec3& vector) {
  auto copy{vector};
  Normalize(copy);
  return copy;
}

Vec4 GetNormalizedCopyFast(const Vec4& vector) {
  auto copy{vector};
  Normalize(copy);
  return copy;
}

f32 Dot(const Vec2& a, const Vec2& b) { return a.x * b.x + a.y * b.y; }

f32 Dot(const Vec3& a, const Vec3& b) {
  return a.x * b.x + a.y * b.y + a.z * b.z;
}

f32 Dot(const Vec4& a, const Vec4& b) {
  return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

Vec3 Cross(const Vec3& a, const Vec3& b) {
  return {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
}
}  // namespace math
}  // namespace comet
