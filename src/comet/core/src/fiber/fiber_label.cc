// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_core_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/core/fiber/fiber_label.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/debug/debug_label.h"

namespace comet {
namespace fiber {
const schar* GetFiberSharedLockTypeLabel(FiberSharedLockType type) {
  switch (type) {
    case FiberSharedLockType::Unknown:
      return "unknown";
    case FiberSharedLockType::Shared:
      return "shared";
    case FiberSharedLockType::Exclusive:
      return "exclusive";
    default:
      return kUnknownLabel;
  }
}
}  // namespace fiber
}  // namespace comet