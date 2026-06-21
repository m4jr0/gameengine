// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RENDER_DRIVER_VULKAN_LABEL_VULKAN_VIEW_LABEL_H_
#define COMET_RENDER_DRIVER_VULKAN_LABEL_VULKAN_VIEW_LABEL_H_

#include "comet/core/essentials.h"
#include "comet/render/driver/vulkan/type/vulkan_view.h"

namespace comet {
namespace rendering {
namespace vk {
const schar* GetRenderTargetKindLabel(RenderTargetKind kind);
const schar* GetViewLoadOpLabel(ViewLoadOp op);
const schar* GetViewStoreOpLabel(ViewStoreOp op);
const schar* GetViewFinalColorOpLabel(ViewFinalColorOp op);
}  // namespace vk
}  // namespace rendering
}  // namespace comet

#endif  // COMET_RENDER_DRIVER_VULKAN_LABEL_VULKAN_VIEW_LABEL_H_