// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RENDER_VIEW_H_
#define COMET_RENDER_VIEW_H_

#include "comet/core/essentials.h"
#include "comet/core/id/string_id.h"
#include "comet/core/id/string_id_allocator.h"
#include "comet/platform/window/window_common.h"
#include "comet/data/render/texture.h"
#include "comet/render/common.h"

namespace comet {
namespace render {
enum class RenderingViewType : u16 {
  Unknown = 0,
  World,
  Shadow,
  Skybox,
#ifdef COMET_DEBUG_VIEW
  Debug,
#endif  // COMET_DEBUG_VIEW
#ifdef COMET_IMGUI
  ImGui,
#endif  // COMET_IMGUI
};

enum class RenderingViewMatrixSource : u8 {
  Unknown = 0,
  SceneCamera,
  UiCamera
};

enum class ViewRenderStage : u8 {
  Unknown = 0,
  Offscreen,
  SceneBase,
  SceneOverlay,
  Overlay,
};

using WorldDebugFlags = u32;

enum WorldDebugFlagBits : WorldDebugFlags {  // >:3 Useless?
  kWorldDebugFlagBitsNone = 0x0,
  kWorldDebugFlagBitsUseTextures = 0x1,
  kWorldDebugFlagBitsUseLighting = 0x2,
  kWorldDebugFlagBitsUseShadows = 0x4,
  kWorldDebugFlagBitsShowNormals = 0x8,
  kWorldDebugFlagBitsAll = static_cast<WorldDebugFlags>(-1),
};

using RenderingViewId = stringid::StringId;
constexpr auto kInvalidRenderingViewId{static_cast<RenderingViewId>(-1)};

struct RenderingViewDescr {
  RenderingViewMatrixSource matrix_source{RenderingViewMatrixSource::Unknown};
  RenderingViewType type{RenderingViewType::Unknown};
  WindowSize width{0};
  WindowSize height{0};
  f32 clear_color[4]{kColorBlackRgb[0], kColorBlackRgb[1], kColorBlackRgb[2],
                     1.0f};
  RenderingViewId id{kInvalidRenderingViewId};
};
}  // namespace render
}  // namespace comet

#endif  // COMET_RENDER_VIEW_H_