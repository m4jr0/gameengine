// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/render/driver/opengl/opengl_frame_state.h"
////////////////////////////////////////////////////////////////////////////////

namespace comet {
namespace rendering {
namespace gl {
FrameState::FrameState(const FrameStateDescr& descr)
    : max_frames_in_flight_{descr.max_frames_in_flight} {}

void FrameState::Initialize() {
  COMET_ASSERT(max_frames_in_flight_ != 0, "FrameState::Initialize",
               "max frames in flight is zero");
  frame_count_ = 0;
  frame_in_flight_index_ = 0;
}

void FrameState::Destroy() {
  frame_count_ = kInvalidFrameIndex;
  frame_in_flight_index_ = kInvalidFrameInFlightIndex;
  max_frames_in_flight_ = 0;
}

FrameIndex FrameState::GetFrameCount() const noexcept { return frame_count_; }

FrameInFlightIndex FrameState::GetFrameInFlightIndex() const noexcept {
  return frame_in_flight_index_;
}

FrameInFlightIndex FrameState::GetMaxFramesInFlight() const noexcept {
  return max_frames_in_flight_;
}

void FrameState::GoToNextFrame() noexcept {
  ++frame_in_flight_index_;
  frame_in_flight_index_ %= max_frames_in_flight_;
  ++frame_count_;
}
}  // namespace gl
}  // namespace rendering
}  // namespace comet