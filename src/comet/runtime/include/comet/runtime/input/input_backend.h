// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RUNTIME_INPUT_INPUT_BACKEND_H_
#define COMET_RUNTIME_INPUT_INPUT_BACKEND_H_

#include "comet/core/essentials.h"
#include "comet/core/math/vector.h"
#include "comet/data/input/input.h"

namespace comet {
namespace input {
struct InputSnapshot {
  bool keys[internal::kKeyCount]{};
  bool mouse_buttons[internal::kMouseButtonCount]{};
  math::Vec2 mouse_position{};
  math::Vec2 scroll_delta{};
};

struct InputUserUpdate {
  bool has_mouse_position{false};
  math::Vec2 mouse_position{};
  MouseCursorMode cursor_mode{MouseCursorMode::Unknown};
};

class InputBackend {
 public:
  InputBackend() = default;
  InputBackend(const InputBackend&) = delete;
  InputBackend(InputBackend&&) = delete;
  InputBackend& operator=(const InputBackend&) = delete;
  InputBackend& operator=(InputBackend&&) = delete;
  virtual ~InputBackend() = default;

  virtual void Poll(InputSnapshot& snapshot) = 0;
  virtual void ApplyUserUpdate(const InputUserUpdate& update) = 0;
};
}  // namespace input
}  // namespace comet

#endif  // COMET_RUNTIME_INPUT_INPUT_BACKEND_H_