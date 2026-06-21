// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_engine_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/engine/debug/ui/debug_ui_manager.h"
////////////////////////////////////////////////////////////////////////////////

#ifdef COMET_HAS_DEBUG_UI

#include "comet/render/debug/rendering_debug_settings.h"
#include "comet/runtime/environment/environment_manager.h"

#ifdef COMET_HAS_PROFILER_DEBUG_UI
#include "comet/runtime/profiler/profiler_manager.h"
#endif  // COMET_HAS_PROFILER_DEBUG_UI

namespace comet {
namespace debug {
DebugUiManager& DebugUiManager::Get() {
  static DebugUiManager singleton{};
  return singleton;
}

void DebugUiManager::OnInitialize() {
  auto& registry{render::DebugUiRegistry::Get()};

  environment_callback_id_ = registry.Register([this]() {
    environment_debug_ui_.Draw(environment::EnvironmentManager::Get());
  });

#ifdef COMET_HAS_PROFILER_DEBUG_UI
  debugger_callback_id_ = registry.Register([this]() {
    auto& profiler_manager{profiler::ProfilerManager::Get()};

    CpuProfilerGraph::Controls controls{};
    controls.is_recording = profiler_manager.IsRecording();
    controls.is_paused = is_paused_;
    controls.toggle_recording = [&profiler_manager]() {
      profiler_manager.ToggleRecording();
    };
    controls.toggle_pause = [this]() { TogglePause(); };

    debugger_debug_ui_.Draw(profiler_manager.GetData(), controls);
  });
#endif  // COMET_HAS_PROFILER_DEBUG_UI

  camera_callback_id_ = registry.Register(
      [this]() { render_debug_ui_.Draw(RenderingDebugSettings::Get()); });
}

void DebugUiManager::OnShutdown() {
  auto& registry{render::DebugUiRegistry::Get()};

#ifdef COMET_HAS_PROFILER_DEBUG_UI
  registry.Unregister(debugger_callback_id_);
  debugger_callback_id_ = render::DebugUiRegistry::kInvalidCallbackId;
#endif  // COMET_HAS_PROFILER_DEBUG_UI

  registry.Unregister(environment_callback_id_);
  environment_callback_id_ = render::DebugUiRegistry::kInvalidCallbackId;

  registry.Unregister(camera_callback_id_);
  camera_callback_id_ = render::DebugUiRegistry::kInvalidCallbackId;
}

void DebugUiManager::TogglePause() {
  is_paused_ = !is_paused_;
  auto& time_manager{time::TimeManager::Get()};

  if (is_paused_) {
    saved_time_scale_ = time_manager.GetTimeScale();
    time_manager.SetTimeScale(.0f);
  } else {
    time_manager.SetTimeScale(saved_time_scale_);
    saved_time_scale_ = 1.0f;
  }
}
}  // namespace debug
}  // namespace comet

#endif  // COMET_HAS_DEBUG_UI