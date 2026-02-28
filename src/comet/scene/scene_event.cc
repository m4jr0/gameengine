// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "scene_event.h"
////////////////////////////////////////////////////////////////////////////////

namespace comet {
namespace scene {
const stringid::StringId SceneLoadRequestEvent::kStaticType_{
    COMET_STRING_ID("event_scene_load_request")};

stringid::StringId SceneLoadRequestEvent::GetType() const noexcept {
  return kStaticType_;
}

const stringid::StringId SceneLoadedEvent::kStaticType_{
    COMET_STRING_ID("event_scene_loaded_event")};

stringid::StringId SceneLoadedEvent::GetType() const noexcept {
  return kStaticType_;
}

const stringid::StringId SceneUnloadedEvent::kStaticType_{
    COMET_STRING_ID("event_scene_unloaded_event")};

stringid::StringId SceneUnloadedEvent::GetType() const noexcept {
  return kStaticType_;
}
}  // namespace scene
}  // namespace comet
