// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_EDITOR_CAMERA_HANDLER_H_
#define COMET_EDITOR_CAMERA_HANDLER_H_

#include "comet/core/essentials.h"
#include "comet/event/event.h"
#include "comet/math/vector.h"
#include "comet/rendering/rendering_handle.h"

namespace comet {
namespace editor {
class CameraHandler {
 public:
  CameraHandler() = default;
  CameraHandler(const CameraHandler&) = delete;
  CameraHandler(CameraHandler&&) = delete;
  CameraHandler& operator=(const CameraHandler&) = delete;
  CameraHandler& operator=(CameraHandler&&) = delete;
  ~CameraHandler();

  void Initialize();
  void Shutdown();

  void Update();

  bool IsInitialized() const noexcept;

 private:
  void OnEvent(const event::Event& event);
  void ResetMousePosition();

  static constexpr f32 kKeyboardMovementSensitivity_{.05f};
  static constexpr f32 kMouseOrbitSensitivity_{.002f};
  static constexpr f32 kMouseRotationSensitivity_{.0005f};
  static constexpr f32 kMousePanSensitivity_{.01f};
  static constexpr f32 kMouseZoomSensitivity_{.05f};

  bool is_initialized_{false};
  bool is_orbiting_from_mouse_{false};
  bool is_rotating_from_mouse_{false};
  bool is_panning_from_mouse_{false};
  bool is_zooming_from_mouse_{false};

  math::Vec2 current_mouse_pos_{.0f, .0f};
  math::Vec2 last_mouse_pos_{.0f, .0f};

  rendering::CameraHandle camera_{};
};
}  // namespace editor
}  // namespace comet

#endif  // COMET_EDITOR_CAMERA_HANDLER_H_