// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_EVENT_INPUT_LABEL_H_
#define COMET_COMET_EVENT_INPUT_LABEL_H_

#include "comet/core/essentials.h"
#include "comet/input/input.h"

namespace comet {
namespace input {
const schar* GetMouseCursorModeLabel(MouseCursorMode mode);
const schar* GetMouseButtonLabel(MouseButton button);
const schar* GetActionLabel(Action action);
const schar* GetKeyCodeLabel(KeyCode key);
}  // namespace input
}  // namespace comet

#endif  // COMET_COMET_EVENT_INPUT_LABEL_H_
