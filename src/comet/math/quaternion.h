// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_MATH_QUATERNION_H_
#define COMET_COMET_MATH_QUATERNION_H_

#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"

#include "comet/core/essentials.h"
#include "comet/math/vector.h"

namespace comet {
namespace math {
// Use aliases before implementing custom library.
using Quat = glm::quat;

Quat& Normalize(Quat& quaternion);
Quat GetNormalizedCopy(const Quat& quaternion);
Quat GetQuaternionRotation(f32 angle, const Vec3& axis);
}  // namespace math
}  // namespace comet

#endif  // COMET_COMET_MATH_QUATERNION_H_
