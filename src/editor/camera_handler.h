// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_EDITOR_CAMERA_HANDLER_H_
#define COMET_EDITOR_CAMERA_HANDLER_H_

#include "glm/glm.hpp"

#include "comet/event/event_manager.h"

// Some temporary code to move the camera in the editor.
namespace comet {
namespace editor {
struct CameraHandlerDescr {};

class CameraHandler {
 public:
  CameraHandler() = delete;
  explicit CameraHandler(const CameraHandlerDescr& descr);
  CameraHandler(const CameraHandler&) = delete;
  CameraHandler(CameraHandler&& other) = delete;
  CameraHandler& operator=(const CameraHandler&) = delete;
  CameraHandler& operator=(CameraHandler&& other) = delete;
  virtual ~CameraHandler();

  void Initialize();
  void Shutdown();
  void Update();

  bool IsInitialized() const noexcept;

 private:
  void OnEvent(const event::Event& event);
  void ResetMousePosition();

  static constexpr f32 kKeyboardMovementSensitivity_{0.05f};
  static constexpr f32 kMouseOrbitSensitivity_{0.002f};
  static constexpr f32 kMouseRotationSensitivity_{0.0005f};
  static constexpr f32 kMousePanSensitivity_{0.01f};
  static constexpr f32 kMouseZoomSensitivity_{0.05f};
  bool is_initialized_{false};
  bool is_orbiting_from_mouse_{false};
  bool is_rotating_from_mouse_{false};
  bool is_panning_from_mouse_{false};
  bool is_zooming_from_mouse_{false};
  glm::vec2 current_mouse_pos_{0.0f, 0.0f};
  glm::vec2 last_mouse_pos_{0.0f, 0.0f};
};
}  // namespace editor
}  // namespace comet

#endif  // COMET_EDITOR_CAMERA_HANDLER_H_
