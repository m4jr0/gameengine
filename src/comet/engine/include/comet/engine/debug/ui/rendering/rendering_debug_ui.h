// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_ENGINE_DEBUG_UI_RENDERING_RENDERING_DEBUG_UI_H_
#define COMET_ENGINE_DEBUG_UI_RENDERING_RENDERING_DEBUG_UI_H_

#include "comet/core/essentials.h"

#ifdef COMET_HAS_DEBUG_UI

#include "comet/debugging/rendering/rendering_debug_settings.h"
#include "comet/rendering/camera_manager.h"
#include "comet/rendering/rendering_handle.h"

namespace comet {
namespace debug {
class RenderingDebugUi {
 public:
  RenderingDebugUi() = default;
  RenderingDebugUi(const RenderingDebugUi&) = delete;
  RenderingDebugUi(RenderingDebugUi&&) = delete;
  RenderingDebugUi& operator=(const RenderingDebugUi&) = delete;
  RenderingDebugUi& operator=(RenderingDebugUi&&) = delete;
  ~RenderingDebugUi() = default;

  void Draw(RenderingDebugSettings& settings);

 private:
  static void DrawWorldFlags(const char* label,
                             RenderingDebugSettings& settings,
                             bool is_debug_camera);
  static void DrawDebugDrawFlags(const char* label,
                                 RenderingDebugSettings& settings,
                                 bool is_debug_camera);
  static void DrawCameraInfo(rendering::CameraManager& camera_manager,
                             rendering::CameraHandle handle);
};
}  // namespace debug
}  // namespace comet

#endif  // COMET_HAS_DEBUG_UI

#endif  // COMET_ENGINE_DEBUG_UI_RENDERING_RENDERING_DEBUG_UI_H_