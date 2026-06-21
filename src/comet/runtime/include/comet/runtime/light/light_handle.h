// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RUNTIME_LIGHT_LIGHT_HANDLE_H_
#define COMET_RUNTIME_LIGHT_LIGHT_HANDLE_H_

#include "comet/core/essentials.h"
#include "comet/core/handle/handle.h"

namespace comet {
namespace light {
struct LightTag {};
using LightHandle = Handle<LightTag>;
}  // namespace light
}  // namespace comet

#endif  // COMET_RUNTIME_LIGHT_LIGHT_HANDLE_H_