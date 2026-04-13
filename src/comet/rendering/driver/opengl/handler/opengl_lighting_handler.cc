// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "opengl_lighting_handler.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/logger.h"
#include "comet/profiler/profiler.h"
#include "comet/rendering/culling/culling_utils.h"
#include "comet/rendering/driver/opengl/opengl_debug.h"
#include "comet/rendering/rendering_utils.h"

namespace comet {
namespace rendering {
namespace gl {
LightingHandler::LightingHandler(const LightingHandlerDescr& descr)
    : Handler{descr}, shadow_settings_{descr.shadow_settings} {
  COMET_ASSERT(shadow_settings_ != nullptr, "Shadow settings are null!");
}

void LightingHandler::Initialize() {
  Handler::Initialize();
  allocator_.Initialize();

  light_to_proxy_map_ = Map<LightId, usize>{&allocator_, kDefaultLightCount_};
  proxies_ = Array<LightProxy>{&allocator_, kDefaultLightCount_};

  light_to_shadow_map_ = Map<LightId, usize>{&allocator_, kDefaultLightCount_};
  shadow_resources_ = Array<ShadowResource>{&allocator_, kDefaultLightCount_};

  shadow_layer_usage_ = Array<bool>{&allocator_};
  shadow_layer_usage_.Resize(kShadowLayerCapacity_);

  for (u32 i{0}; i < kShadowLayerCapacity_; ++i) {
    shadow_layer_usage_[i] = false;
  }

  auto max_frames_in_flight{frame_state_->GetMaxFramesInFlight()};

  ssbo_lights_ = Array<StorageHandle>{&platform_allocator_};
  ssbo_shadow_data_ = Array<StorageHandle>{&platform_allocator_};
  ssbo_lights_size_ = Array<GLsizeiptr>{&platform_allocator_};
  ssbo_shadow_data_size_ = Array<GLsizeiptr>{&platform_allocator_};

  ssbo_lights_.Resize(max_frames_in_flight);
  ssbo_shadow_data_.Resize(max_frames_in_flight);
  ssbo_lights_size_.Resize(max_frames_in_flight);
  ssbo_shadow_data_size_.Resize(max_frames_in_flight);

  auto initial_light_buffer_size{
      static_cast<GLsizeiptr>(kDefaultLightCount_ * sizeof(GpuLight))};

  auto initial_shadow_buffer_size{
      static_cast<GLsizeiptr>(kShadowLayerCapacity_ * sizeof(GpuShadowData))};

  for (FrameInFlightIndex i{0}; i < max_frames_in_flight; ++i) {
    ssbo_lights_[i] = kInvalidStorageHandle;
    ssbo_shadow_data_[i] = kInvalidStorageHandle;
    ssbo_lights_size_[i] = 0;
    ssbo_shadow_data_size_[i] = 0;

    glCreateBuffers(1, &ssbo_lights_[i]);
    COMET_ASSERT(ssbo_lights_[i] != kInvalidStorageHandle,
                 "Failed to create light SSBO!");

    glNamedBufferData(ssbo_lights_[i], initial_light_buffer_size, nullptr,
                      GL_DYNAMIC_DRAW);
#ifdef COMET_RENDERING_USE_DEBUG_LABELS
    COMET_GL_SET_STORAGE_DEBUG_LABEL(ssbo_lights_[i], "ssbo_lights_");
#endif
    ssbo_lights_size_[i] = initial_light_buffer_size;

    glCreateBuffers(1, &ssbo_shadow_data_[i]);
    COMET_ASSERT(ssbo_shadow_data_[i] != kInvalidStorageHandle,
                 "Failed to create shadow SSBO!");

    glNamedBufferData(ssbo_shadow_data_[i], initial_shadow_buffer_size, nullptr,
                      GL_DYNAMIC_DRAW);
#ifdef COMET_RENDERING_USE_DEBUG_LABELS
    COMET_GL_SET_STORAGE_DEBUG_LABEL(ssbo_shadow_data_[i], "ssbo_shadow_data_");
#endif
    ssbo_shadow_data_size_[i] = initial_shadow_buffer_size;
  }

  glBindBuffer(GL_SHADER_STORAGE_BUFFER, kInvalidStorageHandle);

  InitializeShadowArrayResources();
}

void LightingHandler::Shutdown() {
  for (auto& resource : shadow_resources_) {
    DestroyShadowResource(resource);
  }

  shadow_resources_.Destroy();
  light_to_shadow_map_.Destroy();

  DestroyShadowArrayResources();

  for (usize i{0}; i < ssbo_shadow_data_.GetSize(); ++i) {
    if (ssbo_shadow_data_[i] != kInvalidStorageHandle) {
      glDeleteBuffers(1, &ssbo_shadow_data_[i]);
      ssbo_shadow_data_[i] = kInvalidStorageHandle;
      ssbo_shadow_data_size_[i] = 0;
    }

    if (ssbo_lights_[i] != kInvalidStorageHandle) {
      glDeleteBuffers(1, &ssbo_lights_[i]);
      ssbo_lights_[i] = kInvalidStorageHandle;
      ssbo_lights_size_[i] = 0;
    }
  }

  ssbo_shadow_data_.Destroy();
  ssbo_lights_.Destroy();
  ssbo_shadow_data_size_.Destroy();
  ssbo_lights_size_.Destroy();

  shadow_layer_usage_.Destroy();
  proxies_.Destroy();
  light_to_proxy_map_.Destroy();

  allocator_.Destroy();
  render_jobs_ = nullptr;

  Handler::Shutdown();
}

void LightingHandler::Update(const frame::FramePacket* packet) {
  COMET_PROFILE("LightingHandler::Update");
  COMET_ASSERT(packet != nullptr, "Frame packet is null!");

  if (!packet->added_lights->IsEmpty()) {
    AddLights(packet->added_lights);
  }

  if (!packet->dirty_lights->IsEmpty()) {
    UpdateLights(packet->dirty_lights);
  }

  if (!packet->removed_lights->IsEmpty()) {
    RemoveLights(packet->removed_lights);
  }

  auto frame_index{frame_state_->GetFrameInFlightIndex()};
  RebuildRenderJobs(packet);
  UploadGpuShadowData(frame_index);
  UploadGpuLights(frame_index);
}

const LightProxy* LightingHandler::Get(LightProxyHandle handle) const {
  return TryGetLight(static_cast<LightId>(handle));
}

const LightProxy* LightingHandler::TryGetLight(
    LightId light_id) const noexcept {
  auto* index{light_to_proxy_map_.TryGet(light_id)};
  return index != nullptr ? &proxies_[*index] : nullptr;
}

u32 LightingHandler::GetLightCount() const noexcept {
  return static_cast<u32>(proxies_.GetSize());
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
  return shadow_array_texture_map_.texture != nullptr
             ? &shadow_array_texture_map_
             : nullptr;
}

TextureHandle LightingHandler::GetShadowArrayTextureHandle() const noexcept {
  return shadow_array_texture_handle_;
}

GLenum LightingHandler::GetShadowArrayFormat() const noexcept {
  return shadow_array_texture_.internal_format;
}

void LightingHandler::AddLights(const frame::AddedLights* lights) {
  for (const auto& payload : *lights) {
    if (light_to_proxy_map_.IsContained(payload.light_id)) {
      continue;
    }

    auto proxy_index{proxies_.GetSize()};
    light_to_proxy_map_[payload.light_id] = proxy_index;

    auto& proxy{proxies_.EmplaceBack()};
    proxy.id = payload.light_id;
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
    auto* proxy_index{light_to_proxy_map_.TryGet(payload.light_id)};

    if (proxy_index == nullptr) {
      continue;
    }

    auto& proxy{proxies_[*proxy_index]};
    proxy.props = payload.props;
    proxy.shadow = payload.shadow;
    proxy.is_dirty = true;

    UpdateShadowForLight(proxy);
  }
}

void LightingHandler::RemoveLights(const frame::RemovedLights* lights) {
  for (const auto& payload : *lights) {
    RemoveShadowForLight(payload.light_id);

    auto* proxy_index{light_to_proxy_map_.TryGet(payload.light_id)};
    if (proxy_index == nullptr) {
      continue;
    }

    auto index{*proxy_index};
    auto last_index{proxies_.GetSize() - 1};

    if (index != last_index) {
      proxies_[index] = proxies_[last_index];
      light_to_proxy_map_[proxies_[index].id] = index;
    }

    proxies_.PopBack();
    light_to_proxy_map_.Remove(payload.light_id);
  }
}

void LightingHandler::AddShadowForLight(const LightProxy& light) {
  auto shadow_type{ResolveShadowType(light.props, light.shadow)};

  if (shadow_type == ShadowType::None) {
    return;
  }

  if (!IsShadowSupportedForLight(light.props, light.shadow)) {
    if (shadow_type == ShadowType::PointCubemap) {
      COMET_LOG_RENDERING_WARNING(
          "Point cubemap shadows are not implemented yet. Ignoring shadow "
          "setup for light ",
          light.id, ".");
    }

    return;
  }

  if (light_to_shadow_map_.IsContained(light.id)) {
    return;
  }

  auto& resource{shadow_resources_.EmplaceBack()};
  InitializeShadowResource(light.id, light, resource);
  light_to_shadow_map_[light.id] = shadow_resources_.GetSize() - 1;
}

void LightingHandler::UpdateShadowForLight(const LightProxy& light) {
  auto shadow_type{ResolveShadowType(light.props, light.shadow)};
  auto is_supported{IsShadowSupportedForLight(light.props, light.shadow)};
  auto* existing{TryGetShadowResource(light.id)};

  if (shadow_type == ShadowType::None || !is_supported) {
    if (existing != nullptr) {
      RemoveShadowForLight(light.id);
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

void LightingHandler::RemoveShadowForLight(LightId light_id) {
  auto* proxy_index{light_to_shadow_map_.TryGet(light_id)};
  if (proxy_index == nullptr) {
    return;
  }

  auto index{*proxy_index};
  auto last_index{shadow_resources_.GetSize() - 1};

  DestroyShadowResource(shadow_resources_[index]);

  if (index != last_index) {
    shadow_resources_[index] = shadow_resources_[last_index];
    light_to_shadow_map_[shadow_resources_[index].light_id] = index;
  }

  shadow_resources_.PopBack();
  light_to_shadow_map_.Remove(light_id);
}

void LightingHandler::RebuildRenderJobs(const frame::FramePacket* packet) {
  COMET_PROFILE("LightingHandler::RebuildRenderJobs");

  render_jobs_ = COMET_FRAME_ARRAY(
      ShadowRenderJob, shadow_resources_.GetSize() * kMaxShadowCascades_);

  for (auto& proxy : proxies_) {
    proxy.gpu_shadow_index = -1;
    proxy.shadow_entry_count = 0;
  }

  for (usize i{0}; i < shadow_resources_.GetSize(); ++i) {
    auto& resource{shadow_resources_[i]};
    auto* light{TryGetLight(resource.light_id)};

    if (light == nullptr) {
      continue;
    }

    auto first_shadow_index{static_cast<s32>(render_jobs_->GetSize())};
    resource.gpu_shadow_index = first_shadow_index;

    if (auto* proxy_index{light_to_proxy_map_.TryGet(resource.light_id)};
        proxy_index != nullptr) {
      proxies_[*proxy_index].gpu_shadow_index = first_shadow_index;
      proxies_[*proxy_index].shadow_entry_count = resource.view_proj_count;
      proxies_[*proxy_index].is_dirty = true;
    }

    if (light->props.type == LightType::Directional &&
        resource.type == ShadowType::DirectionalOrtho) {
      PopulateCascadeSplits(
          packet->camera_data, resource.max_distance, resource.view_proj_count,
          light->shadow.cascade_lambda, resource.cascade_splits);

      auto cascade_near{packet->camera_data.near_plane};

      for (u32 j{0}; j < resource.view_proj_count; ++j) {
        auto cascade_far{resource.cascade_splits[j]};

        resource.view_proj[j] = ComputeDirectionalCascadeViewProj(
            packet->camera_data, light->props.direction, cascade_near,
            cascade_far);

        ShadowRenderJob job{};
        job.light_id = resource.light_id;
        job.type = resource.type;
        job.resolution = resource.resolution;
        job.bias_constant = resource.bias_constant;
        job.bias_slope = resource.bias_slope;
        job.resource = &resource;
        job.view_proj_index = j;
        job.view_proj = resource.view_proj[j];
        job.framebuffer = resource.framebuffers[j];
        render_jobs_->PushBack(job);

        cascade_near = cascade_far;
      }
    } else if (light->props.type == LightType::Spot &&
               resource.type == ShadowType::SpotPerspective) {
      resource.cascade_splits[0] = resource.max_distance;
      resource.view_proj[0] =
          ComputeSpotLightViewProj(light->props, resource.max_distance);

      ShadowRenderJob job{};
      job.light_id = resource.light_id;
      job.type = resource.type;
      job.resolution = resource.resolution;
      job.bias_constant = resource.bias_constant;
      job.bias_slope = resource.bias_slope;
      job.resource = &resource;
      job.view_proj_index = 0;
      job.view_proj = resource.view_proj[0];
      job.framebuffer = resource.framebuffers[0];
      render_jobs_->PushBack(job);
    }
  }
}

void LightingHandler::UploadGpuLights(FrameInFlightIndex frame_index) {
  auto light_count{proxies_.GetSize()};
  if (light_count == 0) {
    return;
  }

  auto required_size{static_cast<GLsizeiptr>(light_count * sizeof(GpuLight))};

  EnsureStorageBufferCapacity(ssbo_lights_[frame_index],
                              ssbo_lights_size_[frame_index], required_size);

  glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_lights_[frame_index]);

  auto* gpu_lights{static_cast<GpuLight*>(
      glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, required_size,
                       GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT))};

  COMET_ASSERT(gpu_lights != nullptr, "Failed to map light SSBO!");

  for (usize i{0}; i < light_count; ++i) {
    gpu_lights[i] = GenerateGpuLight(proxies_[i]);
    proxies_[i].is_dirty = false;
  }

  glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, kInvalidStorageHandle);
}

void LightingHandler::UploadGpuShadowData(FrameInFlightIndex frame_index) {
  auto shadow_count{render_jobs_ != nullptr ? render_jobs_->GetSize() : 0};
  if (shadow_count == 0) {
    return;
  }

  auto required_size{
      static_cast<GLsizeiptr>(shadow_count * sizeof(GpuShadowData))};

  EnsureStorageBufferCapacity(ssbo_shadow_data_[frame_index],
                              ssbo_shadow_data_size_[frame_index],
                              required_size);

  glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_shadow_data_[frame_index]);

  auto* gpu_data{static_cast<GpuShadowData*>(
      glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, required_size,
                       GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT))};

  COMET_ASSERT(gpu_data != nullptr, "Failed to map shadow SSBO!");

  for (usize i{0}; i < shadow_count; ++i) {
    const auto& job{render_jobs_->Get(i)};
    const auto* resource{job.resource};

    gpu_data[i].view_proj = job.view_proj;

    auto split_near{(resource->type == ShadowType::DirectionalOrtho &&
                     job.view_proj_index > 0)
                        ? resource->cascade_splits[job.view_proj_index - 1]
                        : .0f};

    auto split_far{resource->type == ShadowType::DirectionalOrtho
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
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, kInvalidStorageHandle);
}

GpuLight LightingHandler::GenerateGpuLight(const LightProxy& light) const {
  auto shadow_type{ResolveShadowType(light.props, light.shadow)};

  return {
      math::Vec4{light.props.position.x, light.props.position.y,
                 light.props.position.z, static_cast<f32>(light.props.type)},
      math::Vec4{light.props.direction.x, light.props.direction.y,
                 light.props.direction.z, light.props.intensity},
      math::Vec4{light.props.color.x, light.props.color.y, light.props.color.z,
                 light.props.range},
      math::Vec4{static_cast<f32>(shadow_type),
                 static_cast<f32>(light.gpu_shadow_index),
                 static_cast<f32>(light.shadow_entry_count), .0f},
      math::Vec4{light.props.inner_angle, light.props.outer_angle, .0f, .0f},
  };
}

void LightingHandler::InitializeShadowArrayResources() {
  auto resolution{static_cast<GLsizei>(shadow_settings_->resolution)};

  glGenTextures(1, &shadow_array_texture_handle_);
  COMET_ASSERT(shadow_array_texture_handle_ != kInvalidTextureHandle,
               "Failed to create shadow array texture!");

  glBindTexture(GL_TEXTURE_2D_ARRAY, shadow_array_texture_handle_);
  glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT32F, resolution,
               resolution, static_cast<GLsizei>(kShadowLayerCapacity_), 0,
               GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BASE_LEVEL, 0);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_LEVEL, 0);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_MODE,
                  GL_COMPARE_REF_TO_TEXTURE);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);

  GLfloat border_color[4]{1.0f, 1.0f, 1.0f, 1.0f};
  glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, border_color);

  glBindTexture(GL_TEXTURE_2D_ARRAY, kInvalidTextureHandle);

  glGenSamplers(1, &shadow_array_sampler_wrapper_.handle);
  COMET_ASSERT(shadow_array_sampler_wrapper_.handle != 0,
               "Failed to create shadow array sampler!");

  glSamplerParameteri(shadow_array_sampler_wrapper_.handle,
                      GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glSamplerParameteri(shadow_array_sampler_wrapper_.handle,
                      GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glSamplerParameteri(shadow_array_sampler_wrapper_.handle, GL_TEXTURE_WRAP_S,
                      GL_CLAMP_TO_BORDER);
  glSamplerParameteri(shadow_array_sampler_wrapper_.handle, GL_TEXTURE_WRAP_T,
                      GL_CLAMP_TO_BORDER);
  glSamplerParameteri(shadow_array_sampler_wrapper_.handle, GL_TEXTURE_WRAP_R,
                      GL_CLAMP_TO_BORDER);
  glSamplerParameteri(shadow_array_sampler_wrapper_.handle,
                      GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
  glSamplerParameteri(shadow_array_sampler_wrapper_.handle,
                      GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
  glSamplerParameterfv(shadow_array_sampler_wrapper_.handle,
                       GL_TEXTURE_BORDER_COLOR, border_color);

  shadow_array_sampler_wrapper_.id = kInvalidSamplerId;
  shadow_array_sampler_wrapper_.ref_count = 1;

  shadow_array_texture_ = {};
  shadow_array_texture_.id = kInvalidTextureId;
  shadow_array_texture_.type = TextureType::Unknown;
  shadow_array_texture_.ref_count = 1;
  shadow_array_texture_.width = shadow_settings_->resolution;
  shadow_array_texture_.height = shadow_settings_->resolution;
  shadow_array_texture_.depth = kShadowLayerCapacity_;
  shadow_array_texture_.mip_levels = 1;
  shadow_array_texture_.channel_count = 1;
  shadow_array_texture_.format = GL_DEPTH_COMPONENT;
  shadow_array_texture_.internal_format = GL_DEPTH_COMPONENT32F;
  shadow_array_texture_.handle = shadow_array_texture_handle_;

  shadow_array_texture_map_ = {};
  shadow_array_texture_map_.sampler = &shadow_array_sampler_wrapper_;
  shadow_array_texture_map_.texture = &shadow_array_texture_;
  shadow_array_texture_map_.texture_resource_id = resource::kInvalidResourceId;
  shadow_array_texture_map_.type = TextureType::Unknown;
}

void LightingHandler::DestroyShadowArrayResources() {
  if (shadow_array_sampler_wrapper_.handle != 0) {
    glDeleteSamplers(1, &shadow_array_sampler_wrapper_.handle);
    shadow_array_sampler_wrapper_.handle = 0;
  }

  if (shadow_array_texture_handle_ != kInvalidTextureHandle) {
    glDeleteTextures(1, &shadow_array_texture_handle_);
    shadow_array_texture_handle_ = kInvalidTextureHandle;
  }

  shadow_array_texture_map_ = {};
  shadow_array_texture_ = {};
  shadow_array_sampler_wrapper_ = {};
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

  COMET_LOG_RENDERING_ERROR("No free shadow layers left in shadow array!");
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

void LightingHandler::InitializeShadowResource(LightId light_id,
                                               const LightProxy& light,
                                               ShadowResource& resource) {
  resource = {};
  resource.is_alive = true;
  resource.is_dirty = true;
  resource.light_id = light_id;
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
               "Failed to allocate shadow layers!");

  for (u32 i{0}; i < resource.view_proj_count; ++i) {
    glGenFramebuffers(1, &resource.framebuffers[i]);
    COMET_ASSERT(resource.framebuffers[i] != kInvalidFrameBufferHandle,
                 "Failed to create shadow framebuffer!");

    glBindFramebuffer(GL_FRAMEBUFFER, resource.framebuffers[i]);
    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                              shadow_array_texture_handle_, 0,
                              resource.first_layer_index + i);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    [[maybe_unused]] auto status{glCheckFramebufferStatus(GL_FRAMEBUFFER)};
    COMET_ASSERT(status == GL_FRAMEBUFFER_COMPLETE,
                 "Shadow framebuffer is incomplete!");
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
  auto is_resolution_changed{resource.resolution !=
                             shadow_settings_->resolution};
  auto resolved_type{ResolveShadowType(light.props, light.shadow)};
  auto is_type_changed{resource.type != resolved_type};
  u32 desired_count{1};

  if (resolved_type == ShadowType::DirectionalOrtho) {
    desired_count = math::Min<u32>(
        math::Max<u32>(light.shadow.cascade_count, 1u), kMaxShadowCascades_);
  }

  auto is_count_changed{resource.view_proj_count != desired_count};

  if (!is_resolution_changed && !is_type_changed && !is_count_changed) {
    return;
  }

  auto light_id{resource.light_id};
  DestroyShadowResource(resource);
  InitializeShadowResource(light_id, light, resource);
}

ShadowResource* LightingHandler::TryGetShadowResource(
    LightId light_id) noexcept {
  auto* index{light_to_shadow_map_.TryGet(light_id)};
  return index != nullptr ? &shadow_resources_[*index] : nullptr;
}

const ShadowResource* LightingHandler::TryGetShadowResource(
    LightId light_id) const noexcept {
  auto* index{light_to_shadow_map_.TryGet(light_id)};
  return index != nullptr ? &shadow_resources_[*index] : nullptr;
}

void LightingHandler::PopulateCascadeSplits(const RenderCameraData& camera_data,
                                            f32 max_distance, u32 cascade_count,
                                            f32 lambda, f32* out_splits) const {
  COMET_ASSERT(out_splits != nullptr, "Cascade split output is null!");

  auto near_plane{camera_data.near_plane};
  auto far_plane{max_distance};
  auto ratio{far_plane / near_plane};

  for (u32 i{0}; i < cascade_count; ++i) {
    auto p{static_cast<f32>(i + 1) / static_cast<f32>(cascade_count)};
    auto log_split{near_plane * math::Pow(ratio, p)};
    auto uni_split{near_plane + (far_plane - near_plane) * p};
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
  auto light_up{ComputeStableUpVector(dir)};

  auto cascade_depth{cascade_far - cascade_near};
  auto caster_extrusion{cascade_depth *
                        shadow_settings_->caster_extrusion_factor};

  auto light_distance{cascade_far + caster_extrusion};
  auto light_position{center - dir * light_distance};
  auto light_view{LookAt(light_position, center, light_up)};

  auto min_x{kF32Max};
  auto max_x{-kF32Max};
  auto min_y{kF32Max};
  auto max_y{-kF32Max};
  auto min_z{kF32Max};
  auto max_z{-kF32Max};

  auto accumulate_ls_point{[&](const math::Vec3& world_p) {
    auto ls4{light_view * math::Vec4{world_p, 1.0f}};

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
  auto dir{math::GetNormalizedCopy(props.direction)};
  auto up{ComputeStableUpVector(dir)};
  auto view{LookAt(props.position, props.position + dir, up)};

  auto fov_y{props.outer_angle * 2.0f};
  auto aspect{1.0f};
  auto near_plane{.1f};

  auto proj{GeneratePerspectiveMatrix(fov_y, aspect, near_plane, max_distance,
                                      ClipSpaceDepthRange::ZeroToOne)};

  return proj * view;
}

void LightingHandler::EnsureStorageBufferCapacity(StorageHandle& handle,
                                                  GLsizeiptr& capacity,
                                                  GLsizeiptr required_size) {
  COMET_ASSERT(handle != kInvalidStorageHandle,
               "Storage buffer must be allocated before resizing!");

  if (required_size <= capacity) {
    return;
  }

  glBindBuffer(GL_SHADER_STORAGE_BUFFER, handle);
  glBufferData(GL_SHADER_STORAGE_BUFFER, required_size, nullptr,
               GL_DYNAMIC_DRAW);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, kInvalidStorageHandle);
  capacity = required_size;
}
}  // namespace gl
}  // namespace rendering
}  // namespace comet