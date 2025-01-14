// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "comet_pch.h"

#include "scene_event.h"

namespace comet {
namespace scene {
const stringid::StringId SceneLoadRequestEvent::kStaticType_{
    COMET_STRING_ID("event_scene_load_request")};

SceneLoadRequestEvent::SceneLoadRequestEvent(frame::FramePacket* frame_packet)
    : frame_packet_{frame_packet} {}

stringid::StringId SceneLoadRequestEvent::GetType() const noexcept {
  return kStaticType_;
}

frame::FramePacket* SceneLoadRequestEvent::GetFramePacket() const noexcept {
  return frame_packet_;
}

const stringid::StringId SceneLoadedEvent::kStaticType_{
    COMET_STRING_ID("event_scene_loaded_event")};

stringid::StringId SceneLoadedEvent::GetType() const noexcept {
  return kStaticType_;
}
}  // namespace scene
}  // namespace comet
