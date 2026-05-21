// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RUNTIME_ANIMATION_TYPE_ANIMATION_OVERRIDE_H_
#define COMET_RUNTIME_ANIMATION_TYPE_ANIMATION_OVERRIDE_H_

#include "comet/animation/type/animation_clip.h"
#include "comet/core/essentials.h"
#include "comet/core/type/array.h"
#include "comet/math/matrix.h"

namespace comet {
namespace animation {
using AnimationOverrideFlags = u8;

enum AnimationOverrideFlagBits : AnimationOverrideFlags {
  kAnimationOverrideFlagBitsNone = 0x0,
  kAnimationOverrideFlagBitsIsLoop = 0x1,
};
}  // namespace animation
}  // namespace comet

#endif  // COMET_RUNTIME_ANIMATION_TYPE_ANIMATION_OVERRIDE_H_