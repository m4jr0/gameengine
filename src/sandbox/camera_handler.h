// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_EDITOR_CAMERA_HANDLER_H_
#define COMET_EDITOR_CAMERA_HANDLER_H_

#include "comet/core/essentials.h"
#include "comet/runtime/event/event.h"
#include "comet/runtime/camera/camera_handle.h"
#include "comet/runtime/event/event_manager.h"
#include "comet/core/math/vector.h"
#include "comet/render/render_handle.h"

namespace comet {
namespace sandbox {
enum class ControlledCamera {
  Game,
#ifdef COMET_DEBUG
  Debug,
#endif  // COMET_DEBUG
};

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

  void RegisterEvents();
  void UnregisterEvents();

  void ResetMousePosition();

  void SwitchControlledCamera();
  void StopMouseActions();

  camera::CameraHandle GetControlledCamera() const;

  static constexpr f32 kKeyboardMovementSensitivity_{.05f};
  static constexpr f32 kMouseOrbitSensitivity_{.002f};
  static constexpr f32 kMouseRotationSensitivity_{.0005f};
  static constexpr f32 kMousePanSensitivity_{.01f};
  static constexpr f32 kMouseZoomSensitivity_{.05f};

  bool is_initialized_{false};

  event::EventListenerId keyboard_listener_id_{event::kInvalidEventListenerId};
  event::EventListenerId mouse_move_listener_id_{
      event::kInvalidEventListenerId};
  event::EventListenerId mouse_scroll_listener_id_{
      event::kInvalidEventListenerId};
  event::EventListenerId mouse_click_listener_id_{
      event::kInvalidEventListenerId};
  event::EventListenerId mouse_release_listener_id_{
      event::kInvalidEventListenerId};

  bool is_orbiting_from_mouse_{false};
  bool is_rotating_from_mouse_{false};
  bool is_panning_from_mouse_{false};
  bool is_zooming_from_mouse_{false};

  ControlledCamera controlled_camera_{ControlledCamera::Game};

  math::Vec2 current_mouse_pos_{.0f, .0f};
  math::Vec2 last_mouse_pos_{.0f, .0f};

  camera::CameraHandle game_camera_{};
#ifdef COMET_DEBUG
  camera::CameraHandle debug_camera_{};
#endif  // COMET_DEBUG
};
}  // namespace sandbox
}  // namespace comet

#endif  // COMET_EDITOR_CAMERA_HANDLER_H_