// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "opengl_view_label.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/debug_label.h"

namespace comet {
namespace rendering {
namespace gl {
const schar* GetRenderTargetKindLabel(RenderTargetKind kind) {
  switch (kind) {
    case RenderTargetKind::Swapchain:
      return "swapchain";
    case RenderTargetKind::Offscreen:
      return "offscreen";
    default:
      return kUnknownLabel;
  }
}

const schar* GetViewLoadOpLabel(ViewLoadOp op) {
  switch (op) {
    case ViewLoadOp::DontCare:
      return "dont_care";
    case ViewLoadOp::Load:
      return "load";
    case ViewLoadOp::Clear:
      return "clear";
    default:
      return kUnknownLabel;
  }
}

const schar* GetViewStoreOpLabel(ViewStoreOp op) {
  switch (op) {
    case ViewStoreOp::DontCare:
      return "dont_care";
    case ViewStoreOp::Store:
      return "store";
    default:
      return kUnknownLabel;
  }
}

const schar* GetViewFinalColorOpLabel(ViewFinalColorOp op) {
  switch (op) {
    case ViewFinalColorOp::Keep:
      return "keep";
    case ViewFinalColorOp::Present:
      return "present";
    default:
      return kUnknownLabel;
  }
}
}  // namespace gl
}  // namespace rendering
}  // namespace comet