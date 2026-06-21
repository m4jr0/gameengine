// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_render_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/render/driver/vulkan/handler/vulkan_debug_handler.h"
////////////////////////////////////////////////////////////////////////////////

#ifdef COMET_DEBUG_RENDERING

#include "comet/render/driver/vulkan/type/vulkan_mesh.h"
#include "comet/render/driver/vulkan/utils/vulkan_buffer_utils.h"

namespace comet {
namespace render {
namespace vk {
DebugHandler::DebugHandler(const DebugHandlerDescr& descr)
    : Handler{descr},
      render_proxy_record_store_{descr.render_proxy_record_store} {
  COMET_ASSERT(render_proxy_record_store_ != nullptr,
               "DebugHandler::DebugHandler",
               "render proxy record store is null");
}

void DebugHandler::Update() {
  aabb_count_ =
      static_cast<u32>(render_proxy_record_store_->GetRecordSlotCount());

  EnsureAabbCapacity(GetDebugAabbCount());

  EnsureLineCapacity(GetDebugAabbCount(), GetDebugCameraFrustumCount(),
                     GetDebugCascadeFrustumCount(),
                     GetDebugLightFrustumCount());
}

DebugGpuData DebugHandler::GetGpuData(
    FrameInFlightIndex frame_index) const noexcept {
  DebugGpuData gpu_data{};

  const auto& debug_data{ssbo_debug_data_[frame_index]};

  gpu_data.ssbo_debug_data_handle = debug_data.handle;
  gpu_data.ssbo_debug_data_size = debug_data.size;

  gpu_data.ssbo_debug_lines_handle = ssbo_debug_lines_.handle;
  gpu_data.ssbo_debug_lines_size = ssbo_debug_lines_.size;

  gpu_data.ssbo_debug_aabbs_handle = ssbo_debug_aabbs_.handle;
  gpu_data.ssbo_debug_aabbs_size = ssbo_debug_aabbs_.size;

  gpu_data.ssbo_debug_camera_frustums_handle =
      ssbo_debug_camera_frustums_.handle;
  gpu_data.ssbo_debug_camera_frustums_size = ssbo_debug_camera_frustums_.size;

  gpu_data.ssbo_debug_cascade_frustums_handle =
      ssbo_debug_cascade_frustums_.handle;
  gpu_data.ssbo_debug_cascade_frustums_size = ssbo_debug_cascade_frustums_.size;

  gpu_data.ssbo_debug_light_frustums_handle = ssbo_debug_light_frustums_.handle;
  gpu_data.ssbo_debug_light_frustums_size = ssbo_debug_light_frustums_.size;

  return gpu_data;
}

u32 DebugHandler::GetAabbCount() const noexcept { return aabb_count_; }

void DebugHandler::PopulateCullDebugReadBarriers(
    FrameInFlightIndex frame_index, Array<VkBufferMemoryBarrier>& out) const {
  const auto& debug_data{ssbo_debug_data_[frame_index]};

  if (debug_data.handle == VK_NULL_HANDLE) {
    return;
  }

  AddBufferMemoryBarrier(debug_data, &out, VK_ACCESS_SHADER_WRITE_BIT,
                         VK_ACCESS_HOST_READ_BIT);
}

void DebugHandler::PrepareCullDebugWrite(FrameInFlightIndex frame_index) {
  auto* debug_data{debug_data_[frame_index]};

  if (debug_data == nullptr) {
    visible_count_ = 0;
    return;
  }

  visible_count_ = debug_data->visible_count;
  debug_data->visible_count = 0;
}

u32 DebugHandler::GetVisibleCount() const noexcept { return visible_count_; }

void DebugHandler::SetDebugCameraFrustum(u32 index,
                                         const math::Mat4& view_proj) {
  if (index >= kMaxDebugCameraFrustumCount_) {
    return;
  }

  if (debug_camera_frustums_.GetSize() <= index) {
    debug_camera_frustums_.Resize(index + 1);
  }

  debug_camera_frustums_[index] = view_proj;

  ScopedMappedBuffer mapped{ssbo_debug_camera_frustums_};
  auto* memory{
      static_cast<math::Mat4*>(ssbo_debug_camera_frustums_.mapped_memory)};
  memory[index] = view_proj;
}

void DebugHandler::ClearDebugCameraFrustums() {
  debug_camera_frustums_.Clear();
}

void DebugHandler::SetDebugCascadeFrustumCorners(
    u32 index, const StaticArray<math::Vec3, 8>& corners) {
  if (index >= kMaxDebugLightFrustumCount_) {
    return;
  }

  if (debug_cascade_frustums_.GetSize() <= index) {
    debug_cascade_frustums_.Resize(index + 1);
  }

  auto& dst{debug_cascade_frustums_[index]};

  for (u32 i{0}; i < 8; ++i) {
    dst.corners[i] = math::Vec4{corners[i].x, corners[i].y, corners[i].z, 1.0f};
  }

  ScopedMappedBuffer mapped{ssbo_debug_cascade_frustums_};
  auto* memory{static_cast<DebugFrustumCorners*>(
      ssbo_debug_cascade_frustums_.mapped_memory)};
  memory[index] = dst;
}

void DebugHandler::ClearDebugCascadeFrustumCorners() {
  debug_cascade_frustums_.Clear();
}

void DebugHandler::SetDebugLightFrustum(u32 index,
                                        const math::Mat4& view_proj) {
  if (index >= kMaxDebugLightFrustumCount_) {
    return;
  }

  if (debug_light_frustums_.GetSize() <= index) {
    debug_light_frustums_.Resize(index + 1);
  }

  debug_light_frustums_[index] = view_proj;

  ScopedMappedBuffer mapped{ssbo_debug_light_frustums_};
  auto* memory{
      static_cast<math::Mat4*>(ssbo_debug_light_frustums_.mapped_memory)};
  memory[index] = view_proj;
}

void DebugHandler::ClearDebugLightFrustums() { debug_light_frustums_.Clear(); }

const Buffer& DebugHandler::GetDebugLineBuffer() const noexcept {
  return ssbo_debug_lines_;
}

const Buffer& DebugHandler::GetDebugAabbBuffer() const noexcept {
  return ssbo_debug_aabbs_;
}

const Buffer& DebugHandler::GetDebugCameraFrustumBuffer() const noexcept {
  return ssbo_debug_camera_frustums_;
}

const Buffer& DebugHandler::GetDebugCascadeFrustumCornersBuffer()
    const noexcept {
  return ssbo_debug_cascade_frustums_;
}

const Buffer& DebugHandler::GetDebugLightFrustumBuffer() const noexcept {
  return ssbo_debug_light_frustums_;
}

u32 DebugHandler::GetDebugAabbCount() const noexcept { return aabb_count_; }

u32 DebugHandler::GetDebugLineVertexCount() const noexcept {
  return aabb_count_ * kDebugAabbLineVertexCount_ +
         GetDebugCameraFrustumCount() * kDebugFrustumLineVertexCount_ +
         GetDebugCascadeFrustumCount() * kDebugFrustumLineVertexCount_ +
         GetDebugLightFrustumCount() * kDebugFrustumLineVertexCount_;
}

u32 DebugHandler::GetDebugCameraFrustumCount() const noexcept {
  return static_cast<u32>(debug_camera_frustums_.GetSize());
}

u32 DebugHandler::GetDebugCascadeFrustumCount() const noexcept {
  return static_cast<u32>(debug_cascade_frustums_.GetSize());
}

u32 DebugHandler::GetDebugLightFrustumCount() const noexcept {
  return static_cast<u32>(debug_light_frustums_.GetSize());
}

void DebugHandler::OnInitialize() {
  InitializeReadbackData();
  InitializeGpuData();
}

void DebugHandler::OnShutdown() {
  DestroyReadbackData();
  DestroyGpuData();

  aabb_count_ = 0;
  visible_count_ = 0;
}

void DebugHandler::InitializeReadbackData() {
  auto* allocator{context_->GetAllocatorHandle()};

  for (u32 i{0}; i < kDebugDataBufferCount_; ++i) {
    auto& debug_data{ssbo_debug_data_[i]};

    RecreateBuffer(
        debug_data, allocator, sizeof(GpuDebugData),
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VMA_MEMORY_USAGE_GPU_TO_CPU, 0, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        VK_SHARING_MODE_EXCLUSIVE, "ssbo_debug_data_");

    MapBuffer(debug_data);
    debug_data_[i] = static_cast<GpuDebugData*>(debug_data.mapped_memory);
  }
}

void DebugHandler::DestroyReadbackData() {
  for (u32 i{0}; i < kDebugDataBufferCount_; ++i) {
    auto& debug_data{ssbo_debug_data_[i]};

    if (IsBufferInitialized(debug_data)) {
      UnmapBuffer(debug_data);
      DestroyBuffer(debug_data);
    }

    debug_data_[i] = nullptr;
  }
}

void DebugHandler::InitializeGpuData() {
  debug_cascade_frustums_ = Array<DebugFrustumCorners>::WithCapacity(
      &platform_allocator_, kMaxDebugLightFrustumCount_);

  debug_light_frustums_ = Array<math::Mat4>::WithCapacity(
      &platform_allocator_, kMaxDebugLightFrustumCount_);

  debug_camera_frustums_ = Array<math::Mat4>::WithCapacity(
      &platform_allocator_, kMaxDebugCameraFrustumCount_);

  auto* allocator{context_->GetAllocatorHandle()};

  RecreateBuffer(
      ssbo_debug_aabbs_, allocator, GetAabbBufferSize(kDefaultDebugAabbCount_),
      VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
      VMA_MEMORY_USAGE_GPU_ONLY, 0, 0, VK_SHARING_MODE_EXCLUSIVE,
      "ssbo_debug_aabbs_");

  RecreateBuffer(
      ssbo_debug_cascade_frustums_, allocator,
      static_cast<VkDeviceSize>(kMaxDebugLightFrustumCount_ *
                                sizeof(DebugFrustumCorners)),
      VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
      VMA_MEMORY_USAGE_CPU_TO_GPU, 0, 0, VK_SHARING_MODE_EXCLUSIVE,
      "ssbo_debug_cascade_frustums_");

  RecreateBuffer(
      ssbo_debug_light_frustums_, allocator,
      static_cast<VkDeviceSize>(kMaxDebugLightFrustumCount_ *
                                sizeof(math::Mat4)),
      VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
      VMA_MEMORY_USAGE_CPU_TO_GPU, 0, 0, VK_SHARING_MODE_EXCLUSIVE,
      "ssbo_debug_light_frustums_");

  RecreateBuffer(
      ssbo_debug_camera_frustums_, allocator,
      static_cast<VkDeviceSize>(kMaxDebugCameraFrustumCount_ *
                                sizeof(math::Mat4)),
      VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
      VMA_MEMORY_USAGE_CPU_TO_GPU, 0, 0, VK_SHARING_MODE_EXCLUSIVE,
      "ssbo_debug_camera_frustums_");

  RecreateBuffer(
      ssbo_debug_lines_, allocator,
      GetLineBufferSize(kDefaultDebugAabbCount_, kMaxDebugCascadeFrustumCount_,
                        kMaxDebugLightFrustumCount_,
                        kMaxDebugCameraFrustumCount_),
      VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
      VMA_MEMORY_USAGE_GPU_ONLY, 0, 0, VK_SHARING_MODE_EXCLUSIVE,
      "ssbo_debug_lines_");
}

void DebugHandler::DestroyGpuData() {
  if (IsBufferInitialized(ssbo_debug_aabbs_)) {
    DestroyBuffer(ssbo_debug_aabbs_);
  }

  if (IsBufferInitialized(ssbo_debug_lines_)) {
    DestroyBuffer(ssbo_debug_lines_);
  }

  if (IsBufferInitialized(ssbo_debug_cascade_frustums_)) {
    DestroyBuffer(ssbo_debug_cascade_frustums_);
  }

  if (IsBufferInitialized(ssbo_debug_light_frustums_)) {
    DestroyBuffer(ssbo_debug_light_frustums_);
  }

  if (IsBufferInitialized(ssbo_debug_camera_frustums_)) {
    DestroyBuffer(ssbo_debug_camera_frustums_);
  }

  debug_cascade_frustums_.Release();
  debug_light_frustums_.Release();
  debug_camera_frustums_.Release();
}

void DebugHandler::EnsureAabbCapacity(u32 count) {
  const auto size{GetAabbBufferSize(count)};

  if (size == 0) {
    return;
  }

  if (IsBufferInitialized(ssbo_debug_aabbs_) &&
      ssbo_debug_aabbs_.size >= size) {
    return;
  }

  RecreateBuffer(
      ssbo_debug_aabbs_, context_->GetAllocatorHandle(), size,
      VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
      VMA_MEMORY_USAGE_GPU_ONLY, 0, 0, VK_SHARING_MODE_EXCLUSIVE,
      "ssbo_debug_aabbs_");
}

void DebugHandler::EnsureLineCapacity(u32 aabb_count, u32 camera_frustum_count,
                                      u32 cascade_frustum_count,
                                      u32 light_frustum_count) {
  const auto size{GetLineBufferSize(aabb_count, camera_frustum_count,
                                    cascade_frustum_count,
                                    light_frustum_count)};

  if (size == 0) {
    return;
  }

  if (IsBufferInitialized(ssbo_debug_lines_) &&
      ssbo_debug_lines_.size >= size) {
    return;
  }

  RecreateBuffer(
      ssbo_debug_lines_, context_->GetAllocatorHandle(), size,
      VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
      VMA_MEMORY_USAGE_GPU_ONLY, 0, 0, VK_SHARING_MODE_EXCLUSIVE,
      "ssbo_debug_lines_");
}

VkDeviceSize DebugHandler::GetAabbBufferSize(u32 count) const noexcept {
  return static_cast<VkDeviceSize>(count * sizeof(GpuDebugAabb));
}

VkDeviceSize DebugHandler::GetLineBufferSize(
    u32 aabb_count, u32 camera_frustum_count, u32 cascade_frustum_count,
    u32 light_frustum_count) const noexcept {
  const auto vertex_count{aabb_count * kDebugAabbLineVertexCount_ +
                          camera_frustum_count * kDebugFrustumLineVertexCount_ +
                          cascade_frustum_count *
                              kDebugFrustumLineVertexCount_ +
                          light_frustum_count * kDebugFrustumLineVertexCount_};

  return static_cast<VkDeviceSize>(vertex_count * sizeof(GpuDebugLineVertex));
}
}  // namespace vk
}  // namespace render
}  // namespace comet

#endif  // COMET_DEBUG_RENDERING