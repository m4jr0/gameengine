// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_OPENGL_DATA_OPENGL_SHADOW_H_
#define COMET_COMET_RENDERING_DRIVER_OPENGL_DATA_OPENGL_SHADOW_H_

// External. ///////////////////////////////////////////////////////////////////
#include "glad/glad.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/essentials.h"
#include "comet/math/matrix.h"
#include "comet/rendering/driver/opengl/data/opengl_framebuffer.h"
#include "comet/rendering/light/light_common.h"

namespace comet {
namespace rendering {
namespace gl {
constexpr u32 kMaxShadowViewProjCount{6};
constexpr u32 kMaxShadowCascades{4};

struct ShadowResource {
  bool is_alive{false};
  bool is_dirty{false};

  LightId light_id{kInvalidLightId};
  ShadowType type{ShadowType::None};

  u32 resolution{0};
  f32 max_distance{.0f};
  f32 bias_constant{.0f};
  f32 bias_slope{.0f};

  u32 view_proj_count{1};
  s32 gpu_shadow_index{-1};

  // First layer in the shared shadow array texture.
  s32 first_layer_index{-1};

  math::Mat4 view_proj[kMaxShadowCascades]{};
  f32 cascade_splits[kMaxShadowCascades]{};

  FrameBufferHandle framebuffers[kMaxShadowCascades]{kInvalidFrameBufferHandle};
};

struct ShadowRenderJob {
  LightId light_id{kInvalidLightId};
  ShadowType type{ShadowType::None};

  u32 view_proj_index{0};
  math::Mat4 view_proj{1.0f};

  FrameBufferHandle framebuffer{kInvalidFrameBufferHandle};

  u32 resolution{0};

  f32 bias_constant{.0f};
  f32 bias_slope{.0f};

  const ShadowResource* resource{nullptr};
};

struct GpuShadowSettings {
  math::Vec4 params0{};     // x = cascadeBlendRatio, y = pcfRadius, z =
                            // pcfSamples,
                            // w = cascadeCount.
  math::S32Vec4 params1{};  // x = debugSingleCascade, y = debugCascades, z =
                            // disableBlending, w = 0.
};

struct GpuShadowData {
  math::Mat4 view_proj{};
  math::Vec4
      cascade_data{};      // x splitNear, y splitFar, z layerIndex, w unused.
  math::Vec4 bias_data{};  // x biasConstant, y biasSlope, z/w unused.
};
}  // namespace gl
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_OPENGL_DATA_OPENGL_SHADOW_H_