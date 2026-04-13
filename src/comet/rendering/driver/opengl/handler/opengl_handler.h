// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_OPENGL_HANDLER_OPENGL_HANDLER_H_
#define COMET_COMET_RENDERING_DRIVER_OPENGL_HANDLER_OPENGL_HANDLER_H_

#include "comet/core/essentials.h"
#include "comet/rendering/driver/opengl/opengl_frame_state.h"

namespace comet {
namespace rendering {
namespace gl {
struct HandlerDescr {
  FrameState* frame_state{nullptr};
};

class Handler {
 public:
  Handler() = delete;
  explicit Handler(const HandlerDescr& descr);
  Handler(const Handler&) = delete;
  Handler(Handler&&) = delete;
  Handler& operator=(const Handler&) = delete;
  Handler& operator=(Handler&&) = delete;
  virtual ~Handler();

  virtual void Initialize();
  virtual void Shutdown();

  bool IsInitialized() const noexcept;

 protected:
  bool is_initialized_{false};
  FrameState* frame_state_{nullptr};
};
}  // namespace gl
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_OPENGL_HANDLER_OPENGL_HANDLER_H_
