// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_GAME_STATE_MANAGER_H_
#define COMET_COMET_CORE_GAME_STATE_MANAGER_H_

#include "comet/core/essentials.h"
#include "comet/core/manager.h"

namespace comet {
class GameStateManager : public Manager {
 public:
  static GameStateManager& Get();

  GameStateManager() = default;
  GameStateManager(const GameStateManager&) = delete;
  GameStateManager(GameStateManager&&) = delete;
  GameStateManager& operator=(const GameStateManager&) = delete;
  GameStateManager& operator=(GameStateManager&&) = delete;
  virtual ~GameStateManager() = default;

  void Pause();
  void Resume();
  void TogglePause();

  bool IsPaused() const noexcept;

 private:
  bool is_paused_{false};
};
}  // namespace comet

#endif  // COMET_COMET_CORE_GAME_STATE_MANAGER_H_
