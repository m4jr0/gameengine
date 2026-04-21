// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "opengl_view.h"
////////////////////////////////////////////////////////////////////////////////

namespace comet {
namespace rendering {
namespace gl {
View::View(const ViewDescr& descr)
    : pass_descr_{descr.pass_descr},
      width_{descr.width},
      height_{descr.height},
      clear_color_{descr.clear_color[0], descr.clear_color[1],
                   descr.clear_color[2], descr.clear_color[3]},
      id_{descr.id},
      frame_state_{descr.frame_state} {
  COMET_ASSERT(frame_state_ != nullptr, "View::View", "frame state is null");
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
  width_ = 0;
  height_ = 0;
  clear_color_[0] = kColorBlackRgb[0];
  clear_color_[1] = kColorBlackRgb[1];
  clear_color_[2] = kColorBlackRgb[2];
  clear_color_[3] = 1.0f;
  pass_descr_ = {};
  is_initialized_ = false;
  frame_state_ = nullptr;
}

void View::SetSize(WindowSize width, WindowSize height) {
  if (width_ == width && height_ == height) {
    return;
  }

  width_ = width;
  height_ = height;
}

bool View::IsInitialized() const noexcept { return is_initialized_; }

RenderingViewId View::GetId() const noexcept { return id_; }

bool View::IsSwapchainTarget() const noexcept {
  return (pass_descr_.flags & kViewPassFlagBitsSwapchainTarget) != 0;
}

bool View::IsOffscreenTarget() const noexcept {
  return (pass_descr_.flags & kViewPassFlagBitsOffscreenTarget) != 0;
}

void View::OnInitialize() {}

void View::OnDestroy() {}
}  // namespace gl
}  // namespace rendering
}  // namespace comet