// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_TYPE_RENDERING_VIEW_TYPE_H_
#define COMET_COMET_RENDERING_TYPE_RENDERING_VIEW_TYPE_H_

#include "comet/core/essentials.h"
#include "comet/core/type/string_id.h"
#include "comet/rendering/type/rendering_common_type.h"
#include "comet/rendering/type/rendering_texture_type.h"

namespace comet {
namespace rendering {
enum class RenderingViewType : u16 {
  Unknown = 0,
  World,
  Shadow,
  Skybox,
  Debug,
  ImGui
};

enum class RenderingViewMatrixSource : u8 {
  Unknown = 0,
  SceneCamera,
  UiCamera
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
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_TYPE_RENDERING_VIEW_TYPE_H_