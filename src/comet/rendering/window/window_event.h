// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_WINDOW_WINDOW_EVENT_H_
#define COMET_COMET_RENDERING_WINDOW_WINDOW_EVENT_H_

#include "comet/core/essentials.h"
#include "comet/core/type/string_id.h"
#include "comet/event/event.h"
#include "comet/rendering/rendering_common.h"

namespace comet {
namespace rendering {
class WindowInitializedEvent : public event::Event {
 public:
  const static stringid::StringId kStaticType_;

  WindowInitializedEvent(rendering::WindowSize width,
                         rendering::WindowSize height);
  WindowInitializedEvent(const WindowInitializedEvent&) = default;
  WindowInitializedEvent(WindowInitializedEvent&&) noexcept = default;
  WindowInitializedEvent& operator=(const WindowInitializedEvent&) = default;
  WindowInitializedEvent& operator=(WindowInitializedEvent&&) noexcept =
      default;
  virtual ~WindowInitializedEvent() = default;

  stringid::StringId GetType() const noexcept override;
  rendering::WindowSize GetWidth() const noexcept;
  rendering::WindowSize GetHeight() const noexcept;

 private:
  rendering::WindowSize width_{0};
  rendering::WindowSize height_{0};
};

class WindowResizeEvent : public event::Event {
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

class WindowCloseEvent : public event::Event {
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
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_WINDOW_WINDOW_EVENT_H_
