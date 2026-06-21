// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RUNTIME_ANIMATION_ANIMATION_OVERRIDE_H_
#define COMET_RUNTIME_ANIMATION_ANIMATION_OVERRIDE_H_

#include "comet/data/animation/animation_clip.h"
#include "comet/core/essentials.h"
#include "comet/core/container/array.h"
#include "comet/core/math/matrix.h"

namespace comet {
namespace animation {
using AnimationOverrideFlags = u8;

enum AnimationOverrideFlagBits : AnimationOverrideFlags {
  kAnimationOverrideFlagBitsNone = 0x0,
  kAnimationOverrideFlagBitsIsLoop = 0x1,
};
}  // namespace animation
}  // namespace comet

#endif  // COMET_RUNTIME_ANIMATION_ANIMATION_OVERRIDE_H_