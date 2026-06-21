// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RENDER_DRIVER_VULKAN_HANDLER_VULKAN_LIGHTING_HANDLER_H_
#define COMET_RENDER_DRIVER_VULKAN_HANDLER_VULKAN_LIGHTING_HANDLER_H_

#include "comet/core/container/array.h"
#include "comet/core/essentials.h"
#include "comet/core/math/matrix.h"
#include "comet/core/memory/memory.h"
#include "comet/render/driver/vulkan/handler/vulkan_handler.h"
#include "comet/render/driver/vulkan/handler/vulkan_render_pass_handler.h"
#include "comet/render/driver/vulkan/handler/vulkan_sampler_handler.h"
#include "comet/render/driver/vulkan/handler/vulkan_texture_handler.h"
#include "comet/render/driver/vulkan/type/vulkan_buffer.h"
#include "comet/render/driver/vulkan/type/vulkan_light.h"
#include "comet/render/driver/vulkan/type/vulkan_shadow.h"
#include "comet/render/driver/vulkan/type/vulkan_texture_map.h"
#include "comet/render/render_handle.h"
#include "comet/runtime/camera/camera.h"
#include "comet/runtime/frame/frame_packet.h"
#include "comet/runtime/light/light.h"
#include "comet/runtime/light/light_handle.h"
#include "comet/runtime/memory/allocator/free_list_allocator.h"
#include "comet/runtime/memory/allocator/platform_allocator.h"
#include "comet/runtime/memory/memory_tag.h"

namespace comet {
namespace render {
namespace vk {
struct LightGpuData {
  VkBuffer ssbo_lights_handle{VK_NULL_HANDLE};
  VkDeviceSize ssbo_lights_size{0};
};

struct ShadowGpuData {
  VkBuffer ssbo_shadow_data_handle{VK_NULL_HANDLE};
  VkDeviceSize ssbo_shadow_data_size{0};
};

struct LightingHandlerDescr : HandlerDescr {
  const ShadowSettings* shadow_settings{nullptr};
  TextureHandler* texture_handler{nullptr};
  SamplerHandler* sampler_handler{nullptr};
  RenderPassHandler* render_pass_handler{nullptr};
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

  void Update(const frame::FramePacket* packet);

  const LightProxy* Get(LightHandle handle) const;
  const LightProxy* TryGetLight(LightHandle handle) const noexcept;

  u32 GetLightCount() const noexcept;
  LightGpuData GetLightGpuData(FrameInFlightIndex frame_index) const noexcept;
  ShadowGpuData GetShadowGpuData(FrameInFlightIndex frame_index) const noexcept;

  const frame::FrameArray<ShadowRenderJob>* GetRenderJobs() const noexcept;
  const TextureMap* GetShadowArrayTextureMap() const noexcept;
  VkImage GetShadowArrayImageHandle() const noexcept;
  VkFormat GetShadowArrayFormat() const noexcept;

  void SetRenderPass(RenderPassHandle render_pass_handle) noexcept;

 protected:
  void OnInitialize() override;
  void OnShutdown() override;

 private:
  static inline constexpr usize kDefaultLightCount_{128};
  static inline constexpr u32 kMaxShadowCascades_{4};

  void AddLights(const frame::AddedLights* lights);
  void UpdateLights(const frame::DirtyLights* lights);
  void RemoveLights(const frame::RemovedLights* lights);

  void AddShadowForLight(const LightProxy& light);
  void UpdateShadowForLight(const LightProxy& light);
  void RemoveShadowForLight(LightHandle light_handle);

  void RebuildRenderJobs(const frame::FramePacket* packet);
  void UploadGpuLights(FrameInFlightIndex frame_index);
  void UploadGpuShadowData(FrameInFlightIndex frame_index);

  GpuLight GenerateGpuLight(const LightProxy& light) const;

  void InitializeShadowArrayResources();
  void DestroyShadowArrayResources();

  s32 AllocateShadowLayers(u32 layer_count);
  void FreeShadowLayers(s32 first_layer, u32 layer_count);

  void InitializeShadowResource(LightHandle light_handle,
                                const LightProxy& light,
                                ShadowResource& resource);
  void DestroyShadowResource(ShadowResource& resource);
  void RecreateShadowResourceIfNeeded(ShadowResource& resource,
                                      const LightProxy& light);

  ShadowResource* TryGetShadowResource(LightHandle light_handle) noexcept;
  const ShadowResource* TryGetShadowResource(
      LightHandle light_handle) const noexcept;

  bool IsLightSlotAlive(usize index) const noexcept;
  bool IsShadowSlotAlive(usize index) const noexcept;

  void PopulateCascadeSplits(const CameraViewData* camera_data,
                             f32 max_distance, u32 cascade_count, f32 lambda,
                             f32* out_splits) const;

  math::Mat4 ComputeDirectionalCascadeViewProj(
      const math::Vec3& light_dir, const StaticArray<math::Vec3, 8>& corners,
      f32 cascade_near, f32 cascade_far) const;

  math::Mat4 ComputeSpotLightViewProj(const LightProperties& props,
                                      f32 max_distance) const;

  const TextureHandler* GetTextureHandler() const;

  memory::PlatformAllocator platform_allocator_{kEngineMemoryTagRender};
  memory::FiberFreeListAllocator allocator_{sizeof(u32), 1024 * 64,
                                            kEngineMemoryTagRender};

  Array<LightProxy> proxies_{};
  Array<ShadowResource> shadow_resources_{};

  Array<Buffer> ssbo_lights_{};
  Array<Buffer> ssbo_shadow_data_{};

  TextureMap shadow_array_texture_map_{};

  Array<bool> shadow_layer_usage_{};

  RenderPassHandle render_pass_handle_{};
  const ShadowSettings* shadow_settings_{nullptr};
  frame::FrameArray<ShadowRenderJob>* render_jobs_{nullptr};

  TextureHandler* texture_handler_{nullptr};
  SamplerHandler* sampler_handler_{nullptr};
  RenderPassHandler* render_pass_handler_{nullptr};
};
}  // namespace vk
}  // namespace render
}  // namespace comet

#endif  // COMET_RENDER_DRIVER_VULKAN_HANDLER_VULKAN_LIGHTING_HANDLER_H_