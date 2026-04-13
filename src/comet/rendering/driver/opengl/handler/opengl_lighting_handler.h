// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_OPENGL_HANDLER_OPENGL_LIGHTING_HANDLER_H_
#define COMET_COMET_RENDERING_DRIVER_OPENGL_HANDLER_OPENGL_LIGHTING_HANDLER_H_

// External. ///////////////////////////////////////////////////////////////////
#include "glad/glad.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/essentials.h"
#include "comet/core/frame/frame_packet.h"
#include "comet/core/memory/allocator/free_list_allocator.h"
#include "comet/core/memory/allocator/platform_allocator.h"
#include "comet/core/memory/memory.h"
#include "comet/core/type/array.h"
#include "comet/core/type/map.h"
#include "comet/math/matrix.h"
#include "comet/rendering/driver/opengl/data/opengl_frame.h"
#include "comet/rendering/driver/opengl/data/opengl_light.h"
#include "comet/rendering/driver/opengl/data/opengl_shadow.h"
#include "comet/rendering/driver/opengl/data/opengl_storage.h"
#include "comet/rendering/driver/opengl/data/opengl_texture.h"
#include "comet/rendering/driver/opengl/data/opengl_texture_map.h"
#include "comet/rendering/driver/opengl/handler/opengl_handler.h"
#include "comet/rendering/light/light_common.h"
#include "comet/rendering/rendering_common.h"

namespace comet {
namespace rendering {
namespace gl {
struct LightGpuData {
  StorageHandle ssbo_lights_handle{kInvalidStorageHandle};
  GLsizeiptr ssbo_lights_size{0};
};

struct ShadowGpuData {
  StorageHandle ssbo_shadow_data_handle{kInvalidStorageHandle};
  GLsizeiptr ssbo_shadow_data_size{0};
};

struct LightingHandlerDescr : HandlerDescr {
  const ShadowSettings* shadow_settings{nullptr};
};

class LightingHandler : public Handler {
 public:
  LightingHandler() = delete;
  explicit LightingHandler(const LightingHandlerDescr& descr);
  LightingHandler(const LightingHandler&) = delete;
  LightingHandler(LightingHandler&&) = delete;
  LightingHandler& operator=(const LightingHandler&) = delete;
  LightingHandler& operator=(LightingHandler&&) = delete;
  ~LightingHandler() override = default;

  void Initialize() override;
  void Shutdown() override;
  void Update(const frame::FramePacket* packet);

  const LightProxy* Get(LightProxyHandle handle) const;
  const LightProxy* TryGetLight(LightId light_id) const noexcept;

  u32 GetLightCount() const noexcept;
  LightGpuData GetLightGpuData(FrameInFlightIndex frame_index) const noexcept;
  ShadowGpuData GetShadowGpuData(FrameInFlightIndex frame_index) const noexcept;

  const frame::FrameArray<ShadowRenderJob>* GetRenderJobs() const noexcept;
  const TextureMap* GetShadowArrayTextureMap() const noexcept;
  TextureHandle GetShadowArrayTextureHandle() const noexcept;
  GLenum GetShadowArrayFormat() const noexcept;

 private:
  static inline constexpr usize kDefaultLightCount_{128};
  static inline constexpr u32 kShadowLayerCapacity_{128};
  static inline constexpr u32 kMaxShadowCascades_{4};

  void AddLights(const frame::AddedLights* lights);
  void UpdateLights(const frame::DirtyLights* lights);
  void RemoveLights(const frame::RemovedLights* lights);

  void AddShadowForLight(const LightProxy& light);
  void UpdateShadowForLight(const LightProxy& light);
  void RemoveShadowForLight(LightId light_id);

  void RebuildRenderJobs(const frame::FramePacket* packet);
  void UploadGpuLights(FrameInFlightIndex frame_index);
  void UploadGpuShadowData(FrameInFlightIndex frame_index);

  GpuLight GenerateGpuLight(const LightProxy& light) const;

  void InitializeShadowArrayResources();
  void DestroyShadowArrayResources();

  s32 AllocateShadowLayers(u32 layer_count);
  void FreeShadowLayers(s32 first_layer, u32 layer_count);

  void InitializeShadowResource(LightId light_id, const LightProxy& light,
                                ShadowResource& resource);
  void DestroyShadowResource(ShadowResource& resource);
  void RecreateShadowResourceIfNeeded(ShadowResource& resource,
                                      const LightProxy& light);

  ShadowResource* TryGetShadowResource(LightId light_id) noexcept;
  const ShadowResource* TryGetShadowResource(LightId light_id) const noexcept;

  void PopulateCascadeSplits(const RenderCameraData& camera_data,
                             f32 max_distance, u32 cascade_count, f32 lambda,
                             f32* out_splits) const;

  math::Mat4 ComputeDirectionalCascadeViewProj(
      const RenderCameraData& camera_data, const math::Vec3& light_dir,
      f32 cascade_near, f32 cascade_far) const;

  math::Mat4 ComputeSpotLightViewProj(const LightProperties& props,
                                      f32 max_distance) const;

  void EnsureStorageBufferCapacity(StorageHandle& handle, GLsizeiptr& capacity,
                                   GLsizeiptr required_size);

  memory::PlatformAllocator platform_allocator_{
      memory::kEngineMemoryTagRendering};
  memory::FiberFreeListAllocator allocator_{sizeof(u32), 1024 * 64,
                                            memory::kEngineMemoryTagRendering};

  Map<LightId, usize> light_to_proxy_map_{};
  Array<LightProxy> proxies_{};

  Map<LightId, usize> light_to_shadow_map_{};
  Array<ShadowResource> shadow_resources_{};

  Array<StorageHandle> ssbo_lights_{};
  Array<StorageHandle> ssbo_shadow_data_{};

  Array<GLsizeiptr> ssbo_lights_size_{};
  Array<GLsizeiptr> ssbo_shadow_data_size_{};

  TextureHandle shadow_array_texture_handle_{kInvalidTextureHandle};
  Sampler shadow_array_sampler_wrapper_{};
  Texture shadow_array_texture_{};
  TextureMap shadow_array_texture_map_{};

  Array<bool> shadow_layer_usage_{};

  const ShadowSettings* shadow_settings_{nullptr};
  frame::FrameArray<ShadowRenderJob>* render_jobs_{nullptr};
};
}  // namespace gl
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_OPENGL_HANDLER_OPENGL_LIGHTING_HANDLER_H_