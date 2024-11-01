// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_FRAME_PACKET_H_
#define COMET_COMET_CORE_FRAME_PACKET_H_

#include "comet/core/essentials.h"
#include "comet/time/time_manager.h"

namespace comet {
namespace frame {
using FrameCount = usize;

enum FrameStage { Unknown = -1, Game = 0, Render = 1, Gpu = 2, Flip = 3 };
using FrameStageTimestamp = usize;
constexpr auto kInvalidFrameStageTimestamp{
    static_cast<FrameStageTimestamp>(-1)};

struct StageTimes {
  FrameStageTimestamp start{kInvalidFrameStageTimestamp};
  FrameStageTimestamp end{kInvalidFrameStageTimestamp};
};

constexpr auto kFrameStageCount{4};

#ifdef COMET_DEBUG
using FramePacketDebugId = usize;
constexpr auto kInvalidFramePacketDebugId{static_cast<FramePacketDebugId>(-1)};
#endif  // COMET_DEBUG

struct FramePacket {
#ifdef COMET_DEBUG
 private:
  inline static FramePacketDebugId debug_id_counter_{0};

 public:
  FramePacketDebugId debug_id{FramePacket::debug_id_counter_++};
#endif  // COMET_DEBUG
  FrameCount frame_count{0};
  f64 lag{.0f};
  time::Interpolation interpolation{.0f};
  // TODO(m4jr0): Add skinning matrices.
  // TODO(m4jr0): Add list of meshes to render.
  StageTimes stage_times[kFrameStageCount]{};
};

bool IsFrameStageStarted(const FramePacket& packet, FrameStage stage);
bool IsFrameStageFinished(const FramePacket& packet, FrameStage stage);
}  // namespace frame
}  // namespace comet

#endif  // COMET_COMET_CORE_FRAME_PACKET_H_
