// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_ENGINE_DEBUG_UI_DEBUG_UI_MANAGER_H_
#define COMET_ENGINE_DEBUG_UI_DEBUG_UI_MANAGER_H_

#include "comet/core/essentials.h"

#ifdef COMET_HAS_DEBUG_UI

#include "comet/runtime/manager.h"
#include "comet/engine/debug/ui/environment/environment_debug_ui.h"
#include "comet/engine/debug/ui/render/render_debug_ui.h"
#include "comet/render/debug_ui_registry.h"

#ifdef COMET_HAS_PROFILER_DEBUG_UI
#include "comet/engine/debug/ui/debugger_debug_ui.h"
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

  render::DebugUiRegistry::CallbackId environment_callback_id_{
      render::DebugUiRegistry::kInvalidCallbackId};
  render::DebugUiRegistry::CallbackId camera_callback_id_{
      render::DebugUiRegistry::kInvalidCallbackId};

#ifdef COMET_HAS_PROFILER_DEBUG_UI
  render::DebugUiRegistry::CallbackId debugger_callback_id_{
      render::DebugUiRegistry::kInvalidCallbackId};
  DebuggerDebugUi debugger_debug_ui_{};
#endif  // COMET_HAS_PROFILER_DEBUG_UI

  EnvironmentDebugUi environment_debug_ui_{};
  RenderDebugUi render_debug_ui_{};
};
}  // namespace debug
}  // namespace comet

#endif  // COMET_HAS_DEBUG_UI

#endif  // COMET_ENGINE_DEBUG_UI_DEBUG_UI_MANAGER_H_