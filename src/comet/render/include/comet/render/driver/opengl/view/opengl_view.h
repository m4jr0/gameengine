// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RENDER_DRIVER_OPENGL_VIEW_OPENGL_VIEW_H_
#define COMET_RENDER_DRIVER_OPENGL_VIEW_OPENGL_VIEW_H_

#include "comet/core/essentials.h"
#include "comet/runtime/frame/frame_packet.h"
#include "comet/render/driver/opengl/opengl_frame_state.h"
#include "comet/platform/window/window_common.h"
#include "comet/render/driver/opengl/type/opengl_view.h"
#include "comet/runtime/camera/camera.h"
#include "comet/render/common.h"
#include "comet/data/render/texture.h"
#include "comet/render/view.h"

namespace comet {
namespace render {
namespace gl {
struct ViewUpdate {
  frame::FramePacket* packet{nullptr};
  const CameraViewData* camera_data{nullptr};
  CameraKind camera_kind{CameraKind::Game};
  CameraFlags camera_flags{kCameraFlagBitsNone};
  usize camera_index{kInvalidIndex};
  bool is_debug_draw_enabled{false};
  ViewportRect viewport{};
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
  ViewRenderStage render_stage{ViewRenderStage::Unknown};
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

  void Initialize();
  void Destroy();

  virtual void Prepare(const ViewUpdate&);
  virtual void Begin(const ViewUpdate&);
  virtual void Draw(const ViewUpdate&);
  virtual void End(const ViewUpdate&);

  virtual void SetSize(WindowSize width, WindowSize height);

  CameraFlags GetCameraFlags() const noexcept;
  void SetCameraFlags(CameraFlags flags) noexcept;
  bool SupportsCamera(CameraFlagBits bit) const noexcept;

  ViewRenderStage GetRenderStage() const noexcept;
  void SetRenderStage(ViewRenderStage stage) noexcept;

  RenderingViewId GetId() const noexcept;

  bool IsInitialized() const noexcept;
  bool IsSwapchainTarget() const noexcept;
  bool IsOffscreenTarget() const noexcept;
  bool IsOverlayTarget() const noexcept;

 protected:
  virtual void OnInitialize();
  virtual void OnDestroy();

  bool is_initialized_{false};
  ViewPassDescr pass_descr_{};

  CameraFlags camera_flags_{kCameraFlagBitsNone};

  WindowSize width_{0};
  WindowSize height_{0};

  f32 clear_color_[4]{kColorBlackRgb[0], kColorBlackRgb[1], kColorBlackRgb[2],
                      1.0f};

  RenderingViewId id_{kInvalidRenderingViewId};

  const FrameState* frame_state_{nullptr};

 private:
  ViewRenderStage render_stage_{ViewRenderStage::SceneOverlay};
};
}  // namespace gl
}  // namespace render
}  // namespace comet

#endif  // COMET_RENDER_DRIVER_OPENGL_VIEW_OPENGL_VIEW_H_