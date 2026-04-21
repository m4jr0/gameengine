// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_LABEL_RENDERING_COMMON_LABEL_H_
#define COMET_COMET_RENDERING_LABEL_RENDERING_COMMON_LABEL_H_

#include "comet/core/essentials.h"
#include "comet/rendering/type/rendering_common_type.h"

namespace comet {
namespace rendering {
const schar* GetDriverTypeLabel(DriverType type);
const schar* GetAntiAliasingTypeLabel(AntiAliasingType type);
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_LABEL_RENDERING_COMMON_LABEL_H_