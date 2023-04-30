// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "window_event.h"

namespace comet {
namespace event {
const stringid::StringId WindowInitializedEvent::kStaticType_{
    COMET_STRING_ID("event_window_initialize")};

WindowInitializedEvent::WindowInitializedEvent(rendering::WindowSize width,
                                               rendering::WindowSize height)
    : width_{width}, height_{height} {}

stringid::StringId WindowInitializedEvent::GetType() const noexcept {
  return kStaticType_;
}

rendering::WindowSize WindowInitializedEvent::GetWidth() const noexcept {
  return width_;
}

rendering::WindowSize WindowInitializedEvent::GetHeight() const noexcept {
  return height_;
}

const stringid::StringId WindowResizeEvent::kStaticType_{
    COMET_STRING_ID("event_window_resize")};

WindowResizeEvent::WindowResizeEvent(rendering::WindowSize width,
                                     rendering::WindowSize height)
    : width_{width}, height_{height} {}

stringid::StringId WindowResizeEvent::GetType() const noexcept {
  return kStaticType_;
}

rendering::WindowSize WindowResizeEvent::GetWidth() const noexcept {
  return width_;
}

rendering::WindowSize WindowResizeEvent::GetHeight() const noexcept {
  return height_;
}

const stringid::StringId WindowCloseEvent::kStaticType_{
    COMET_STRING_ID("event_window_close")};

stringid::StringId WindowCloseEvent::GetType() const noexcept {
  return kStaticType_;
}
}  // namespace event
}  // namespace comet
