// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RENDER_DRIVER_VULKAN_TYPE_VULKAN_VIEW_H_
#define COMET_RENDER_DRIVER_VULKAN_TYPE_VULKAN_VIEW_H_

#include "comet/core/essentials.h"

namespace comet {
namespace rendering {
namespace vk {
enum class RenderTargetKind : u8 { Swapchain, Offscreen };

enum class ViewLoadOp : u8 { DontCare, Load, Clear };

enum class ViewStoreOp : u8 { DontCare, Store };

enum class ViewFinalColorOp : u8 { Keep, Present };

using ViewPassFlags = u32;

enum ViewPassFlagBits : ViewPassFlags {
  kViewPassFlagBitsNone = 0x0,
  kViewPassFlagBitsSwapchainTarget = 0x1,
  kViewPassFlagBitsOffscreenTarget = 0x2,
  kViewPassFlagBitsOverlayTarget = 0x4,
  kViewPassFlagBitsHasColor = 0x8,
  kViewPassFlagBitsHasDepth = 0x10
};
}  // namespace vk
}  // namespace rendering
}  // namespace comet

#endif  // COMET_RENDER_DRIVER_VULKAN_TYPE_VULKAN_VIEW_H_