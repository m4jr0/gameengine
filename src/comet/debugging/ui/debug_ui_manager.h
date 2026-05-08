// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_DEBUGGING_UI_DEBUG_UI_MANAGER_H_
#define COMET_COMET_DEBUGGING_UI_DEBUG_UI_MANAGER_H_

#include "comet/core/essentials.h"

#ifdef COMET_HAS_DEBUG_UI

#include "comet/core/manager.h"
#include "comet/debugging/ui/environment/environment_debug_ui.h"
#include "comet/debugging/ui/rendering/rendering_debug_ui.h"
#include "comet/rendering/debug_ui_registry.h"

#ifdef COMET_HAS_PROFILER_DEBUG_UI
#include "comet/debugging/ui/debugger/debugger_debug_ui.h"
#endif  // COMET_HAS_PROFILER_DEBUG_UI

namespace comet {
namespace debug {
class DebugUiManager : public Manager {
 public:
  static DebugUiManager& Get();

  DebugUiManager() = default;
  DebugUiManager(const DebugUiManager&) = delete;
  DebugUiManager(DebugUiManager&&) = delete;
  DebugUiManager& operator=(const DebugUiManager&) = delete;
  DebugUiManager& operator=(DebugUiManager&&) = delete;
  ~DebugUiManager() override = default;

 protected:
  void OnInitialize() override;
  void OnShutdown() override;

 private:
  void TogglePause();

  bool is_paused_{false};
  f32 saved_time_scale_{1.0f};

  rendering::DebugUiRegistry::CallbackId environment_callback_id_{
      rendering::DebugUiRegistry::kInvalidCallbackId};
  rendering::DebugUiRegistry::CallbackId camera_callback_id_{
      rendering::DebugUiRegistry::kInvalidCallbackId};

#ifdef COMET_HAS_PROFILER_DEBUG_UI
  rendering::DebugUiRegistry::CallbackId debugger_callback_id_{
      rendering::DebugUiRegistry::kInvalidCallbackId};
  DebuggerDebugUi debugger_debug_ui_{};
#endif  // COMET_HAS_PROFILER_DEBUG_UI

  EnvironmentDebugUi environment_debug_ui_{};
  RenderingDebugUi rendering_debug_ui_{};
};
}  // namespace debug
}  // namespace comet

#endif  // COMET_HAS_DEBUG_UI

#endif  // COMET_COMET_DEBUGGING_UI_DEBUG_UI_MANAGER_H_