// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RENDER_DRIVER_OPENGL_TYPE_OPENGL_SHADOW_H_
#define COMET_RENDER_DRIVER_OPENGL_TYPE_OPENGL_SHADOW_H_

// External. ///////////////////////////////////////////////////////////////////
#include "glad/glad.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/essentials.h"
#include "comet/core/math/matrix.h"
#include "comet/render/driver/opengl/type/opengl_framebuffer.h"
#include "comet/data/light/light.h"
#include "comet/runtime/light/light_handle.h"

namespace comet {
namespace render {
namespace gl {
constexpr u32 kMaxShadowViewProjCount{6};
constexpr u32 kMaxShadowCascades{4};
constexpr u32 kMaxShadowLayerCount{128};

struct ShadowResource {
  LightHandle light_handle{};
  bool is_dirty{false};

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
  LightHandle light_handle{};
  ShadowType type{ShadowType::None};

  u32 view_proj_index{0};
  math::Mat4 view_proj{1.0f};

  FrameBufferHandle framebuffer{kInvalidFrameBufferHandle};

  u32 resolution{0};

  f32 bias_constant{.0f};
  f32 bias_slope{.0f};

  const ShadowResource* resource{nullptr};

#ifdef COMET_DEBUG_RENDERING
  StaticArray<math::Vec3, 8> cascade_corners{};
#endif  // COMET_DEBUG_RENDERING
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

struct ShadowCullBatchRange {
  u32 indirect_offset{0};  // In GpuIndirectRenderProxy units.
  u32 proxy_id_offset{0};  // In RenderProxyId units.
  u32 batch_count{0};
  u32 instance_capacity{0};
#ifdef COMET_DEBUG_RENDERING
  u32 visible_count_debug{0};
#endif  // COMET_DEBUG_RENDERING
};
}  // namespace gl
}  // namespace render
}  // namespace comet

#endif  // COMET_RENDER_DRIVER_OPENGL_TYPE_OPENGL_SHADOW_H_