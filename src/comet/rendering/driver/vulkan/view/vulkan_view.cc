// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "vulkan_view.h"

namespace comet {
namespace rendering {
namespace vk {
View::View(const ViewDescr& descr)
    : id_{descr.id},
      is_first_{descr.is_first},
      is_last_{descr.is_last},
      width_{descr.width},
      height_{descr.height},
      clear_color_{descr.clear_color[0], descr.clear_color[1],
                   descr.clear_color[2], descr.clear_color[3]},
      context_{descr.context},
      render_pass_handler_{descr.render_pass_handler} {
  COMET_ASSERT(context_ != nullptr, "Context is null!");
  COMET_ASSERT(render_pass_handler_ != nullptr, "Render pass handler is null!");
}

View::~View() {
  COMET_ASSERT(!is_initialized_,
               "Destructor called for view, but it is still initialized!");
}

void View::Initialize() {
  COMET_ASSERT(!is_initialized_,
               "Tried to initialize view, but it is already done!");
  is_initialized_ = true;
}

void View::Destroy() {
  id_ = kInvalidRenderingViewId;
  context_ = nullptr;

  width_ = 0;
  height_ = 0;
  clear_color_[0] = kColorBlack[0];
  clear_color_[1] = kColorBlack[1];
  clear_color_[2] = kColorBlack[2];
  clear_color_[3] = 1.0f;

  if (render_pass_ != nullptr && render_pass_handler_->IsInitialized()) {
    render_pass_handler_->Destroy(*render_pass_);
    render_pass_ = nullptr;
  }

  render_pass_handler_ = nullptr;
  is_initialized_ = false;
}

void View::SetSize(WindowSize width, WindowSize height) {
  COMET_ASSERT(render_pass_ != nullptr, "Tried to set size to ", width, "x",
               height, ", but render pass is null!");
  render_pass_->extent.width = static_cast<u32>(width);
  render_pass_->extent.height = static_cast<u32>(height);
  render_pass_handler_->Refresh(*render_pass_);
}

bool View::IsInitialized() const noexcept { return is_initialized_; }

RenderingViewId View::GetId() const noexcept { return id_; }
}  // namespace vk
}  // namespace rendering
}  // namespace comet