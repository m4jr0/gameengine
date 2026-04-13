// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "debug_ui_manager.h"
////////////////////////////////////////////////////////////////////////////////

#ifdef COMET_HAS_DEBUG_UI

#include "comet/scene/environment/environment_manager.h"

#ifdef COMET_HAS_PROFILER_DEBUG_UI
#include "comet/core/game_state_manager.h"
#include "comet/profiler/profiler_manager.h"
#endif  // COMET_HAS_PROFILER_DEBUG_UI

namespace comet {
namespace debugui {
DebugUiManager& DebugUiManager::Get() {
  static DebugUiManager singleton{};
  return singleton;
}

void DebugUiManager::Initialize() {
  Manager::Initialize();

  auto& registry{rendering::DebugUiRegistry::Get()};

  environment_callback_id_ = registry.Register([this]() {
    environment_debug_ui_.Draw(scene::EnvironmentManager::Get());
  });

#ifdef COMET_HAS_PROFILER_DEBUG_UI
  debugger_callback_id_ = registry.Register([this]() {
    auto& profiler_manager{profiler::ProfilerManager::Get()};

    CpuProfilerGraph::Controls controls{};
    controls.is_recording = profiler_manager.IsRecording();
    controls.is_paused = GameStateManager::Get().IsPaused();
    controls.toggle_recording = [&profiler_manager]() {
      profiler_manager.ToggleRecording();
    };
    controls.toggle_pause = []() { GameStateManager::Get().TogglePause(); };

    debugger_debug_ui_.Draw(profiler_manager.GetData(), controls);
  });
#endif  // COMET_HAS_PROFILER_DEBUG_UI
}

void DebugUiManager::Shutdown() {
  auto& registry{rendering::DebugUiRegistry::Get()};

#ifdef COMET_HAS_PROFILER_DEBUG_UI
  registry.Unregister(debugger_callback_id_);
  debugger_callback_id_ = rendering::DebugUiRegistry::kInvalidCallbackId;
#endif  // COMET_HAS_PROFILER_DEBUG_UI

  registry.Unregister(environment_callback_id_);
  environment_callback_id_ = rendering::DebugUiRegistry::kInvalidCallbackId;

  Manager::Shutdown();
}
}  // namespace debugui
}  // namespace comet

#endif  // COMET_HAS_DEBUG_UI