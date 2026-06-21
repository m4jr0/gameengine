// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RUNTIME_FRAME_LOGIC_FRAME_MANAGER_H_
#define COMET_RUNTIME_FRAME_LOGIC_FRAME_MANAGER_H_

#include "comet/core/essentials.h"
#include "comet/runtime/frame/frame_packet.h"
#include "comet/runtime/manager.h"

namespace comet {
class EngineFrameManager : public Manager {
 public:
  static EngineFrameManager& Get();

  EngineFrameManager(const EngineFrameManager&) = delete;
  EngineFrameManager(EngineFrameManager&&) = delete;
  EngineFrameManager& operator=(const EngineFrameManager&) = delete;
  EngineFrameManager& operator=(EngineFrameManager&&) = delete;
  ~EngineFrameManager() = default;

  void RunFrame(f64& lag, EngineClient& client);

 private:
  EngineFrameManager() = default;

  void BeginFrame(f64& lag, EngineClient& client);

  void PrepareFramePackets(frame::FramePacket* logic_packet,
                           frame::FramePacket* render_packet, f64 lag,
                           job::Counter* counter);
  void PrepareLogicPacket(frame::FramePacket* packet);

  void KickLogicUpdate(frame::FramePacket* packet);
  void WaitFrameJobs(job::CounterGuard& guard, frame::FramePacket* logic_packet,
                     frame::FramePacket* render_packet, f64& lag);
  void EndFrame(frame::FrameManager& frame_manager);
};
}  // namespace comet

#endif  // COMET_RUNTIME_FRAME_LOGIC_FRAME_MANAGER_H_
