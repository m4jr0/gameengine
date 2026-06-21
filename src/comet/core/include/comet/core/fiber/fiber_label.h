// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_CORE_FIBER_FIBER_LABEL_H_
#define COMET_CORE_FIBER_FIBER_LABEL_H_

#include "comet/core/fiber/fiber_primitive.h"
#include "comet/core/essentials.h"

namespace comet {
namespace fiber {
const schar* GetFiberSharedLockTypeLabel(FiberSharedLockType type);
}  // namespace fiber
}  // namespace comet

#endif  // COMET_CORE_FIBER_FIBER_LABEL_H_