// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_LOGIC_GAME_LOGIC_MANAGER_H_
#define COMET_COMET_CORE_LOGIC_GAME_LOGIC_MANAGER_H_

#include "comet/core/essentials.h"
#include "comet/core/frame/frame_packet.h"
#include "comet/core/manager.h"

namespace comet {
class GameLogicManager : public Manager {
 public:
  static GameLogicManager& Get();

  GameLogicManager() = default;
  GameLogicManager(const GameLogicManager&) = delete;
  GameLogicManager(GameLogicManager&&) = delete;
  GameLogicManager& operator=(const GameLogicManager&) = delete;
  GameLogicManager& operator=(GameLogicManager&&) = delete;
  ~GameLogicManager() override = default;

  void Update(frame::FramePacket* packet);

 private:
  void PopulatePacket(frame::FramePacket* packet);
};
}  // namespace comet

#endif  // COMET_COMET_CORE_LOGIC_GAME_LOGIC_MANAGER_H_
