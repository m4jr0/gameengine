// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RENDER_UTILS_DRIVER_UTILS_H_
#define COMET_RENDER_UTILS_DRIVER_UTILS_H_

// External. ///////////////////////////////////////////////////////////////////
#include <string_view>
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/essentials.h"
#include "comet/render/common.h"

namespace comet {
namespace render {
DriverType GetDriverTypeFromStr(std::string_view str);
DriverType GetDriverType();

bool IsMultithreading([[maybe_unused]] DriverType type);

AntiAliasingType GetAntiAliasingTypeFromStr(std::string_view str);

u8 GetMsaaSampleCount(AntiAliasingType anti_aliasing_type);
}  // namespace render
}  // namespace comet

#endif  // COMET_RENDER_UTILS_DRIVER_UTILS_H_