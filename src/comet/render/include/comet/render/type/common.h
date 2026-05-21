// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RENDER_TYPE_COMMON_H_
#define COMET_RENDER_TYPE_COMMON_H_

#include "comet/core/essentials.h"

namespace comet {
namespace rendering {
constexpr auto kMaxAppNameLen{256};  // comet/platform/window/window_type.h >:3
constexpr auto kMaxWindowNameLen{
    256};  // comet/platform/window/window_type.h >:3

enum class ClipSpaceDepthRange : u8 { MinusOneToOne, ZeroToOne };

enum class DriverType : u8 { Unknown = 0, Empty, OpenGl, Vulkan, Direct3d12 };

using FrameCount = u32;
constexpr auto kInvalidFrameCount{static_cast<FrameCount>(-1)};

enum class AntiAliasingType : u16 {
  None = 0,
  Msaa,
  MsaaX2,
  MsaaX4,
  MsaaX8,
  MsaaX16,
  MsaaX32,
  MsaaX64
};

using WindowSize = u16;  // comet/platform/window/window_type.h >:3

struct WindowExtent {  // comet/platform/window/window_type.h >:3
  WindowSize width{0};
  WindowSize height{0};
};
}  // namespace rendering
}  // namespace comet

#endif  // COMET_RENDER_TYPE_COMMON_H_