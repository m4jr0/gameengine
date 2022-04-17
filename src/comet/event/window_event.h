// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_EVENT_WINDOW_EVENT_H_
#define COMET_COMET_EVENT_WINDOW_EVENT_H_

#include "comet/event/event.h"
#include "comet_precompile.h"

namespace comet {
namespace event {
class WindowResizeEvent : public Event {
 public:
  static core::StringId kStaticType_;

  WindowResizeEvent(unsigned int, unsigned int);
  WindowResizeEvent(const WindowResizeEvent&) = default;
  WindowResizeEvent(WindowResizeEvent&&) noexcept = default;
  WindowResizeEvent& operator=(const WindowResizeEvent&) = default;
  WindowResizeEvent& operator=(WindowResizeEvent&&) noexcept = default;
  virtual ~WindowResizeEvent() = default;

  virtual const core::StringId& GetType() const noexcept override;
  unsigned int GetWidth() const noexcept;
  unsigned int GetHeight() const noexcept;

 private:
  unsigned int width_ = 0;
  unsigned int height_ = 0;
};

class WindowCloseEvent : public Event {
 public:
  static core::StringId kStaticType_;

  WindowCloseEvent() = default;
  WindowCloseEvent(const WindowCloseEvent&) = default;
  WindowCloseEvent(WindowCloseEvent&&) noexcept = default;
  WindowCloseEvent& operator=(const WindowCloseEvent&) = default;
  WindowCloseEvent& operator=(WindowCloseEvent&&) noexcept = default;
  virtual ~WindowCloseEvent() = default;

  virtual const core::StringId& GetType() const noexcept override;
};
}  // namespace event
}  // namespace comet

#endif  // COMET_COMET_EVENT_WINDOW_EVENT_H_
