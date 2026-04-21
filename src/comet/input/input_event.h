// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_EVENT_INPUT_EVENT_H_
#define COMET_COMET_EVENT_INPUT_EVENT_H_

#include "comet/core/essentials.h"
#include "comet/core/type/string_id.h"
#include "comet/event/event.h"
#include "comet/input/input.h"
#include "comet/math/vector.h"

namespace comet {
namespace input {
class KeyboardEvent : public event::Event {
 public:
  const static stringid::StringId kStaticType_;

  KeyboardEvent(KeyCode key, ScanCode scan_code, Action action, Mods mods);
  KeyboardEvent(const KeyboardEvent&) = default;
  KeyboardEvent(KeyboardEvent&&) noexcept = default;
  KeyboardEvent& operator=(const KeyboardEvent&) = default;
  KeyboardEvent& operator=(KeyboardEvent&&) noexcept = default;
  ~KeyboardEvent() override = default;

  stringid::StringId GetType() const noexcept override;

  KeyCode GetKey() const noexcept;
  ScanCode GetScanCode() const noexcept;
  Action GetAction() const noexcept;
  Mods GetMods() const noexcept;

 private:
  KeyCode key_{KeyCode::Unknown};
  ScanCode scan_code_{0};
  Action action_{Action::Unknown};
  Mods mods_{Mods::Empty};
};

class MouseMoveEvent : public event::Event {
 public:
  const static stringid::StringId kStaticType_;

  explicit MouseMoveEvent(const math::Vec2& position);
  MouseMoveEvent(const MouseMoveEvent&) = default;
  MouseMoveEvent(MouseMoveEvent&&) noexcept = default;
  MouseMoveEvent& operator=(const MouseMoveEvent&) = default;
  MouseMoveEvent& operator=(MouseMoveEvent&&) noexcept = default;
  ~MouseMoveEvent() override = default;

  stringid::StringId GetType() const noexcept override;

  const math::Vec2& GetPosition() const noexcept;

 private:
  math::Vec2 position_{.0f, .0f};
};

class MouseScrollEvent : public event::Event {
 public:
  const static stringid::StringId kStaticType_;

  MouseScrollEvent(f64 x_offset, f64 y_offset);
  MouseScrollEvent(const MouseScrollEvent&) = default;
  MouseScrollEvent(MouseScrollEvent&&) noexcept = default;
  MouseScrollEvent& operator=(const MouseScrollEvent&) = default;
  MouseScrollEvent& operator=(MouseScrollEvent&&) noexcept = default;
  ~MouseScrollEvent() override = default;

  stringid::StringId GetType() const noexcept override;

  f64 GetXOffset() const noexcept;
  f64 GetYOffset() const noexcept;

 private:
  f64 x_offset_{0};
  f64 y_offset_{0};
};

class MouseClickEvent : public event::Event {
 public:
  const static stringid::StringId kStaticType_;

  MouseClickEvent(MouseButton button, Mods mods);
  MouseClickEvent(const MouseClickEvent&) = default;
  MouseClickEvent(MouseClickEvent&&) noexcept = default;
  MouseClickEvent& operator=(const MouseClickEvent&) = default;
  MouseClickEvent& operator=(MouseClickEvent&&) noexcept = default;
  ~MouseClickEvent() override = default;

  stringid::StringId GetType() const noexcept override;

  MouseButton GetButton() const noexcept;
  Mods GetMods() const noexcept;

 private:
  MouseButton button_{MouseButton::Unknown};
  Mods mods_{Mods::Empty};
};

class MouseReleaseEvent : public event::Event {
 public:
  const static stringid::StringId kStaticType_;

  MouseReleaseEvent(MouseButton button, Mods mods);
  MouseReleaseEvent(const MouseReleaseEvent&) = default;
  MouseReleaseEvent(MouseReleaseEvent&&) noexcept = default;
  MouseReleaseEvent& operator=(const MouseReleaseEvent&) = default;
  MouseReleaseEvent& operator=(MouseReleaseEvent&&) noexcept = default;
  ~MouseReleaseEvent() override = default;

  stringid::StringId GetType() const noexcept override;

  MouseButton GetButton() const noexcept;
  Mods GetMods() const noexcept;

 private:
  MouseButton button_{MouseButton::Unknown};
  Mods mods_{Mods::Empty};
};
}  // namespace input
}  // namespace comet

#endif  // COMET_COMET_EVENT_INPUT_EVENT_H_
