// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_runtime_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/runtime/entity/label/entity_changes_fence_label.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/debug/debug_label.h"

namespace comet {
namespace entity {
const schar* GetEntityChangesFenceStateLabel(EntityChangesFenceState state) {
  switch (state) {
    case EntityChangesFenceState::Idle:
      return "idle";
    case EntityChangesFenceState::Waiting:
      return "waiting";
    case EntityChangesFenceState::Passed:
      return "passed";
    default:
      return kUnknownLabel;
  }
}
}  // namespace entity
}  // namespace comet