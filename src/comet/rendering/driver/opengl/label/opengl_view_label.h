// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_OPENGL_LABEL_OPENGL_VIEW_LABEL_H_
#define COMET_COMET_RENDERING_DRIVER_OPENGL_LABEL_OPENGL_VIEW_LABEL_H_

#include "comet/core/essentials.h"
#include "comet/rendering/driver/opengl/type/opengl_view.h"

namespace comet {
namespace rendering {
namespace gl {
const schar* GetRenderTargetKindLabel(RenderTargetKind kind);
const schar* GetViewLoadOpLabel(ViewLoadOp op);
const schar* GetViewStoreOpLabel(ViewStoreOp op);
const schar* GetViewFinalColorOpLabel(ViewFinalColorOp op);
}  // namespace gl
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_OPENGL_LABEL_OPENGL_VIEW_LABEL_H_