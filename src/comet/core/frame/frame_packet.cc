// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "frame_packet.h"

namespace comet {
namespace frame {
bool IsFrameStageStarted(const FramePacket& packet, FrameStage stage) {
  COMET_ASSERT(stage >= 0 && stage < kFrameStageCount,
               "Invalid frame stage: ", stage, "!");
  return packet.stage_times[stage].start > 0;
}

bool IsFrameStageFinished(const FramePacket& packet, FrameStage stage) {
  COMET_ASSERT(stage >= 0 && stage < kFrameStageCount,
               "Invalid frame stage: ", stage, "!");
  return packet.stage_times[stage].end > 0;
}
}  // namespace frame
}  // namespace comet
