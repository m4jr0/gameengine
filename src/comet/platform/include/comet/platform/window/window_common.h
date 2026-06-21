// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_PLATFORM_WINDOW_WINDOW_COMMON_H_
#define COMET_PLATFORM_WINDOW_WINDOW_COMMON_H_

#include "comet/core/essentials.h"

namespace comet {
namespace platform {
constexpr auto kMaxAppNameLen{256};
constexpr auto kMaxWindowNameLen{256};

using WindowSize = u16;

struct WindowExtent {
  WindowSize width{0};
  WindowSize height{0};
};
}  // namespace platform
}  // namespace comet

#endif  // COMET_PLATFORM_WINDOW_WINDOW_COMMON_H_