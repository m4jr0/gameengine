// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_VULKAN_DATA_VULKAN_RENDER_PROXY_H_
#define COMET_COMET_RENDERING_DRIVER_VULKAN_DATA_VULKAN_RENDER_PROXY_H_

#include "comet/core/essentials.h"
#include "comet/math/matrix.h"
#include "comet/rendering/driver/vulkan/data/vulkan_material.h"
#include "comet/rendering/driver/vulkan/data/vulkan_mesh.h"
#include "comet/rendering/driver/vulkan/data/vulkan_shader.h"

namespace comet {
namespace rendering {
namespace vk {
using RenderProxyId = u32;
constexpr auto kInvalidRenderProxyId{static_cast<RenderProxyId>(-1)};

using BatchId = u32;
constexpr auto kInvalidBatchId{static_cast<BatchId>(-1)};

struct RenderProxy {
  RenderProxyId id{kInvalidRenderProxyId};
  MeshProxyHandle mesh_handle{kInvalidMeshProxyHandle};
  MaterialId mat_id{kInvalidMaterialId};
};

struct RenderBatchEntry {
  using SortKey = u64;
  static inline constexpr auto kInvalidSortKey{static_cast<SortKey>(-1)};

  SortKey sort_key{kInvalidSortKey};
  const RenderProxy* proxy{nullptr};
};

struct RenderIndirectBatch {
  u32 offset{0};
  u32 count{0};
  const RenderProxy* proxy{nullptr};
};

struct RenderBatchGroup {
  u32 offset{0};
  u32 count{0};
};

// Use Vec4 instances instead of Vec3 to ensure proper alignment in the shader.
struct GpuRenderProxyLocalData {
  math::Vec4 local_center{0.0f};
  math::Vec4 local_max_extents{0.0f};
  math::Mat4 transform{1.0f};
};

struct GpuRenderProxyInstance {
  RenderProxyId proxy_id{kInvalidRenderProxyId};
  BatchId batch_id{kInvalidBatchId};
};

struct GpuIndirectRenderProxy {
  VkDrawIndexedIndirectCommand command{};
  RenderProxyId proxy_id{kInvalidRenderProxyId};
  BatchId batch_id{kInvalidBatchId};
};
}  // namespace vk
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_VULKAN_DATA_VULKAN_RENDER_PROXY_H_
