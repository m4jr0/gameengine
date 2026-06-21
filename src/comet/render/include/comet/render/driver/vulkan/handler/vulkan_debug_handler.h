// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RENDER_DRIVER_VULKAN_HANDLER_VULKAN_DEBUG_HANDLER_H_
#define COMET_RENDER_DRIVER_VULKAN_HANDLER_VULKAN_DEBUG_HANDLER_H_

// External. ///////////////////////////////////////////////////////////////////
#include "vulkan/vulkan.h"
////////////////////////////////////////////////////////////////////////////////

#ifdef COMET_DEBUG_RENDERING
#include "comet/core/essentials.h"
#include "comet/runtime/memory/allocator/platform_allocator.h"
#include "comet/core/container/array.h"
#include "comet/core/math/matrix.h"
#include "comet/render/driver/vulkan/handler/vulkan_handler.h"
#include "comet/render/driver/vulkan/type/vulkan_buffer.h"
#include "comet/render/driver/vulkan/type/vulkan_debug.h"
#include "comet/render/driver/vulkan/type/vulkan_shadow.h"
#include "comet/render/render_proxy_record_store.h"

namespace comet {
namespace rendering {
namespace vk {
struct DebugHandlerDescr : HandlerDescr {
  const RenderProxyRecordStore* render_proxy_record_store{nullptr};
};

class DebugHandler : public Handler {
 public:
  DebugHandler() = delete;
  explicit DebugHandler(const DebugHandlerDescr& descr);
  DebugHandler(const DebugHandler&) = delete;
  DebugHandler(DebugHandler&&) = delete;
  DebugHandler& operator=(const DebugHandler&) = delete;
  DebugHandler& operator=(DebugHandler&&) = delete;
  ~DebugHandler() override = default;

  void Update();

  DebugGpuData GetGpuData(FrameInFlightIndex frame_index) const noexcept;

  u32 GetAabbCount() const noexcept;

  void PopulateCullDebugReadBarriers(FrameInFlightIndex frame_index,
                                     Array<VkBufferMemoryBarrier>& out) const;
  void PrepareCullDebugWrite(FrameInFlightIndex frame_index);

  u32 GetVisibleCount() const noexcept;

  void SetDebugCameraFrustum(u32 index, const math::Mat4& view_proj);
  void ClearDebugCameraFrustums();

  void SetDebugCascadeFrustumCorners(u32 index,
                                     const StaticArray<math::Vec3, 8>& corners);
  void ClearDebugCascadeFrustumCorners();

  void SetDebugLightFrustum(u32 index, const math::Mat4& view_proj);
  void ClearDebugLightFrustums();

  const Buffer& GetDebugLineBuffer() const noexcept;
  const Buffer& GetDebugAabbBuffer() const noexcept;
  const Buffer& GetDebugCameraFrustumBuffer() const noexcept;
  const Buffer& GetDebugCascadeFrustumCornersBuffer() const noexcept;
  const Buffer& GetDebugLightFrustumBuffer() const noexcept;

  u32 GetDebugAabbCount() const noexcept;
  u32 GetDebugLineVertexCount() const noexcept;
  u32 GetDebugCameraFrustumCount() const noexcept;
  u32 GetDebugCascadeFrustumCount() const noexcept;
  u32 GetDebugLightFrustumCount() const noexcept;

 protected:
  void OnInitialize() override;
  void OnShutdown() override;

 private:
  static inline constexpr usize kDebugDataBufferCount_{2};
  static inline constexpr u32 kDebugAabbLineVertexCount_{24};
  static inline constexpr u32 kDebugFrustumLineVertexCount_{24};
  static inline constexpr u32 kDefaultDebugAabbCount_{512};
  static inline constexpr u32 kMaxDebugCascadeFrustumCount_{
      kMaxShadowLayerCount};
  static inline constexpr u32 kMaxDebugLightFrustumCount_{kMaxShadowLayerCount};
  static inline constexpr u32 kMaxDebugCameraFrustumCount_{5};

  void InitializeReadbackData();
  void DestroyReadbackData();

  void InitializeGpuData();
  void DestroyGpuData();

  void EnsureAabbCapacity(u32 count);
  void EnsureLineCapacity(u32 aabb_count, u32 camera_frustum_count,
                          u32 cascade_frustum_count, u32 light_frustum_count);

  VkDeviceSize GetAabbBufferSize(u32 count) const noexcept;
  VkDeviceSize GetLineBufferSize(u32 aabb_count, u32 camera_frustum_count,
                                 u32 cascade_frustum_count,
                                 u32 light_frustum_count) const noexcept;

  memory::PlatformAllocator platform_allocator_{
      kEngineMemoryTagRender};

  u32 aabb_count_{0};
  u32 visible_count_{0};

  Buffer ssbo_debug_data_[kDebugDataBufferCount_]{};
  GpuDebugData* debug_data_[kDebugDataBufferCount_]{nullptr, nullptr};

  Buffer ssbo_debug_aabbs_{};
  Buffer ssbo_debug_lines_{};

  Array<math::Mat4> debug_camera_frustums_{};
  Buffer ssbo_debug_camera_frustums_{};

  Array<DebugFrustumCorners> debug_cascade_frustums_{};
  Buffer ssbo_debug_cascade_frustums_{};

  Array<math::Mat4> debug_light_frustums_{};
  Buffer ssbo_debug_light_frustums_{};

  const RenderProxyRecordStore* render_proxy_record_store_{nullptr};
};
}  // namespace vk
}  // namespace rendering
}  // namespace comet

#endif  // COMET_DEBUG_RENDERING

#endif  // COMET_RENDER_DRIVER_VULKAN_HANDLER_VULKAN_DEBUG_HANDLER_H_