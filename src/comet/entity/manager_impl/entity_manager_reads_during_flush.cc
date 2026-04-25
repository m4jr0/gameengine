// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/entity/entity_manager.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/essentials.h"

#ifndef COMET_DEBUG_ENTITY
#include "comet/core/logger/logging.h"
#endif  // !COMET_DEBUG_ENTITY

namespace comet {
namespace entity {
#ifdef COMET_DEBUG
void EntityManager::DiagnosePublicReadDuringFlush(const schar* context) const {
  if (!is_flushing_snapshot_.load(std::memory_order_acquire)) {
    return;
  }

#ifdef COMET_DEBUG_ENTITY
  COMET_ASSERT(false, context, "public entity read during entity flush");
#else
  COMET_LOG_WARNING(LoggerType::Entity, context,
                    "public entity read during entity flush");
#endif  // COMET_DEBUG_ENTITY
}
#endif  // COMET_DEBUG
}  // namespace entity
}  // namespace comet