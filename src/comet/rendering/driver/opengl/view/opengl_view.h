// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_OPENGL_VIEW_OPENGL_VIEW_H_
#define COMET_COMET_RENDERING_DRIVER_OPENGL_VIEW_OPENGL_VIEW_H_

#include "comet_precompile.h"

#include "comet/math/matrix.h"
#include "comet/rendering/driver/opengl/data/opengl_frame.h"
#include "comet/rendering/rendering_common.h"
#include "comet/time/time_manager.h"

#ifdef COMET_DEBUG
#include "comet/rendering/debugger/debugger_displayer_manager.h"
#endif  // COMET_DEBUG

namespace comet {
namespace rendering {
namespace gl {
struct ViewPacket {
  FrameIndex frame_count{kInvalidFrameIndex};
  time::Interpolation interpolation{1};
  math::Mat4 projection_matrix{1.0f};
  const math::Mat4* view_matrix{nullptr};
};

struct ViewDescr {
  RenderingViewId id{kInvalidRenderingViewId};
};

class View {
 public:
  explicit View(const ViewDescr& descr);
  View(const View&) = delete;
  View(View&&) = delete;
  View& operator=(const View&) = delete;
  View& operator=(View&&) = delete;
  virtual ~View();

  virtual void Initialize();
  virtual void Destroy();
  virtual void Update(const ViewPacket& packet) = 0;

  bool IsInitialized() const noexcept;
  RenderingViewId GetId() const noexcept;

 protected:
  bool is_initialized_{false};
  RenderingViewId id_{kInvalidRenderingViewId};
};
}  // namespace gl
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_OPENGL_VIEW_OPENGL_VIEW_H_
