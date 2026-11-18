// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_DATA_RENDER_DRIVER_H_
#define COMET_DATA_RENDER_DRIVER_H_

#include "comet/core/essentials.h"

namespace comet {
namespace render {
enum class DriverType : u8 { Unknown = 0, Empty, OpenGl, Vulkan, Direct3d12 };
}  // namespace render
}  // namespace comet

#endif  // COMET_DATA_RENDER_DRIVER_H_
