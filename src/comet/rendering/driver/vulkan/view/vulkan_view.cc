// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "vulkan_view.h"
////////////////////////////////////////////////////////////////////////////////

namespace comet {
namespace rendering {
namespace vk {
View::View(const ViewDescr& descr)
    : pass_descr_{descr.pass_descr},
      width_{descr.width},
      height_{descr.height},
      clear_color_{descr.clear_color[0], descr.clear_color[1],
                   descr.clear_color[2], descr.clear_color[3]},
      id_{descr.id},
      context_{descr.context},
      render_pass_handler_{descr.render_pass_handler},
      render_stage_{descr.render_stage} {
  COMET_ASSERT(context_ != nullptr, "View::View", "context is null");
  COMET_ASSERT(render_pass_handler_ != nullptr, "View::View",
               "render pass handler is null");
  COMET_ASSERT(render_stage_ != ViewRenderStage::Unknown, "View::View",
               "render stage is unknown");
}

View::~View() {
  COMET_ASSERT(!is_initialized_, "View::~View", "view is still initialized");
}

void View::Initialize() {
  COMET_ASSERT(!is_initialized_, "View::Initialize",
               "view is already initialized");

  OnInitialize();
  is_initialized_ = true;
}

void View::Destroy() {
  COMET_ASSERT(is_initialized_, "View::Destroy", "view is not initialized");

  OnDestroy();

  id_ = kInvalidRenderingViewId;
  context_ = nullptr;
  width_ = 0;
  height_ = 0;
  clear_color_[0] = kColorBlackRgb[0];
  clear_color_[1] = kColorBlackRgb[1];
  clear_color_[2] = kColorBlackRgb[2];
  clear_color_[3] = 1.0f;

  if (render_pass_handle_ && render_pass_handler_->IsInitialized()) {
    render_pass_handler_->Destroy(render_pass_handle_);
    render_pass_handle_.Invalidate();
  }

  render_pass_handler_ = nullptr;
  is_initialized_ = false;
}

void View::Prepare(const ViewUpdate&) {}

void View::Begin(const ViewUpdate&) {}

void View::Draw(const ViewUpdate&) {}

void View::End(const ViewUpdate&) {}

void View::SetSize(WindowSize width, WindowSize height) {
  if (width_ == width && height_ == height) {
    return;
  }

  width_ = width;
  height_ = height;

  COMET_ASSERT(render_pass_handle_, "View::SetSize",
               "render pass handle is invalid", "width", width_, "height",
               height_);

  render_pass_handler_->SetSize(render_pass_handle_, static_cast<u32>(width_),
                                static_cast<u32>(height_));
}

CameraFlags View::GetCameraFlags() const noexcept { return camera_flags_; }

void View::SetCameraFlags(CameraFlags flags) noexcept { camera_flags_ = flags; }

bool View::SupportsCamera(CameraFlagBits bit) const noexcept {
  return (camera_flags_ & bit) != 0;
}

ViewRenderStage View::GetRenderStage() const noexcept { return render_stage_; }

void View::SetRenderStage(ViewRenderStage stage) noexcept {
  render_stage_ = stage;
}

RenderingViewId View::GetId() const noexcept { return id_; }

bool View::IsInitialized() const noexcept { return is_initialized_; }

bool View::IsSwapchainTarget() const noexcept {
  return (pass_descr_.flags & kViewPassFlagBitsSwapchainTarget) != 0;
}

bool View::IsOffscreenTarget() const noexcept {
  return (pass_descr_.flags & kViewPassFlagBitsOffscreenTarget) != 0;
}

bool View::IsOverlayTarget() const noexcept {
  return (pass_descr_.flags & kViewPassFlagBitsOverlayTarget) != 0;
}

void View::OnInitialize() {}

void View::OnDestroy() {}
}  // namespace vk
}  // namespace rendering
}  // namespace comet