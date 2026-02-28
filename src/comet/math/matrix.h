// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_MATH_MATRIX_H_
#define COMET_COMET_MATH_MATRIX_H_

// External. ///////////////////////////////////////////////////////////////////
#include "glm/glm.hpp"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/essentials.h"

namespace comet {
namespace math {
// Use aliases before implementing custom library.
using Mat2 = glm::mat2;
using Mat3 = glm::mat3;
using Mat4 = glm::mat4;
using Mat2x2 = Mat2;
using Mat3x3 = Mat3;
using Mat4x4 = Mat4;
using Mat2x3 = glm::mat2x3;
using Mat2x4 = glm::mat2x4;
using Mat3x2 = glm::mat3x2;
using Mat3x4 = glm::mat3x4;
using Mat4x2 = glm::mat4x2;
using Mat4x3 = glm::mat4x3;
}  // namespace math
}  // namespace comet

#endif  // COMET_COMET_MATH_MATRIX_H_
