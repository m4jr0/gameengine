// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "opengl_view.h"

namespace comet {
namespace rendering {
namespace gl {
View::View(const ViewDescr& descr) : id_{descr.id} {}

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
  is_initialized_ = false;
}

bool View::IsInitialized() const noexcept { return is_initialized_; }

RenderingViewId View::GetId() const noexcept { return id_; }
}  // namespace gl
}  // namespace rendering
}  // namespace comet