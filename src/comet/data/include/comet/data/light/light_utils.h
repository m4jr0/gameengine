// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_DATA_LIGHT_LIGHT_UTILS_H_
#define COMET_DATA_LIGHT_LIGHT_UTILS_H_

#include "comet/core/essentials.h"
#include "comet/data/light/light.h"

namespace comet {
namespace light {
ShadowType ResolveShadowType(const LightProperties& props,
                             const LightShadow& shadow);

bool IsShadowSupportedForLight(const LightProperties& props,
                               const LightShadow& shadow);
}  // namespace light
}  // namespace comet

#endif  // COMET_DATA_LIGHT_LIGHT_UTILS_H_
