// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_EVENT_INPUT_EVENT_H_
#define COMET_COMET_EVENT_INPUT_EVENT_H_

#include "comet_precompile.h"

#include "comet/event/event.h"
#include "glm/glm.hpp"

namespace comet {
namespace event {
class KeyboardEvent : public Event {
 public:
  const static stringid::StringId kStaticType_;

  KeyboardEvent(int key, int scan_code, int action, int mods);
  KeyboardEvent(const KeyboardEvent&) = default;
  KeyboardEvent(KeyboardEvent&&) noexcept = default;
  KeyboardEvent& operator=(const KeyboardEvent&) = default;
  KeyboardEvent& operator=(KeyboardEvent&&) noexcept = default;
  ~KeyboardEvent() = default;

  stringid::StringId GetType() const noexcept override;

  int GetKey() const noexcept;
  int GetScanCode() const noexcept;
  int GetAction() const noexcept;
  int GetMods() const noexcept;

 private:
  int key_{-1};
  int scan_code_{-1};
  int action_{-1};
  int mods_{-1};
};

class MouseMoveEvent : public Event {
 public:
  const static stringid::StringId kStaticType_;

  MouseMoveEvent(glm::vec2 position);
  MouseMoveEvent(const MouseMoveEvent&) = default;
  MouseMoveEvent(MouseMoveEvent&&) noexcept = default;
  MouseMoveEvent& operator=(const MouseMoveEvent&) = default;
  MouseMoveEvent& operator=(MouseMoveEvent&&) noexcept = default;
  ~MouseMoveEvent() = default;

  stringid::StringId GetType() const noexcept override;
  glm::vec2 GetPosition() const noexcept;

 private:
  glm::vec2 position_{glm::vec2(0.0f, 0.0f)};
};

class MouseScrollEvent : public Event {
 public:
  const static stringid::StringId kStaticType_;

  MouseScrollEvent(f64 x_offset, f64 y_offset);
  MouseScrollEvent(const MouseScrollEvent&) = default;
  MouseScrollEvent(MouseScrollEvent&&) noexcept = default;
  MouseScrollEvent& operator=(const MouseScrollEvent&) = default;
  MouseScrollEvent& operator=(MouseScrollEvent&&) noexcept = default;
  ~MouseScrollEvent() = default;

  stringid::StringId GetType() const noexcept override;
  f64 GetXOffset() const noexcept;
  f64 GetYOffset() const noexcept;

 private:
  f64 x_offset_{0};
  f64 y_offset_{0};
};

class MouseClickEvent : public Event {
 public:
  const static stringid::StringId kStaticType_;

  MouseClickEvent(bool is_left_button, bool is_right_button,
                  bool is_middle_button);
  MouseClickEvent(const MouseClickEvent&) = default;
  MouseClickEvent(MouseClickEvent&&) noexcept = default;
  MouseClickEvent& operator=(const MouseClickEvent&) = default;
  MouseClickEvent& operator=(MouseClickEvent&&) noexcept = default;
  ~MouseClickEvent() = default;

  stringid::StringId GetType() const noexcept override;
  bool IsLeftButton() const noexcept;
  bool IsRightButton() const noexcept;
  bool IsMiddleButton() const noexcept;

 private:
  bool is_left_button_{false};
  bool is_right_button_{false};
  bool is_middle_button_{false};
};

class MouseReleaseEvent : public Event {
 public:
  const static stringid::StringId kStaticType_;

  MouseReleaseEvent(bool is_left_button, bool is_right_button,
                    bool is_middle_button);
  MouseReleaseEvent(const MouseReleaseEvent&) = default;
  MouseReleaseEvent(MouseReleaseEvent&&) noexcept = default;
  MouseReleaseEvent& operator=(const MouseReleaseEvent&) = default;
  MouseReleaseEvent& operator=(MouseReleaseEvent&&) noexcept = default;
  ~MouseReleaseEvent() = default;

  stringid::StringId GetType() const noexcept override;
  bool IsLeftButton() const noexcept;
  bool IsRightButton() const noexcept;
  bool IsMiddleButton() const noexcept;

 private:
  bool is_left_button_{false};
  bool is_right_button_{false};
  bool is_middle_button_{false};
};
}  // namespace event
}  // namespace comet

#endif  // COMET_COMET_EVENT_INPUT_EVENT_H_
