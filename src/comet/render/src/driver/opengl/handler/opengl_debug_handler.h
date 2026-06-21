// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RENDER_DRIVER_OPENGL_HANDLER_OPENGL_DEBUG_HANDLER_H_
#define COMET_RENDER_DRIVER_OPENGL_HANDLER_OPENGL_DEBUG_HANDLER_H_

// External. ///////////////////////////////////////////////////////////////////
#include "glad/glad.h"
////////////////////////////////////////////////////////////////////////////////

#ifdef COMET_DEBUG_RENDERING
#include "comet/core/essentials.h"
#include "comet/runtime/memory/allocator/platform_allocator.h"
#include "comet/core/container/array.h"
#include "comet/core/math/matrix.h"
#include "comet/render/driver/opengl/handler/opengl_handler.h"
#include "comet/render/driver/opengl/type/opengl_buffer.h"
#include "comet/render/driver/opengl/type/opengl_debug.h"
#include "comet/render/driver/opengl/type/opengl_shadow.h"
#include "comet/render/render_proxy_record_store.h"

namespace comet {
namespace render {
namespace gl {
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

  void PrepareCullDebugWrite(FrameInFlightIndex frame_index);

  u32 GetVisibleCount() const noexcept;

  void SetDebugCameraFrustum(u32 index, const math::Mat4& view_proj);
  void ClearDebugCameraFrustums();

  void SetDebugCascadeFrustumCorners(u32 index,
                                     const StaticArray<math::Vec3, 8>& corners);
  void ClearDebugCascadeFrustumCorners();

  void SetDebugLightFrustum(u32 index, const math::Mat4& view_proj);
  void ClearDebugLightFrustums();

  GlBufferView GetDebugLineBuffer() const noexcept;
  GlBufferView GetDebugAabbBuffer() const noexcept;
  GlBufferView GetDebugCameraFrustumBuffer() const noexcept;
  GlBufferView GetDebugCascadeFrustumCornersBuffer() const noexcept;
  GlBufferView GetDebugLightFrustumBuffer() const noexcept;

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

  GLsizei GetAabbBufferSize(u32 count) const noexcept;
  GLsizei GetLineBufferSize(u32 aabb_count, u32 camera_frustum_count,
                            u32 cascade_frustum_count,
                            u32 light_frustum_count) const noexcept;

  void ReallocateBufferIfNeeded(GlNativeStorageHandle buffer_handle,
                                GLsizei& buffer_size, GLsizei required_size,
                                GLenum target, GLenum usage);

  memory::PlatformAllocator platform_allocator_{
      kEngineMemoryTagRender};

  u32 aabb_count_{0};
  u32 visible_count_{0};

  GlNativeStorageHandle ssbo_debug_data_handle_[kDebugDataBufferCount_]{};
  GLsizei ssbo_debug_data_size_[kDebugDataBufferCount_]{};
  GpuDebugData* debug_data_[kDebugDataBufferCount_]{nullptr, nullptr};

  GlNativeStorageHandle ssbo_debug_aabbs_handle_{kInvalidGlNativeStorageHandle};
  GLsizei ssbo_debug_aabbs_size_{0};

  GlNativeStorageHandle ssbo_debug_lines_handle_{kInvalidGlNativeStorageHandle};
  GLsizei ssbo_debug_lines_size_{0};

  Array<math::Mat4> debug_camera_frustums_{};
  GlNativeStorageHandle ssbo_debug_camera_frustums_handle_{
      kInvalidGlNativeStorageHandle};
  GLsizei ssbo_debug_camera_frustums_size_{0};

  Array<DebugFrustumCorners> debug_cascade_frustums_{};
  GlNativeStorageHandle ssbo_debug_cascade_frustums_handle_{
      kInvalidGlNativeStorageHandle};
  GLsizei ssbo_debug_cascade_frustums_size_{0};

  Array<math::Mat4> debug_light_frustums_{};
  GlNativeStorageHandle ssbo_debug_light_frustums_handle_{
      kInvalidGlNativeStorageHandle};
  GLsizei ssbo_debug_light_frustums_size_{0};

  const RenderProxyRecordStore* render_proxy_record_store_{nullptr};
};
}  // namespace gl
}  // namespace render
}  // namespace comet

#endif  // COMET_DEBUG_RENDERING

#endif  // COMET_RENDER_DRIVER_OPENGL_HANDLER_OPENGL_DEBUG_HANDLER_H_