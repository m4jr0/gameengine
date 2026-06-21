// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_ENGINE_DEBUG_UI_RENDER_RENDER_DEBUG_UI_H_
#define COMET_ENGINE_DEBUG_UI_RENDER_RENDER_DEBUG_UI_H_

#include "comet/core/essentials.h"

#ifdef COMET_HAS_DEBUG_UI

#include "comet/render/debug/rendering_debug_settings.h"
#include "comet/runtime/camera/camera_manager.h"
#include "comet/runtime/camera/camera_handle.h"

namespace comet {
namespace debug {
class RenderDebugUi {
 public:
  RenderDebugUi() = default;
  RenderDebugUi(const RenderDebugUi&) = delete;
  RenderDebugUi(RenderDebugUi&&) = delete;
  RenderDebugUi& operator=(const RenderDebugUi&) = delete;
  RenderDebugUi& operator=(RenderDebugUi&&) = delete;
  ~RenderDebugUi() = default;

  void Draw(RenderingDebugSettings& settings);

 private:
  static void DrawWorldFlags(const char* label,
                             RenderingDebugSettings& settings,
                             bool is_debug_camera);
  static void DrawDebugDrawFlags(const char* label,
                                 RenderingDebugSettings& settings,
                                 bool is_debug_camera);
  static void DrawCameraInfo(camera::CameraManager& camera_manager,
                             camera::CameraHandle handle);
};
}  // namespace debug
}  // namespace comet

#endif  // COMET_HAS_DEBUG_UI

#endif  // COMET_ENGINE_DEBUG_UI_RENDER_RENDER_DEBUG_UI_H_