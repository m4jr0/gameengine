// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_OPENGL_VIEW_OPENGL_VIEW_H_
#define COMET_COMET_RENDERING_DRIVER_OPENGL_VIEW_OPENGL_VIEW_H_

#include "comet/core/essentials.h"
#include "comet/core/frame/frame_packet.h"
#include "comet/rendering/driver/opengl/opengl_frame_state.h"
#include "comet/rendering/rendering_common.h"

namespace comet {
namespace rendering {
namespace gl {
enum class RenderTargetKind : u8 { Swapchain, Offscreen };

enum class ViewLoadOp : u8 { DontCare, Load, Clear };

enum class ViewStoreOp : u8 { DontCare, Store };

enum class ViewFinalColorOp : u8 { Keep, Present };

using ViewPassFlags = u32;

enum ViewPassFlagBits : ViewPassFlags {
  kViewPassFlagBitsNone = 0x0,
  kViewPassFlagBitsSwapchainTarget = 0x1,
  kViewPassFlagBitsOffscreenTarget = 0x2,
  kViewPassFlagBitsHasColor = 0x4,
  kViewPassFlagBitsHasDepth = 0x8
};

struct ViewPassDescr {
  RenderTargetKind target_kind{RenderTargetKind::Swapchain};
  ViewPassFlags flags{kViewPassFlagBitsNone};

  ViewLoadOp color_load_op{ViewLoadOp::DontCare};
  ViewStoreOp color_store_op{ViewStoreOp::Store};

  ViewLoadOp depth_load_op{ViewLoadOp::DontCare};
  ViewStoreOp depth_store_op{ViewStoreOp::DontCare};

  ViewFinalColorOp final_color_op{ViewFinalColorOp::Keep};
};

struct ViewDescr {
  WindowSize width{0};
  WindowSize height{0};
  f32 clear_color[4]{kColorBlackRgb[0], kColorBlackRgb[1], kColorBlackRgb[2],
                     1.0f};
  RenderingViewId id{kInvalidRenderingViewId};
  ViewPassDescr pass_descr{};
  const FrameState* frame_state{nullptr};
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
  virtual void Update(frame::FramePacket*) = 0;
  virtual void SetSize(WindowSize width, WindowSize height);

  bool IsInitialized() const noexcept;
  RenderingViewId GetId() const noexcept;
  bool IsSwapchainTarget() const noexcept;
  bool IsOffscreenTarget() const noexcept;

 protected:
  bool is_initialized_{false};
  ViewPassDescr pass_descr_{};
  WindowSize width_{0};
  WindowSize height_{0};
  f32 clear_color_[4]{kColorBlackRgb[0], kColorBlackRgb[1], kColorBlackRgb[2],
                      1.0f};
  RenderingViewId id_{kInvalidRenderingViewId};
  const FrameState* frame_state_{nullptr};
};
}  // namespace gl
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_OPENGL_VIEW_OPENGL_VIEW_H_