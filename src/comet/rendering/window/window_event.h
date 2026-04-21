// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_WINDOW_WINDOW_EVENT_H_
#define COMET_COMET_RENDERING_WINDOW_WINDOW_EVENT_H_

#include "comet/core/essentials.h"
#include "comet/core/type/string_id.h"
#include "comet/event/event.h"
#include "comet/rendering/type/rendering_common_type.h"

namespace comet {
namespace rendering {
class WindowInitializedEvent : public event::Event {
 public:
  const static stringid::StringId kStaticType_;

  WindowInitializedEvent(WindowSize width, WindowSize height);
  WindowInitializedEvent(const WindowInitializedEvent&) = default;
  WindowInitializedEvent(WindowInitializedEvent&&) noexcept = default;
  WindowInitializedEvent& operator=(const WindowInitializedEvent&) = default;
  WindowInitializedEvent& operator=(WindowInitializedEvent&&) noexcept =
      default;
  ~WindowInitializedEvent() override = default;

  stringid::StringId GetType() const noexcept override;

  WindowSize GetWidth() const noexcept;
  WindowSize GetHeight() const noexcept;

 private:
  WindowSize width_{0};
  WindowSize height_{0};
};

class WindowResizeEvent : public event::Event {
 public:
  const static stringid::StringId kStaticType_;

  WindowResizeEvent(WindowSize width, WindowSize height);
  WindowResizeEvent(const WindowResizeEvent&) = default;
  WindowResizeEvent(WindowResizeEvent&&) noexcept = default;
  WindowResizeEvent& operator=(const WindowResizeEvent&) = default;
  WindowResizeEvent& operator=(WindowResizeEvent&&) noexcept = default;
  ~WindowResizeEvent() override = default;

  stringid::StringId GetType() const noexcept override;

  WindowSize GetWidth() const noexcept;
  WindowSize GetHeight() const noexcept;

 private:
  WindowSize width_{0};
  WindowSize height_{0};
};

class WindowCloseEvent : public event::Event {
 public:
  const static stringid::StringId kStaticType_;

  WindowCloseEvent() = default;
  WindowCloseEvent(const WindowCloseEvent&) = default;
  WindowCloseEvent(WindowCloseEvent&&) noexcept = default;
  WindowCloseEvent& operator=(const WindowCloseEvent&) = default;
  WindowCloseEvent& operator=(WindowCloseEvent&&) noexcept = default;
  ~WindowCloseEvent() override = default;

  stringid::StringId GetType() const noexcept override;
};
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_WINDOW_WINDOW_EVENT_H_
