// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_render_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/render/driver/opengl/handler/opengl_lighting_handler.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/logger/logging.h"
#include "comet/runtime/profiler/profiler.h"
#include "comet/render/driver/opengl/opengl_debug.h"
#include "comet/render/driver/opengl/utils/opengl_texture_map_utils.h"
#include "comet/runtime/camera/camera.h"
#include "comet/render/common.h"
#include "comet/data/light/light.h"
#include "comet/data/render/texture.h"
#include "comet/runtime/camera/camera_utils.h"
#include "comet/render/utils/culling_utils.h"
#include "comet/data/render/utils/light_utils.h"

namespace comet {
namespace render {
namespace gl {
LightingHandler::LightingHandler(const LightingHandlerDescr& descr)
    : Handler{descr},
      shadow_settings_{descr.shadow_settings},
      texture_handler_{descr.texture_handler},
      sampler_handler_{descr.sampler_handler} {
  COMET_ASSERT(shadow_settings_ != nullptr, "LightingHandler::LightingHandler",
               "shadow settings are null");
  COMET_ASSERT(texture_handler_ != nullptr, "LightingHandler::LightingHandler",
               "texture handler is null");
  COMET_ASSERT(sampler_handler_ != nullptr, "LightingHandler::LightingHandler",
               "sampler handler is null");
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

  const auto frame_index{frame_state_->GetFrameInFlightIndex()};
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
  return {ssbo_lights_[frame_index], ssbo_lights_size_[frame_index]};
}

ShadowGpuData LightingHandler::GetShadowGpuData(
    FrameInFlightIndex frame_index) const noexcept {
  return {ssbo_shadow_data_[frame_index], ssbo_shadow_data_size_[frame_index]};
}

const frame::FrameArray<ShadowRenderJob>* LightingHandler::GetRenderJobs()
    const noexcept {
  return render_jobs_;
}

const TextureMap* LightingHandler::GetShadowArrayTextureMap() const noexcept {
  return shadow_array_texture_map_.texture_handle ? &shadow_array_texture_map_
                                                  : nullptr;
}

GlNativeTextureHandle LightingHandler::GetShadowArrayTextureHandle()
    const noexcept {
  if (!shadow_array_texture_map_.texture_handle) {
    return kInvalidGlNativeTextureHandle;
  }

  const auto* texture{
      GetTextureHandler()->Get(shadow_array_texture_map_.texture_handle)};
  return texture != nullptr ? texture->native_handle
                            : kInvalidGlNativeTextureHandle;
}

GLenum LightingHandler::GetShadowArrayFormat() const noexcept {
  if (!shadow_array_texture_map_.texture_handle) {
    return GL_INVALID_ENUM;
  }

  const auto* texture{
      GetTextureHandler()->Get(shadow_array_texture_map_.texture_handle)};
  return texture != nullptr ? texture->internal_format : GL_INVALID_ENUM;
}

void LightingHandler::OnInitialize() {
  allocator_.Initialize();

  proxies_ = Array<LightProxy>::WithCapacity(&allocator_, kDefaultLightCount_);
  shadow_resources_ =
      Array<ShadowResource>::WithCapacity(&allocator_, kDefaultLightCount_);

  shadow_layer_usage_ = Array<bool>{&allocator_};
  shadow_layer_usage_.Resize(kMaxShadowLayerCount);

  for (u32 i{0}; i < kMaxShadowLayerCount; ++i) {
    shadow_layer_usage_[i] = false;
  }

  const auto max_frames_in_flight{frame_state_->GetMaxFramesInFlight()};

  ssbo_lights_ = Array<GlNativeStorageHandle>{&platform_allocator_};
  ssbo_shadow_data_ = Array<GlNativeStorageHandle>{&platform_allocator_};
  ssbo_lights_size_ = Array<GLsizeiptr>{&platform_allocator_};
  ssbo_shadow_data_size_ = Array<GLsizeiptr>{&platform_allocator_};

  ssbo_lights_.Resize(max_frames_in_flight);
  ssbo_shadow_data_.Resize(max_frames_in_flight);
  ssbo_lights_size_.Resize(max_frames_in_flight);
  ssbo_shadow_data_size_.Resize(max_frames_in_flight);

  const auto initial_light_buffer_size{
      static_cast<GLsizeiptr>(kDefaultLightCount_ * sizeof(GpuLight))};

  const auto initial_shadow_buffer_size{
      static_cast<GLsizeiptr>(kMaxShadowLayerCount * sizeof(GpuShadowData))};

  for (FrameInFlightIndex i{0}; i < max_frames_in_flight; ++i) {
    ssbo_lights_[i] = kInvalidGlNativeStorageHandle;
    ssbo_shadow_data_[i] = kInvalidGlNativeStorageHandle;
    ssbo_lights_size_[i] = 0;
    ssbo_shadow_data_size_[i] = 0;

    glCreateBuffers(1, &ssbo_lights_[i]);
    COMET_ASSERT(ssbo_lights_[i] != kInvalidGlNativeStorageHandle,
                 "LightingHandler::OnInitialize", "failed to create light ssbo",
                 "frame_index", i);

    glNamedBufferData(ssbo_lights_[i], initial_light_buffer_size, nullptr,
                      GL_DYNAMIC_DRAW);
#ifdef COMET_RENDERING_USE_DEBUG_LABELS
    COMET_GL_SET_STORAGE_DEBUG_LABEL(ssbo_lights_[i], "ssbo_lights_");
#endif  // COMET_RENDERING_USE_DEBUG_LABELS
    ssbo_lights_size_[i] = initial_light_buffer_size;

    glCreateBuffers(1, &ssbo_shadow_data_[i]);
    COMET_ASSERT(ssbo_shadow_data_[i] != kInvalidGlNativeStorageHandle,
                 "LightingHandler::OnInitialize",
                 "failed to create shadow ssbo", "frame_index", i);

    glNamedBufferData(ssbo_shadow_data_[i], initial_shadow_buffer_size, nullptr,
                      GL_DYNAMIC_DRAW);
#ifdef COMET_RENDERING_USE_DEBUG_LABELS
    COMET_GL_SET_STORAGE_DEBUG_LABEL(ssbo_shadow_data_[i], "ssbo_shadow_data_");
#endif  // COMET_RENDERING_USE_DEBUG_LABELS
    ssbo_shadow_data_size_[i] = initial_shadow_buffer_size;
  }

  glBindBuffer(GL_SHADER_STORAGE_BUFFER, kInvalidGlNativeStorageHandle);

  InitializeShadowArrayResources();
}

void LightingHandler::OnShutdown() {
  for (auto& resource : shadow_resources_) {
    DestroyShadowResource(resource);
  }

  shadow_resources_.Release();

  DestroyShadowArrayResources();

  for (usize i{0}; i < ssbo_shadow_data_.GetSize(); ++i) {
    if (ssbo_shadow_data_[i] != kInvalidGlNativeStorageHandle) {
      glDeleteBuffers(1, &ssbo_shadow_data_[i]);
      ssbo_shadow_data_[i] = kInvalidGlNativeStorageHandle;
      ssbo_shadow_data_size_[i] = 0;
    }

    if (ssbo_lights_[i] != kInvalidGlNativeStorageHandle) {
      glDeleteBuffers(1, &ssbo_lights_[i]);
      ssbo_lights_[i] = kInvalidGlNativeStorageHandle;
      ssbo_lights_size_[i] = 0;
    }
  }

  ssbo_shadow_data_.Release();
  ssbo_lights_.Release();
  ssbo_shadow_data_size_.Release();
  ssbo_lights_size_.Release();

  shadow_layer_usage_.Release();
  proxies_.Release();

  allocator_.Destroy();
  render_jobs_ = nullptr;
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

  const auto* camera_data{packet->GetMainCameraData()};
  COMET_ASSERT(camera_data != nullptr, "LightingHandler::RebuildRenderJobs",
               "main camera data is null");

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

      f32 cascade_near{camera_data->near_plane};

      for (u32 j{0}; j < resource.view_proj_count; ++j) {
        const f32 cascade_far{resource.cascade_splits[j]};

        StaticArray<math::Vec3, 8> corners{};
        ComputeFrustumCorners(*camera_data, cascade_near, cascade_far, corners);

        resource.view_proj[j] = ComputeDirectionalCascadeViewProj(
            light->props.direction, corners, cascade_near, cascade_far);

        ShadowRenderJob job{};
        job.light_handle = resource.light_handle;
        job.type = resource.type;
        job.resolution = resource.resolution;
        job.bias_constant = resource.bias_constant;
        job.bias_slope = resource.bias_slope;
        job.resource = &resource;
        job.view_proj_index = j;
        job.view_proj = resource.view_proj[j];
        job.framebuffer = resource.framebuffers[j];

#ifdef COMET_DEBUG_RENDERING
        job.cascade_corners = corners;
#endif  // COMET_DEBUG_RENDERING

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
      job.resolution = resource.resolution;
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

  const auto required_size{
      static_cast<GLsizeiptr>(live_light_count * sizeof(GpuLight))};

  EnsureStorageBufferCapacity(
      ssbo_lights_[frame_index], ssbo_lights_size_[frame_index],
      math::Max<GLsizeiptr>(required_size, sizeof(GpuLight)));

  glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_lights_[frame_index]);

  auto* gpu_lights{static_cast<GpuLight*>(
      glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, required_size,
                       GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT))};

  COMET_ASSERT(gpu_lights != nullptr, "LightingHandler::UploadGpuLights",
               "failed to map light ssbo", "frame_index", frame_index,
               "required_size", required_size);

  usize gpu_index{0};

  for (usize i{0}; i < proxies_.GetSize(); ++i) {
    if (!IsLightSlotAlive(i)) {
      continue;
    }

    gpu_lights[gpu_index++] = GenerateGpuLight(proxies_[i]);
    proxies_[i].is_dirty = false;
  }

  glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, kInvalidGlNativeStorageHandle);
}

void LightingHandler::UploadGpuShadowData(FrameInFlightIndex frame_index) {
  const auto shadow_count{render_jobs_ != nullptr ? render_jobs_->GetSize()
                                                  : 0};

  if (shadow_count == 0) {
    return;
  }

  const auto required_size{
      static_cast<GLsizeiptr>(shadow_count * sizeof(GpuShadowData))};

  EnsureStorageBufferCapacity(
      ssbo_shadow_data_[frame_index], ssbo_shadow_data_size_[frame_index],
      math::Max<GLsizeiptr>(required_size,
                            kMaxShadowLayerCount * sizeof(GpuShadowData)));

  glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_shadow_data_[frame_index]);

  auto* gpu_data{static_cast<GpuShadowData*>(
      glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, required_size,
                       GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT))};

  COMET_ASSERT(gpu_data != nullptr, "LightingHandler::UploadGpuShadowData",
               "failed to map shadow ssbo", "frame_index", frame_index,
               "required_size", required_size);

  for (usize i{0}; i < shadow_count; ++i) {
    const auto& job{render_jobs_->Get(i)};
    const auto* resource{job.resource};

    gpu_data[i].view_proj = job.view_proj;

    const auto split_near{
        (resource->type == ShadowType::DirectionalOrtho &&
         job.view_proj_index > 0)
            ? resource->cascade_splits[job.view_proj_index - 1]
            : .0f};

    const auto split_far{resource->type == ShadowType::DirectionalOrtho
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

  glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, kInvalidGlNativeStorageHandle);
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
  RuntimeTextureDescr texture_descr{};
  texture_descr.type = TextureType::Unknown;
  texture_descr.target = GL_TEXTURE_2D_ARRAY;
  texture_descr.width = shadow_settings_->resolution;
  texture_descr.height = shadow_settings_->resolution;
  texture_descr.depth = kMaxShadowLayerCount;
  texture_descr.mip_levels = 1;
  texture_descr.channel_count = 1;
  texture_descr.format = GL_DEPTH_COMPONENT;
  texture_descr.internal_format = GL_DEPTH_COMPONENT32F;

  const auto texture_handle{
      texture_handler_->GenerateRuntimeImmediate(texture_descr)};
  COMET_ASSERT(texture_handle,
               "LightingHandler::InitializeShadowArrayResources",
               "failed to generate shadow array texture");

  SamplerDescr sampler_descr{};
  sampler_descr.wrap_s = GL_CLAMP_TO_BORDER;
  sampler_descr.wrap_t = GL_CLAMP_TO_BORDER;
  sampler_descr.wrap_r = GL_CLAMP_TO_BORDER;
  sampler_descr.min_filter = GL_LINEAR;
  sampler_descr.mag_filter = GL_LINEAR;
  sampler_descr.compare_mode = GL_COMPARE_REF_TO_TEXTURE;
  sampler_descr.compare_func = GL_LEQUAL;
  sampler_descr.border_color = math::Vec4{1.0f, 1.0f, 1.0f, 1.0f};
  sampler_descr.use_border_color = true;

  const auto sampler_handle{sampler_handler_->GetOrGenerate(sampler_descr)};
  COMET_ASSERT(sampler_handle,
               "LightingHandler::InitializeShadowArrayResources",
               "failed to generate shadow array sampler");

  shadow_array_texture_map_ = BuildTextureMap(
      sampler_handle, texture_handle, resource::TextureResourceId::Invalid(),
      TextureType::Unknown);

  const auto* texture{GetTextureHandler()->Get(texture_handle)};
  COMET_ASSERT(texture != nullptr,
               "LightingHandler::InitializeShadowArrayResources",
               "shadow array texture is null");

  glBindTexture(GL_TEXTURE_2D_ARRAY, texture->native_handle);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BASE_LEVEL, 0);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_LEVEL, 0);
  glBindTexture(GL_TEXTURE_2D_ARRAY, kInvalidGlNativeTextureHandle);
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
  if (layer_count == 0 || layer_count > kMaxShadowLayerCount) {
    return -1;
  }

  for (u32 i{0}; i + layer_count <= kMaxShadowLayerCount; ++i) {
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
                  "capacity", kMaxShadowLayerCount);
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
  resource = {};
  resource.is_dirty = true;
  resource.light_handle = light_handle;
  resource.type = ResolveShadowType(light.props, light.shadow);
  resource.resolution = shadow_settings_->resolution;
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
    glGenFramebuffers(1, &resource.framebuffers[i]);
    COMET_ASSERT(resource.framebuffers[i] != kInvalidFrameBufferHandle,
                 "LightingHandler::InitializeShadowResource",
                 "failed to create shadow framebuffer", "light_handle",
                 light_handle, "framebuffer_index", i);

    glBindFramebuffer(GL_FRAMEBUFFER, resource.framebuffers[i]);

    const auto* shadow_texture{
        GetTextureHandler()->Get(shadow_array_texture_map_.texture_handle)};
    COMET_ASSERT(shadow_texture != nullptr,
                 "LightingHandler::InitializeShadowResource",
                 "shadow array texture is null", "light_handle", light_handle);

    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                              shadow_texture->native_handle, 0,
                              resource.first_layer_index + i);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    COMET_FASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER),
                  GL_FRAMEBUFFER_COMPLETE,
                  "LightingHandler::InitializeShadowResource",
                  "shadow framebuffer is incomplete", "light_handle",
                  light_handle, "framebuffer_index", i);
  }

  glBindFramebuffer(GL_FRAMEBUFFER, kInvalidFrameBufferHandle);
}

void LightingHandler::DestroyShadowResource(ShadowResource& resource) {
  for (u32 i{0}; i < resource.view_proj_count; ++i) {
    if (resource.framebuffers[i] != kInvalidFrameBufferHandle) {
      glDeleteFramebuffers(1, &resource.framebuffers[i]);
      resource.framebuffers[i] = kInvalidFrameBufferHandle;
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

void LightingHandler::PopulateCascadeSplits(const CameraViewData* camera_data,
                                            f32 max_distance, u32 cascade_count,
                                            f32 lambda, f32* out_splits) const {
  COMET_ASSERT(out_splits != nullptr, "LightingHandler::PopulateCascadeSplits",
               "cascade split output is null");
  COMET_ASSERT(camera_data != nullptr, "LightingHandler::PopulateCascadeSplits",
               "main camera data is null");

  const auto near_plane{camera_data->near_plane};
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
    const math::Vec3& light_dir, const StaticArray<math::Vec3, 8>& corners,
    f32 cascade_near, f32 cascade_far) const {
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

void LightingHandler::EnsureStorageBufferCapacity(GlNativeStorageHandle& handle,
                                                  GLsizeiptr& capacity,
                                                  GLsizeiptr required_size) {
  COMET_ASSERT(handle != kInvalidGlNativeStorageHandle,
               "LightingHandler::EnsureStorageBufferCapacity",
               "storage buffer handle is invalid");

  if (required_size <= capacity) {
    return;
  }

  glBindBuffer(GL_SHADER_STORAGE_BUFFER, handle);
  glBufferData(GL_SHADER_STORAGE_BUFFER, required_size, nullptr,
               GL_DYNAMIC_DRAW);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, kInvalidGlNativeStorageHandle);
  capacity = required_size;
}

const TextureHandler* LightingHandler::GetTextureHandler() const {
  return texture_handler_;
}
}  // namespace gl
}  // namespace render
}  // namespace comet