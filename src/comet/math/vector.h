// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_MATH_VECTOR_H_
#define COMET_COMET_MATH_VECTOR_H_

#include "glm/glm.hpp"

#include "comet/core/essentials.h"

namespace comet {
namespace math {
// Use aliases before implementing custom library.
using Vec2 = glm::vec2;
using Vec3 = glm::vec3;
using Vec4 = glm::vec4;

f32 GetMagnitude(const Vec2& vec);
f32 GetMagnitude(const Vec3& vec);
f32 GetMagnitude(const Vec4& vec);
f32 GetSquaredMagnitude(const Vec2& vec);
f32 GetSquaredMagnitude(const Vec3& vec);
f32 GetSquaredMagnitude(const Vec4& vec);
Vec2& Normalize(Vec2& vector);
Vec3& Normalize(Vec3& vector);
Vec4& Normalize(Vec4& vector);
Vec2 GetNormalizedCopy(const Vec2& vector);
Vec3 GetNormalizedCopy(const Vec3& vector);
Vec4 GetNormalizedCopy(const Vec4& vector);
Vec2& NormalizeFast(Vec2& vector);
Vec3& NormalizeFast(Vec3& vector);
Vec4& NormalizeFast(Vec4& vector);
Vec2 GetNormalizedCopyFast(const Vec2& vector);
Vec3 GetNormalizedCopyFast(const Vec3& vector);
Vec4 GetNormalizedCopyFast(const Vec4& vector);
f32 Dot(const Vec2& a, const Vec2& b);
f32 Dot(const Vec3& a, const Vec3& b);
f32 Dot(const Vec4& a, const Vec4& b);
Vec3 Cross(const Vec3& a, const Vec3& b);
}  // namespace math
}  // namespace comet

#endif  // COMET_COMET_MATH_VECTOR_H_
