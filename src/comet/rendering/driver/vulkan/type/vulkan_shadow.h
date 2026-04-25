// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_VULKAN_TYPE_VULKAN_SHADOW_H_
#define COMET_COMET_RENDERING_DRIVER_VULKAN_TYPE_VULKAN_SHADOW_H_

// External. ///////////////////////////////////////////////////////////////////
#include "vulkan/vulkan.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/essentials.h"
#include "comet/math/matrix.h"
#include "comet/rendering/type/light.h"

namespace comet {
namespace rendering {
namespace vk {
constexpr u32 kMaxShadowViewProjCount{6};
constexpr u32 kMaxShadowCascades{4};

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

  VkExtent2D extent{};

  math::Mat4 view_proj[kMaxShadowCascades]{};
  f32 cascade_splits[kMaxShadowCascades]{};

  VkImageView layer_image_views[kMaxShadowCascades]{VK_NULL_HANDLE};
  VkFramebuffer framebuffers[kMaxShadowCascades]{VK_NULL_HANDLE};
};

struct ShadowRenderJob {
  LightHandle light_handle{};
  ShadowType type{ShadowType::None};

  u32 view_proj_index{0};
  math::Mat4 view_proj{1.0f};

  VkFramebuffer framebuffer{VK_NULL_HANDLE};
  VkExtent2D extent{};

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
}  // namespace vk
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_VULKAN_TYPE_VULKAN_SHADOW_H_