// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "window_event.h"

namespace comet {
namespace event {
const core::StringId WindowResizeEvent::kStaticType_ =
    COMET_STRING_ID("event_window_resize");

WindowResizeEvent::WindowResizeEvent(unsigned int width, unsigned int height)
    : width_(width), height_(height) {}

const core::StringId& WindowResizeEvent::GetType() const noexcept {
  return kStaticType_;
}

unsigned int WindowResizeEvent::GetWidth() const noexcept { return width_; }

unsigned int WindowResizeEvent::GetHeight() const noexcept { return height_; }

const core::StringId WindowCloseEvent::kStaticType_ =
    COMET_STRING_ID("event_window_close");

const core::StringId& WindowCloseEvent::GetType() const noexcept {
  return kStaticType_;
}
}  // namespace event
}  // namespace comet
