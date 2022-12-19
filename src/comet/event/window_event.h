// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_EVENT_WINDOW_EVENT_H_
#define COMET_COMET_EVENT_WINDOW_EVENT_H_

#include "comet_precompile.h"

#include "comet/event/event.h"
#include "comet/rendering/rendering_common.h"

namespace comet {
namespace event {
class WindowResizeEvent : public Event {
 public:
  const static stringid::StringId kStaticType_;

  WindowResizeEvent(rendering::WindowSize width, rendering::WindowSize height);
  WindowResizeEvent(const WindowResizeEvent&) = default;
  WindowResizeEvent(WindowResizeEvent&&) noexcept = default;
  WindowResizeEvent& operator=(const WindowResizeEvent&) = default;
  WindowResizeEvent& operator=(WindowResizeEvent&&) noexcept = default;
  virtual ~WindowResizeEvent() = default;

  stringid::StringId GetType() const noexcept override;
  rendering::WindowSize GetWidth() const noexcept;
  rendering::WindowSize GetHeight() const noexcept;

 private:
  rendering::WindowSize width_{0};
  rendering::WindowSize height_{0};
};

class WindowCloseEvent : public Event {
 public:
  const static stringid::StringId kStaticType_;

  WindowCloseEvent() = default;
  WindowCloseEvent(const WindowCloseEvent&) = default;
  WindowCloseEvent(WindowCloseEvent&&) noexcept = default;
  WindowCloseEvent& operator=(const WindowCloseEvent&) = default;
  WindowCloseEvent& operator=(WindowCloseEvent&&) noexcept = default;
  virtual ~WindowCloseEvent() = default;

  stringid::StringId GetType() const noexcept override;
};
}  // namespace event
}  // namespace comet

#endif  // COMET_COMET_EVENT_WINDOW_EVENT_H_
