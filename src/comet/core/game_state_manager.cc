// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "game_state_manager.h"
////////////////////////////////////////////////////////////////////////////////

namespace comet {
GameStateManager& GameStateManager::Get() {
  static GameStateManager singleton{};
  return singleton;
}

void GameStateManager::Pause() { is_paused_ = true; }

void GameStateManager::Resume() { is_paused_ = false; }

void GameStateManager::TogglePause() { is_paused_ = !is_paused_; }

bool GameStateManager::IsPaused() const noexcept { return is_paused_; }
}  // namespace comet
