// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_UTILS_DRIVER_UTILS_H_
#define COMET_COMET_RENDERING_UTILS_DRIVER_UTILS_H_

// External. ///////////////////////////////////////////////////////////////////
#include <string_view>
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/essentials.h"
#include "comet/rendering/type/common.h"

namespace comet {
namespace rendering {
DriverType GetDriverTypeFromStr(std::string_view str);
DriverType GetDriverType();

bool IsMultithreading([[maybe_unused]] DriverType type);

AntiAliasingType GetAntiAliasingTypeFromStr(std::string_view str);
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_UTILS_DRIVER_UTILS_H_