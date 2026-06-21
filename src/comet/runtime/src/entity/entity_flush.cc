// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_runtime_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/runtime/entity/entity_flush.h"
////////////////////////////////////////////////////////////////////////////////

namespace comet {
namespace entity {
namespace internal {
bool FlushPlan::IsEmpty() const {
  return entity_moves.IsEmpty() && destroyed_ids.IsEmpty() &&
         entity_in_place_updates.IsEmpty();
}
}  // namespace internal
}  // namespace entity
}  // namespace comet
