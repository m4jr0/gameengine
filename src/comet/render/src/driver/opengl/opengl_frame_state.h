// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RENDER_DRIVER_OPENGL_OPENGL_FRAME_STATE_H_
#define COMET_RENDER_DRIVER_OPENGL_OPENGL_FRAME_STATE_H_

#include "comet/core/essentials.h"
#include "comet/render/driver/opengl/type/opengl_frame.h"

namespace comet {
namespace render {
namespace gl {
struct FrameStateDescr {
  FrameInFlightIndex max_frames_in_flight{2};
};

class FrameState {
 public:
  explicit FrameState(const FrameStateDescr& descr);
  FrameState(const FrameState&) = delete;
  FrameState(FrameState&&) = delete;
  FrameState& operator=(const FrameState&) = delete;
  FrameState& operator=(FrameState&&) = delete;
  ~FrameState() = default;

  void Initialize();
  void Destroy();

  FrameIndex GetFrameCount() const noexcept;
  FrameInFlightIndex GetFrameInFlightIndex() const noexcept;
  FrameInFlightIndex GetMaxFramesInFlight() const noexcept;

  void GoToNextFrame() noexcept;

 private:
  FrameIndex frame_count_{0};
  FrameInFlightIndex frame_in_flight_index_{0};
  FrameInFlightIndex max_frames_in_flight_{2};
};
}  // namespace gl
}  // namespace render
}  // namespace comet

#endif  // COMET_RENDER_DRIVER_OPENGL_OPENGL_FRAME_STATE_H_