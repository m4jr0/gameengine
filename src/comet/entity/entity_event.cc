// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "entity_event.h"
////////////////////////////////////////////////////////////////////////////////

namespace comet {
namespace entity {
const stringid::StringId ModelLoadedEvent::kStaticType_{
    COMET_STRING_ID("event_entity_model_loaded")};

ModelLoadedEvent::ModelLoadedEvent(EntityId entity_id)
    : entity_id_{entity_id} {}

stringid::StringId ModelLoadedEvent::GetType() const noexcept {
  return kStaticType_;
}

EntityId ModelLoadedEvent::GetEntityId() const noexcept { return entity_id_; }
}  // namespace entity
}  // namespace comet
