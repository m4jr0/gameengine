// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_EVENT_INPUT_EVENT_H_
#define COMET_COMET_EVENT_INPUT_EVENT_H_

#include "comet_precompile.h"

#include "glm/glm.hpp"

#include "comet/event/event.h"

namespace comet {
namespace event {
class KeyboardEvent : public Event {
 public:
  const static core::StringId kStaticType_;

  KeyboardEvent(int, int, int, int);
  KeyboardEvent(const KeyboardEvent&) = default;
  KeyboardEvent(KeyboardEvent&&) noexcept = default;
  KeyboardEvent& operator=(const KeyboardEvent&) = default;
  KeyboardEvent& operator=(KeyboardEvent&&) noexcept = default;
  virtual ~KeyboardEvent() = default;

  virtual const core::StringId& GetType() const noexcept override;

  int GetKey() const noexcept;
  int GetScanCode() const noexcept;
  int GetAction() const noexcept;
  int GetMods() const noexcept;

 private:
  int key_ = -1;
  int scan_code_ = -1;
  int action_ = -1;
  int mods_ = -1;
};

class MouseMoveEvent : public Event {
 public:
  const static core::StringId kStaticType_;

  MouseMoveEvent(glm::vec2);
  MouseMoveEvent(const MouseMoveEvent&) = default;
  MouseMoveEvent(MouseMoveEvent&&) noexcept = default;
  MouseMoveEvent& operator=(const MouseMoveEvent&) = default;
  MouseMoveEvent& operator=(MouseMoveEvent&&) noexcept = default;
  virtual ~MouseMoveEvent() = default;

  virtual const core::StringId& GetType() const noexcept override;
  glm::vec2 GetPosition() const noexcept;

 private:
  glm::vec2 position_ = glm::vec2(0.0f, 0.0f);
};

class MouseScrollEvent : public Event {
 public:
  const static core::StringId kStaticType_;

  MouseScrollEvent(double, double);
  MouseScrollEvent(const MouseScrollEvent&) = default;
  MouseScrollEvent(MouseScrollEvent&&) noexcept = default;
  MouseScrollEvent& operator=(const MouseScrollEvent&) = default;
  MouseScrollEvent& operator=(MouseScrollEvent&&) noexcept = default;
  virtual ~MouseScrollEvent() = default;

  virtual const core::StringId& GetType() const noexcept override;
  double GetXOffset() const noexcept;
  double GetYOffset() const noexcept;

 private:
  double x_offset_ = 0;
  double y_offset_ = 0;
};

class MouseClickEvent : public Event {
 public:
  const static core::StringId kStaticType_;

  MouseClickEvent(bool, bool, bool);
  MouseClickEvent(const MouseClickEvent&) = default;
  MouseClickEvent(MouseClickEvent&&) noexcept = default;
  MouseClickEvent& operator=(const MouseClickEvent&) = default;
  MouseClickEvent& operator=(MouseClickEvent&&) noexcept = default;
  virtual ~MouseClickEvent() = default;

  virtual const core::StringId& GetType() const noexcept override;
  bool IsLeftButton() const noexcept;
  bool IsRightButton() const noexcept;
  bool IsMiddleButton() const noexcept;

 private:
  bool is_left_button_ = false;
  bool is_right_button_ = false;
  bool is_middle_button_ = false;
};

class MouseReleaseEvent : public Event {
 public:
  const static core::StringId kStaticType_;

  MouseReleaseEvent(bool, bool, bool);
  MouseReleaseEvent(const MouseReleaseEvent&) = default;
  MouseReleaseEvent(MouseReleaseEvent&&) noexcept = default;
  MouseReleaseEvent& operator=(const MouseReleaseEvent&) = default;
  MouseReleaseEvent& operator=(MouseReleaseEvent&&) noexcept = default;
  virtual ~MouseReleaseEvent() = default;

  virtual const core::StringId& GetType() const noexcept override;
  bool IsLeftButton() const noexcept;
  bool IsRightButton() const noexcept;
  bool IsMiddleButton() const noexcept;

 private:
  bool is_left_button_ = false;
  bool is_right_button_ = false;
  bool is_middle_button_ = false;
};
}  // namespace event
}  // namespace comet

#endif  // COMET_COMET_EVENT_INPUT_EVENT_H_
