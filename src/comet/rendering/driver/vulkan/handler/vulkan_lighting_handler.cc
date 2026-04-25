// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "vulkan_lighting_handler.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/profiler/profiler.h"
#include "comet/rendering/driver/vulkan/utils/vulkan_buffer_utils.h"
#include "comet/rendering/driver/vulkan/utils/vulkan_image_utils.h"
#include "comet/rendering/driver/vulkan/utils/vulkan_texture_map_utils.h"
#include "comet/rendering/driver/vulkan/vulkan_alloc.h"
#include "comet/rendering/driver/vulkan/vulkan_debug.h"
#include "comet/rendering/type/camera.h"
#include "comet/rendering/type/common.h"
#include "comet/rendering/type/light.h"
#include "comet/rendering/type/texture.h"
#include "comet/rendering/utils/camera_utils.h"
#include "comet/rendering/utils/culling_utils.h"
#include "comet/rendering/utils/light_utils.h"

namespace comet {
namespace rendering {
namespace vk {
LightingHandler::LightingHandler(const LightingHandlerDescr& descr)
    : Handler{descr},
      shadow_settings_{descr.shadow_settings},
      texture_handler_{descr.texture_handler},
      sampler_handler_{descr.sampler_handler},
      render_pass_handler_{descr.render_pass_handler} {
  COMET_ASSERT(shadow_settings_ != nullptr, "LightingHandler::LightingHandler",
               "shadow settings are null");
  COMET_ASSERT(texture_handler_ != nullptr, "LightingHandler::LightingHandler",
               "texture handler is null");
  COMET_ASSERT(sampler_handler_ != nullptr, "LightingHandler::LightingHandler",
               "sampler handler is null");
  COMET_ASSERT(render_pass_handler_ != nullptr,
               "LightingHandler::LightingHandler",
               "render pass handler is null");
}

void LightingHandler::Update(const frame::FramePacket* packet) {
  COMET_PROFILE("LightingHandler::Update");
  COMET_ASSERT(packet != nullptr, "LightingHandler::Update",
               "frame packet is null");

  if (!packet->added_lights->IsEmpty()) {
    AddLights(packet->added_lights);
  }

  if (!packet->dirty_lights->IsEmpty()) {
    UpdateLights(packet->dirty_lights);
  }

  if (!packet->removed_lights->IsEmpty()) {
    RemoveLights(packet->removed_lights);
  }

  const auto frame_index{context_->GetFrameInFlightIndex()};
  RebuildRenderJobs(packet);
  UploadGpuShadowData(frame_index);
  UploadGpuLights(frame_index);
}

const LightProxy* LightingHandler::Get(LightHandle handle) const {
  const auto* proxy{TryGetLight(handle)};
  COMET_ASSERT(proxy != nullptr, "LightingHandler::Get",
               "light proxy does not exist", "light_handle", handle);
  return proxy;
}

const LightProxy* LightingHandler::TryGetLight(
    LightHandle handle) const noexcept {
  if (!handle) {
    return nullptr;
  }

  const auto index{static_cast<usize>(handle.GetIndex())};

  if (index >= proxies_.GetSize()) {
    return nullptr;
  }

  const auto& proxy{proxies_[index]};

  if (!proxy.handle || proxy.handle != handle) {
    return nullptr;
  }

  return &proxy;
}

u32 LightingHandler::GetLightCount() const noexcept {
  u32 count{0};

  for (usize i{0}; i < proxies_.GetSize(); ++i) {
    if (IsLightSlotAlive(i)) {
      ++count;
    }
  }

  return count;
}

LightGpuData LightingHandler::GetLightGpuData(
    FrameInFlightIndex frame_index) const noexcept {
  const auto& buffer{ssbo_lights_[frame_index]};

  return {
      .ssbo_lights_handle = buffer.handle,
      .ssbo_lights_size = buffer.size,
  };
}

ShadowGpuData LightingHandler::GetShadowGpuData(
    FrameInFlightIndex frame_index) const noexcept {
  const auto& buffer{ssbo_shadow_data_[frame_index]};

  return {
      .ssbo_shadow_data_handle = buffer.handle,
      .ssbo_shadow_data_size = buffer.size,
  };
}

const frame::FrameArray<ShadowRenderJob>* LightingHandler::GetRenderJobs()
    const noexcept {
  return render_jobs_;
}

const TextureMap* LightingHandler::GetShadowArrayTextureMap() const noexcept {
  return shadow_array_texture_map_.texture_handle ? &shadow_array_texture_map_
                                                  : nullptr;
}

VkImage LightingHandler::GetShadowArrayImageHandle() const noexcept {
  if (!shadow_array_texture_map_.texture_handle) {
    return VK_NULL_HANDLE;
  }

  const auto* texture{
      GetTextureHandler()->Get(shadow_array_texture_map_.texture_handle)};
  return texture != nullptr ? texture->image.handle : VK_NULL_HANDLE;
}

VkFormat LightingHandler::GetShadowArrayFormat() const noexcept {
  if (!shadow_array_texture_map_.texture_handle) {
    return VK_FORMAT_UNDEFINED;
  }

  const auto* texture{
      GetTextureHandler()->Get(shadow_array_texture_map_.texture_handle)};
  return texture != nullptr ? texture->format : VK_FORMAT_UNDEFINED;
}

void LightingHandler::SetRenderPass(
    RenderPassHandle render_pass_handle) noexcept {
  render_pass_handle_ = render_pass_handle;
}

void LightingHandler::OnInitialize() {
  allocator_.Initialize();

  proxies_ = Array<LightProxy>::WithCapacity(&allocator_, kDefaultLightCount_);
  shadow_resources_ =
      Array<ShadowResource>::WithCapacity(&allocator_, kDefaultLightCount_);

  const auto frame_count{context_->GetMaxFramesInFlight()};

  ssbo_lights_ = Array<Buffer>{&platform_allocator_};
  ssbo_lights_.Resize(frame_count);

  for (usize i{0}; i < frame_count; ++i) {
    ssbo_lights_[i] = GenerateBuffer(
        context_->GetAllocatorHandle(), kDefaultLightCount_ * sizeof(GpuLight),
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VMA_MEMORY_USAGE_CPU_TO_GPU, 0, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        VK_SHARING_MODE_EXCLUSIVE, "ssbo_lights_");
  }

  ssbo_shadow_data_ = Array<Buffer>{&platform_allocator_};
  ssbo_shadow_data_.Resize(frame_count);

  for (usize i{0}; i < frame_count; ++i) {
    ssbo_shadow_data_[i] = GenerateBuffer(
        context_->GetAllocatorHandle(),
        kShadowLayerCapacity_ * sizeof(GpuShadowData),
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VMA_MEMORY_USAGE_CPU_TO_GPU, 0, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        VK_SHARING_MODE_EXCLUSIVE, "ssbo_shadow_data_");
  }

  shadow_layer_usage_ = Array<bool>{&allocator_};
  shadow_layer_usage_.Resize(kShadowLayerCapacity_);

  for (u32 i{0}; i < kShadowLayerCapacity_; ++i) {
    shadow_layer_usage_[i] = false;
  }

  InitializeShadowArrayResources();
}

void LightingHandler::OnShutdown() {
  for (auto& resource : shadow_resources_) {
    DestroyShadowResource(resource);
  }

  shadow_resources_.Release();

  DestroyShadowArrayResources();

  for (auto& buffer : ssbo_shadow_data_) {
    if (IsBufferInitialized(buffer)) {
      DestroyBuffer(buffer);
    }
  }

  ssbo_shadow_data_.Release();

  for (auto& buffer : ssbo_lights_) {
    if (IsBufferInitialized(buffer)) {
      DestroyBuffer(buffer);
    }
  }

  ssbo_lights_.Release();

  shadow_layer_usage_.Release();
  proxies_.Release();

  allocator_.Destroy();
  render_jobs_ = nullptr;
  render_pass_handle_.Invalidate();
}

void LightingHandler::AddLights(const frame::AddedLights* lights) {
  for (const auto& payload : *lights) {
    const auto handle{payload.light_handle};

    if (!handle) {
      continue;
    }

    const auto index{static_cast<usize>(handle.GetIndex())};

    if (index >= proxies_.GetSize()) {
      proxies_.Resize(index + 1);
    }

    auto& proxy{proxies_[index]};
    proxy.handle = handle;
    proxy.props = payload.props;
    proxy.shadow = payload.shadow;
    proxy.gpu_shadow_index = -1;
    proxy.shadow_entry_count = 0;
    proxy.is_dirty = true;

    AddShadowForLight(proxy);
  }
}

void LightingHandler::UpdateLights(const frame::DirtyLights* lights) {
  for (const auto& payload : *lights) {
    const auto handle{payload.light_handle};
    const auto index{static_cast<usize>(handle.GetIndex())};

    if (index >= proxies_.GetSize()) {
      continue;
    }

    auto& proxy{proxies_[index]};

    if (!proxy.handle || proxy.handle != handle) {
      continue;
    }

    proxy.props = payload.props;
    proxy.shadow = payload.shadow;
    proxy.is_dirty = true;

    UpdateShadowForLight(proxy);
  }
}

void LightingHandler::RemoveLights(const frame::RemovedLights* lights) {
  for (const auto& payload : *lights) {
    const auto handle{payload.light_handle};

    RemoveShadowForLight(handle);

    const auto index{static_cast<usize>(handle.GetIndex())};

    if (index >= proxies_.GetSize()) {
      continue;
    }

    auto& proxy{proxies_[index]};

    if (!proxy.handle || proxy.handle != handle) {
      continue;
    }

    proxy = {};
  }
}

void LightingHandler::AddShadowForLight(const LightProxy& light) {
  const auto shadow_type{ResolveShadowType(light.props, light.shadow)};

  if (shadow_type == ShadowType::None) {
    return;
  }

  if (!IsShadowSupportedForLight(light.props, light.shadow)) {
    if (shadow_type == ShadowType::PointCubemap) {
      COMET_LOG_WARNING(LoggerType::Rendering,
                        "LightingHandler::AddShadowForLight",
                        "point cubemap shadows not implemented", "light_handle",
                        light.handle);
    }
    return;
  }

  const auto index{static_cast<usize>(light.handle.GetIndex())};

  if (index >= shadow_resources_.GetSize()) {
    shadow_resources_.Resize(index + 1);
  }

  auto& resource{shadow_resources_[index]};

  if (resource.light_handle == light.handle) {
    return;
  }

  InitializeShadowResource(light.handle, light, resource);
}

void LightingHandler::UpdateShadowForLight(const LightProxy& light) {
  const auto shadow_type{ResolveShadowType(light.props, light.shadow)};
  const auto is_supported{IsShadowSupportedForLight(light.props, light.shadow)};
  auto* existing{TryGetShadowResource(light.handle)};

  if (shadow_type == ShadowType::None || !is_supported) {
    if (existing != nullptr) {
      RemoveShadowForLight(light.handle);
    }
    return;
  }

  if (existing == nullptr) {
    AddShadowForLight(light);
    return;
  }

  RecreateShadowResourceIfNeeded(*existing, light);
  existing->max_distance = light.shadow.max_distance;
  existing->bias_constant = light.shadow.bias_constant;
  existing->bias_slope = light.shadow.bias_slope;
  existing->is_dirty = true;
}

void LightingHandler::RemoveShadowForLight(LightHandle light_handle) {
  auto* resource{TryGetShadowResource(light_handle)};

  if (resource == nullptr) {
    return;
  }

  DestroyShadowResource(*resource);
}

void LightingHandler::RebuildRenderJobs(const frame::FramePacket* packet) {
  COMET_PROFILE("LightingHandler::RebuildRenderJobs");

  usize live_shadow_count{0};

  for (usize i{0}; i < shadow_resources_.GetSize(); ++i) {
    if (IsShadowSlotAlive(i)) {
      ++live_shadow_count;
    }
  }

  render_jobs_ = COMET_FRAME_ARRAY_WITH_CAPACITY(
      ShadowRenderJob, live_shadow_count * kMaxShadowCascades_);

  for (usize i{0}; i < proxies_.GetSize(); ++i) {
    if (!IsLightSlotAlive(i)) {
      continue;
    }

    proxies_[i].gpu_shadow_index = -1;
    proxies_[i].shadow_entry_count = 0;
  }

  const auto& camera_data{packet->camera_data};

  for (usize i{0}; i < shadow_resources_.GetSize(); ++i) {
    if (!IsShadowSlotAlive(i)) {
      continue;
    }

    auto& resource{shadow_resources_[i]};
    const auto* light{TryGetLight(resource.light_handle)};

    if (light == nullptr) {
      continue;
    }

    const auto first_shadow_index{static_cast<s32>(render_jobs_->GetSize())};
    resource.gpu_shadow_index = first_shadow_index;

    const auto light_index{
        static_cast<usize>(resource.light_handle.GetIndex())};
    proxies_[light_index].gpu_shadow_index = first_shadow_index;
    proxies_[light_index].shadow_entry_count = resource.view_proj_count;
    proxies_[light_index].is_dirty = true;

    if (light->props.type == LightType::Directional &&
        resource.type == ShadowType::DirectionalOrtho) {
      PopulateCascadeSplits(
          camera_data, resource.max_distance, resource.view_proj_count,
          light->shadow.cascade_lambda, resource.cascade_splits);

      f32 cascade_near{camera_data.near_plane};

      for (u32 j{0}; j < resource.view_proj_count; ++j) {
        const f32 cascade_far{resource.cascade_splits[j]};

        resource.view_proj[j] = ComputeDirectionalCascadeViewProj(
            camera_data, light->props.direction, cascade_near, cascade_far);

        ShadowRenderJob job{};
        job.light_handle = resource.light_handle;
        job.type = resource.type;
        job.extent = resource.extent;
        job.bias_constant = resource.bias_constant;
        job.bias_slope = resource.bias_slope;
        job.resource = &resource;
        job.view_proj_index = j;
        job.view_proj = resource.view_proj[j];
        job.framebuffer = resource.framebuffers[j];
        render_jobs_->PushLast(job);

        cascade_near = cascade_far;
      }
    } else if (light->props.type == LightType::Spot &&
               resource.type == ShadowType::SpotPerspective) {
      resource.cascade_splits[0] = resource.max_distance;
      resource.view_proj[0] =
          ComputeSpotLightViewProj(light->props, resource.max_distance);

      ShadowRenderJob job{};
      job.light_handle = resource.light_handle;
      job.type = resource.type;
      job.extent = resource.extent;
      job.bias_constant = resource.bias_constant;
      job.bias_slope = resource.bias_slope;
      job.resource = &resource;
      job.view_proj_index = 0;
      job.view_proj = resource.view_proj[0];
      job.framebuffer = resource.framebuffers[0];
      render_jobs_->PushLast(job);
    }
  }
}

void LightingHandler::UploadGpuLights(FrameInFlightIndex frame_index) {
  usize live_light_count{0};

  for (usize i{0}; i < proxies_.GetSize(); ++i) {
    if (IsLightSlotAlive(i)) {
      ++live_light_count;
    }
  }

  if (live_light_count == 0) {
    return;
  }

  auto& ssbo_lights{ssbo_lights_[frame_index]};

  const auto required_size{
      static_cast<VkDeviceSize>(live_light_count * sizeof(GpuLight))};

  if (required_size > ssbo_lights.size) {
    ResizeBuffer(
        ssbo_lights, context_->GetDevice(),
        context_->GetFrameData().command_pool_handle,
        context_->GetAllocatorHandle(),
        math::Max<VkDeviceSize>(required_size, sizeof(GpuLight)),
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VMA_MEMORY_USAGE_CPU_TO_GPU,
        context_->GetDevice().GetGraphicsQueueHandle(), 0,
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, VK_SHARING_MODE_EXCLUSIVE,
        VK_NULL_HANDLE, "ssbo_lights_");
  }

  ScopedMappedBuffer mapped{ssbo_lights};
  auto* gpu_lights{static_cast<GpuLight*>(ssbo_lights.mapped_memory)};

  usize gpu_index{0};

  for (usize i{0}; i < proxies_.GetSize(); ++i) {
    if (!IsLightSlotAlive(i)) {
      continue;
    }

    gpu_lights[gpu_index++] = GenerateGpuLight(proxies_[i]);
    proxies_[i].is_dirty = false;
  }
}

void LightingHandler::UploadGpuShadowData(FrameInFlightIndex frame_index) {
  const auto shadow_count{render_jobs_ != nullptr ? render_jobs_->GetSize()
                                                  : 0};

  if (shadow_count == 0) {
    return;
  }

  auto& ssbo_shadow_data{ssbo_shadow_data_[frame_index]};

  const auto required_size{
      static_cast<VkDeviceSize>(shadow_count * sizeof(GpuShadowData))};

  if (required_size > ssbo_shadow_data.size) {
    ResizeBuffer(
        ssbo_shadow_data, context_->GetDevice(),
        context_->GetFrameData().command_pool_handle,
        context_->GetAllocatorHandle(),
        math::Max<VkDeviceSize>(required_size,
                                kShadowLayerCapacity_ * sizeof(GpuShadowData)),
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VMA_MEMORY_USAGE_CPU_TO_GPU,
        context_->GetDevice().GetGraphicsQueueHandle(), 0,
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, VK_SHARING_MODE_EXCLUSIVE,
        VK_NULL_HANDLE, "ssbo_shadow_data_");
  }

  ScopedMappedBuffer mapped{ssbo_shadow_data};
  auto* gpu_data{static_cast<GpuShadowData*>(ssbo_shadow_data.mapped_memory)};

  for (usize i{0}; i < shadow_count; ++i) {
    const auto& job{render_jobs_->Get(i)};
    const auto* resource{job.resource};

    gpu_data[i].view_proj = job.view_proj;

    const auto split_near{
        (resource->type == ShadowType::DirectionalOrtho &&
         job.view_proj_index > 0)
            ? resource->cascade_splits[job.view_proj_index - 1]
            : .0f};

    const auto split_far{(resource->type == ShadowType::DirectionalOrtho)
                             ? resource->cascade_splits[job.view_proj_index]
                             : resource->max_distance};

    gpu_data[i].cascade_data =
        math::Vec4{split_near, split_far,
                   static_cast<f32>(resource->first_layer_index +
                                    static_cast<s32>(job.view_proj_index)),
                   .0f};

    gpu_data[i].bias_data =
        math::Vec4{job.bias_constant, job.bias_slope, .0f, .0f};
  }
}

GpuLight LightingHandler::GenerateGpuLight(const LightProxy& light) const {
  const auto shadow_type{ResolveShadowType(light.props, light.shadow)};

  return {
      .position_type =
          math::Vec4{light.props.position.x, light.props.position.y,
                     light.props.position.z,
                     static_cast<f32>(light.props.type)},
      .direction_intensity =
          math::Vec4{light.props.direction.x, light.props.direction.y,
                     light.props.direction.z, light.props.intensity},
      .color_range = math::Vec4{light.props.color.x, light.props.color.y,
                                light.props.color.z, light.props.range},
      .shadow_header =
          math::Vec4{static_cast<f32>(shadow_type),
                     static_cast<f32>(light.gpu_shadow_index),
                     static_cast<f32>(light.shadow_entry_count), .0f},
      .spot_data = math::Vec4{light.props.inner_angle, light.props.outer_angle,
                              .0f, .0f},
  };
}

void LightingHandler::InitializeShadowArrayResources() {
  const auto depth_format{context_->GetDevice().ChooseDepthFormat()};
  const auto shadow_resolution{shadow_settings_->resolution};

  RuntimeTextureDescr texture_descr{};
  texture_descr.type = TextureType::Unknown;
  texture_descr.width = shadow_resolution;
  texture_descr.height = shadow_resolution;
  texture_descr.depth = 1;
  texture_descr.mip_levels = 1;
  texture_descr.channel_count = 1;
  texture_descr.format = depth_format;
  texture_descr.usage =
      VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
  texture_descr.aspect_flags = VK_IMAGE_ASPECT_DEPTH_BIT;
  texture_descr.view_type = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
  texture_descr.layer_count = kShadowLayerCapacity_;
  texture_descr.sample_count = VK_SAMPLE_COUNT_1_BIT;
  texture_descr.final_layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
#ifdef COMET_RENDERING_USE_DEBUG_LABELS
  texture_descr.debug_label = "shadow_array_depth_image";
#endif  // COMET_RENDERING_USE_DEBUG_LABELS

  const auto texture_handle{texture_handler_->Generate(texture_descr)};
  COMET_ASSERT(texture_handle,
               "LightingHandler::InitializeShadowArrayResources",
               "failed to generate shadow array texture");

  SamplerDescr sampler_descr{};
  sampler_descr.min_filter = VK_FILTER_LINEAR;
  sampler_descr.mag_filter = VK_FILTER_LINEAR;
  sampler_descr.mipmap_mode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
  sampler_descr.address_mode_u = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
  sampler_descr.address_mode_v = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
  sampler_descr.address_mode_w = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
  sampler_descr.compare_enable = true;
  sampler_descr.compare_op = VK_COMPARE_OP_LESS_OR_EQUAL;
  sampler_descr.min_lod = .0f;
  sampler_descr.max_lod = 1.0f;
  sampler_descr.border_color = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
  sampler_descr.unnormalized_coordinates = false;

  const auto sampler_handle{sampler_handler_->GetOrGenerate(sampler_descr)};
  COMET_ASSERT(sampler_handle,
               "LightingHandler::InitializeShadowArrayResources",
               "failed to generate shadow array sampler");

  shadow_array_texture_map_ = BuildTextureMap(
      sampler_handle, texture_handle, resource::TextureResourceId::Invalid(),
      TextureType::Unknown);
}

void LightingHandler::DestroyShadowArrayResources() {
  if (shadow_array_texture_map_.sampler_handle) {
    sampler_handler_->Destroy(shadow_array_texture_map_.sampler_handle);
  }

  if (shadow_array_texture_map_.texture_handle) {
    texture_handler_->Destroy(shadow_array_texture_map_.texture_handle);
  }

  shadow_array_texture_map_ = {};
}

s32 LightingHandler::AllocateShadowLayers(u32 layer_count) {
  if (layer_count == 0 || layer_count > kShadowLayerCapacity_) {
    return -1;
  }

  for (u32 i{0}; i + layer_count <= kShadowLayerCapacity_; ++i) {
    bool is_free{true};

    for (u32 j{0}; j < layer_count; ++j) {
      if (shadow_layer_usage_[i + j]) {
        is_free = false;
        break;
      }
    }

    if (!is_free) {
      continue;
    }

    for (u32 j{0}; j < layer_count; ++j) {
      shadow_layer_usage_[i + j] = true;
    }

    return static_cast<s32>(i);
  }

  COMET_LOG_ERROR(LoggerType::Rendering,
                  "LightingHandler::AllocateShadowLayers",
                  "no free shadow layers left", "layer_count", layer_count,
                  "capacity", kShadowLayerCapacity_);
  return -1;
}

void LightingHandler::FreeShadowLayers(s32 first_layer, u32 layer_count) {
  if (first_layer < 0) {
    return;
  }

  for (u32 i{0}; i < layer_count; ++i) {
    shadow_layer_usage_[static_cast<u32>(first_layer) + i] = false;
  }
}

void LightingHandler::InitializeShadowResource(LightHandle light_handle,
                                               const LightProxy& light,
                                               ShadowResource& resource) {
  COMET_ASSERT(render_pass_handle_, "LightingHandler::InitializeShadowResource",
               "shadow render pass is invalid");

  const auto* shadow_texture{
      GetTextureHandler()->Get(shadow_array_texture_map_.texture_handle)};
  COMET_ASSERT(shadow_texture != nullptr,
               "LightingHandler::InitializeShadowResource",
               "shadow array texture is null", "light_handle", light_handle);

  auto& device{context_->GetDevice()};
  const auto depth_format{device.ChooseDepthFormat()};

  resource = {};
  resource.is_dirty = true;
  resource.light_handle = light_handle;
  resource.type = ResolveShadowType(light.props, light.shadow);
  resource.resolution = shadow_settings_->resolution;
  resource.extent = {shadow_settings_->resolution,
                     shadow_settings_->resolution};
  resource.max_distance = light.shadow.max_distance;
  resource.bias_constant = light.shadow.bias_constant;
  resource.bias_slope = light.shadow.bias_slope;
  resource.gpu_shadow_index = -1;

  if (resource.type == ShadowType::DirectionalOrtho) {
    resource.view_proj_count = math::Min<u32>(
        math::Max<u32>(light.shadow.cascade_count, 1u), kMaxShadowCascades_);
  } else {
    resource.view_proj_count = 1;
  }

  resource.first_layer_index = AllocateShadowLayers(resource.view_proj_count);
  COMET_ASSERT(resource.first_layer_index >= 0,
               "LightingHandler::InitializeShadowResource",
               "failed to allocate shadow layers", "light_handle", light_handle,
               "view_proj_count", resource.view_proj_count);

  for (u32 i{0}; i < resource.view_proj_count; ++i) {
    resource.layer_image_views[i] =
        GenerateImageView(device, shadow_texture->image.handle, depth_format,
                          VK_IMAGE_ASPECT_DEPTH_BIT, 1,
                          static_cast<u32>(resource.first_layer_index) + i, 1,
                          VK_IMAGE_VIEW_TYPE_2D);

    VkFramebufferCreateInfo framebuffer_info{};
    framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebuffer_info.renderPass =
        render_pass_handler_->GetVkHandle(render_pass_handle_);
    framebuffer_info.attachmentCount = 1;
    framebuffer_info.pAttachments = &resource.layer_image_views[i];
    framebuffer_info.width = resource.extent.width;
    framebuffer_info.height = resource.extent.height;
    framebuffer_info.layers = 1;

    COMET_CHECK_VK(
        vkCreateFramebuffer(device, &framebuffer_info,
                            MemoryCallbacks::Get().GetAllocCallbacksHandle(),
                            &resource.framebuffers[i]),
        "LightingHandler::InitializeShadowResource",
        "failed to create shadow framebuffer", "light_handle", light_handle,
        "framebuffer_index", i);
  }
}

void LightingHandler::DestroyShadowResource(ShadowResource& resource) {
  const auto& device{context_->GetDevice()};

  for (u32 i{0}; i < resource.view_proj_count; ++i) {
    if (resource.framebuffers[i] != VK_NULL_HANDLE) {
      vkDestroyFramebuffer(device, resource.framebuffers[i],
                           MemoryCallbacks::Get().GetAllocCallbacksHandle());
      resource.framebuffers[i] = VK_NULL_HANDLE;
    }

    if (resource.layer_image_views[i] != VK_NULL_HANDLE) {
      vkDestroyImageView(device, resource.layer_image_views[i],
                         MemoryCallbacks::Get().GetAllocCallbacksHandle());
      resource.layer_image_views[i] = VK_NULL_HANDLE;
    }
  }

  FreeShadowLayers(resource.first_layer_index, resource.view_proj_count);
  resource = {};
}

void LightingHandler::RecreateShadowResourceIfNeeded(ShadowResource& resource,
                                                     const LightProxy& light) {
  bool is_resolution_changed{resource.resolution !=
                             shadow_settings_->resolution};
  const auto resolved_type{ResolveShadowType(light.props, light.shadow)};
  bool is_type_changed{resource.type != resolved_type};
  u32 desired_count{1};

  if (resolved_type == ShadowType::DirectionalOrtho) {
    desired_count = math::Min<u32>(
        math::Max<u32>(light.shadow.cascade_count, 1u), kMaxShadowCascades_);
  }

  bool is_count_changed{resource.view_proj_count != desired_count};

  if (!is_resolution_changed && !is_type_changed && !is_count_changed) {
    return;
  }

  const auto light_handle{resource.light_handle};
  DestroyShadowResource(resource);
  InitializeShadowResource(light_handle, light, resource);
}

ShadowResource* LightingHandler::TryGetShadowResource(
    LightHandle light_handle) noexcept {
  if (!light_handle) {
    return nullptr;
  }

  const auto index{static_cast<usize>(light_handle.GetIndex())};

  if (index >= shadow_resources_.GetSize()) {
    return nullptr;
  }

  auto& resource{shadow_resources_[index]};

  if (!resource.light_handle || resource.light_handle != light_handle) {
    return nullptr;
  }

  return &resource;
}

const ShadowResource* LightingHandler::TryGetShadowResource(
    LightHandle light_handle) const noexcept {
  if (!light_handle) {
    return nullptr;
  }

  const auto index{static_cast<usize>(light_handle.GetIndex())};

  if (index >= shadow_resources_.GetSize()) {
    return nullptr;
  }

  const auto& resource{shadow_resources_[index]};

  if (!resource.light_handle || resource.light_handle != light_handle) {
    return nullptr;
  }

  return &resource;
}

bool LightingHandler::IsLightSlotAlive(usize index) const noexcept {
  if (index >= proxies_.GetSize()) {
    return false;
  }

  const auto& proxy{proxies_[index]};
  return proxy.handle.IsValid();
}

bool LightingHandler::IsShadowSlotAlive(usize index) const noexcept {
  if (index >= shadow_resources_.GetSize()) {
    return false;
  }

  const auto& resource{shadow_resources_[index]};
  return resource.light_handle.IsValid();
}

void LightingHandler::PopulateCascadeSplits(const RenderCameraData& camera_data,
                                            f32 max_distance, u32 cascade_count,
                                            f32 lambda, f32* out_splits) const {
  COMET_ASSERT(out_splits != nullptr, "LightingHandler::PopulateCascadeSplits",
               "cascade split output is null");

  const auto near_plane{camera_data.near_plane};
  const auto far_plane{max_distance};
  const auto ratio{far_plane / near_plane};

  for (u32 i{0}; i < cascade_count; ++i) {
    const auto p{static_cast<f32>(i + 1) / static_cast<f32>(cascade_count)};
    const auto log_split{near_plane * math::Pow(ratio, p)};
    const auto uni_split{near_plane + (far_plane - near_plane) * p};
    out_splits[i] = lambda * log_split + (1.0f - lambda) * uni_split;
  }
}

math::Mat4 LightingHandler::ComputeDirectionalCascadeViewProj(
    const RenderCameraData& camera_data, const math::Vec3& light_dir,
    f32 cascade_near, f32 cascade_far) const {
  StaticArray<math::Vec3, 8> corners{};
  ComputeFrustumCorners(camera_data, cascade_near, cascade_far, corners);

  math::Vec3 center{.0f};

  for (const auto& p : corners) {
    center += p;
  }

  center /= corners.GetSize();
  math::Vec3 dir{math::GetNormalizedCopy(light_dir)};
  const auto light_up{ComputeStableUpVector(dir)};

  const auto cascade_depth{cascade_far - cascade_near};
  const auto caster_extrusion{cascade_depth *
                              shadow_settings_->caster_extrusion_factor};

  const auto light_distance{cascade_far + caster_extrusion};
  const auto light_position{center - dir * light_distance};
  const auto light_view{LookAt(light_position, center, light_up)};

  auto min_x{kF32Max};
  auto max_x{-kF32Max};
  auto min_y{kF32Max};
  auto max_y{-kF32Max};
  auto min_z{kF32Max};
  auto max_z{-kF32Max};

  const auto accumulate_ls_point{[&](const math::Vec3& world_p) {
    const auto ls4{light_view * math::Vec4{world_p, 1.0f}};

    min_x = math::Min(min_x, ls4.x);
    max_x = math::Max(max_x, ls4.x);
    min_y = math::Min(min_y, ls4.y);
    max_y = math::Max(max_y, ls4.y);
    min_z = math::Min(min_z, ls4.z);
    max_z = math::Max(max_z, ls4.z);
  }};

  for (const auto& p : corners) {
    accumulate_ls_point(p);
    accumulate_ls_point(p - dir * caster_extrusion);
  }

  min_x -= shadow_settings_->receiver_pad_xy;
  max_x += shadow_settings_->receiver_pad_xy;
  min_y -= shadow_settings_->receiver_pad_xy;
  max_y += shadow_settings_->receiver_pad_xy;
  min_z -= shadow_settings_->receiver_pad_z;
  max_z += shadow_settings_->receiver_pad_z;

  return GenerateOrthographicMatrix(min_x, max_x, min_y, max_y, -max_z, -min_z,
                                    ClipSpaceDepthRange::ZeroToOne) *
         light_view;
}

math::Mat4 LightingHandler::ComputeSpotLightViewProj(
    const LightProperties& props, f32 max_distance) const {
  const auto dir{math::GetNormalizedCopy(props.direction)};
  const auto up{ComputeStableUpVector(dir)};
  const auto view{LookAt(props.position, props.position + dir, up)};

  const auto fov_y{props.outer_angle * 2.0f};
  const auto aspect{1.0f};
  const auto near_plane{.1f};

  const auto proj{GeneratePerspectiveMatrix(
      fov_y, aspect, near_plane, max_distance, ClipSpaceDepthRange::ZeroToOne)};

  return proj * view;
}

const TextureHandler* LightingHandler::GetTextureHandler() const {
  return texture_handler_;
}
}  // namespace vk
}  // namespace rendering
}  // namespace comet