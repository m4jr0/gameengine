// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "vulkan_mesh_handler.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/logger/logging.h"
#include "comet/profiler/profiler.h"
#include "comet/rendering/driver/vulkan/type/vulkan_buffer.h"
#include "comet/rendering/driver/vulkan/utils/vulkan_buffer_utils.h"
#include "comet/rendering/driver/vulkan/utils/vulkan_command_buffer_utils.h"
#include "comet/rendering/driver/vulkan/utils/vulkan_initializer_utils.h"
#include "comet/rendering/driver/vulkan/vulkan_context.h"

namespace comet {
namespace rendering {
namespace vk {
MeshHandler::MeshHandler(const MeshHandlerDescr& descr) : Handler{descr} {}

void MeshHandler::Update(const frame::FramePacket* packet) {
  COMET_PROFILE("MeshHandler::Update");
  COMET_ASSERT(packet != nullptr, "MeshHandler::Update",
               "frame packet is null");

  if (packet->added_geometries->IsEmpty() && packet->dirty_meshes->IsEmpty() &&
      packet->removed_geometries->IsEmpty()) {
    return;
  }

  DestroyMeshProxies(packet->removed_geometries);
  auto update_context{PrepareUpdate(packet)};
  UpdateMeshProxies(packet->dirty_meshes, update_context);
  AddMeshProxies(packet->added_geometries, update_context);
  FinishUpdate(update_context);
}

void MeshHandler::Bind() {
  const auto command_buffer_handle{
      context_->GetFrameData().command_buffer_handle};
  vertex_buffer_.Bind(command_buffer_handle);
  index_buffer_.Bind(command_buffer_handle);
}

void MeshHandler::AcquireFromTransferQueueIfNeeded() {
  if (!is_transfer_) {
    return;
  }

  if (is_transfer_queue_) {
    auto* acquire_barriers{COMET_FRAME_ARRAY_WITH_CAPACITY(
        VkBufferMemoryBarrier, kDefaultAcquireBarrierCount_)};
    auto& device{context_->GetDevice()};
    const auto transfer_queue_index{device.GetTransferQueueIndex()};
    const auto graphics_queue_index{device.GetGraphicsQueueIndex()};

    AddBufferMemoryBarrier(vertex_buffer_.GetBuffer(), acquire_barriers,
                           VK_ACCESS_NONE,
                           VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT |
                               VK_ACCESS_INDIRECT_COMMAND_READ_BIT,
                           transfer_queue_index, graphics_queue_index);

    AddBufferMemoryBarrier(
        index_buffer_.GetBuffer(), acquire_barriers, VK_ACCESS_NONE,
        VK_ACCESS_INDEX_READ_BIT | VK_ACCESS_INDIRECT_COMMAND_READ_BIT,
        transfer_queue_index, graphics_queue_index);

    COMET_ASSERT(acquire_barriers != nullptr,
                 "MeshHandler::AcquireFromTransferQueueIfNeeded",
                 "acquire barriers are null");

    ApplyBufferMemoryBarriers(*acquire_barriers,
                              context_->GetFrameData().command_buffer_handle,
                              VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                              VK_PIPELINE_STAGE_VERTEX_INPUT_BIT |
                                  VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT);
  }

  is_transfer_ = false;
}

const MeshProxy* MeshHandler::Get(geometry::MeshHandle handle) const {
  const auto* proxy{TryGet(handle)};
  COMET_ASSERT(proxy != nullptr, "MeshHandler::Get", "mesh proxy not found",
               "mesh_handle", handle);
  return proxy;
}

const MeshProxy* MeshHandler::TryGet(geometry::MeshHandle handle) const {
  if (!handle) {
    return nullptr;
  }

  const auto index{static_cast<usize>(handle.GetIndex())};

  if (index >= proxies_.GetSize()) {
    return nullptr;
  }

  const auto& proxy{proxies_[index]};
  return proxy.is_alive ? &proxy : nullptr;
}

void MeshHandler::OnInitialize() {
  allocator_.Initialize();

  proxies_ = Array<MeshProxy>::WithCapacity(&allocator_, kDefaultProxyCount_);

  const auto fence_info{init::GenerateFenceCreateInfo()};
  auto& device{context_->GetDevice()};

  vkCreateFence(device, &fence_info, VK_NULL_HANDLE, &upload_fence_handle_);

  auto* allocator_handle{context_->GetAllocatorHandle()};

  is_transfer_queue_ =
      IsTransferFamilyInQueueFamilyIndices(device.GetQueueFamilyIndices());

  staging_buffer_ = GenerateBuffer(
      allocator_handle, kDefaultStagingBufferSize_,
      VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
      VMA_MEMORY_USAGE_CPU_ONLY, 0, 0, VK_SHARING_MODE_EXCLUSIVE,
      "staging_buffer_");

  vertex_buffer_ = VertexGpuBuffer{&allocator_,
                                   context_,
                                   kVertexCountPerBlock_,
                                   kDefaultVertexCount_,
                                   VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                   VMA_MEMORY_USAGE_GPU_ONLY,
                                   0,
                                   VK_SHARING_MODE_EXCLUSIVE,
                                   "vertex_buffer_"};

  index_buffer_ = IndexGpuBuffer{&allocator_,
                                 context_,
                                 kIndexCountPerBlock_,
                                 kDefaultIndexCount_,
                                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                 VMA_MEMORY_USAGE_GPU_ONLY,
                                 0,
                                 VK_SHARING_MODE_EXCLUSIVE,
                                 "index_buffer_"};

  vertex_buffer_.Initialize();
  index_buffer_.Initialize();
}

void MeshHandler::OnShutdown() {
  proxies_.Release();

  vertex_buffer_.Destroy();
  index_buffer_.Destroy();

  if (IsBufferInitialized(staging_buffer_)) {
    DestroyBuffer(staging_buffer_);
  }

  if (upload_fence_handle_ != VK_NULL_HANDLE) {
    vkDestroyFence(context_->GetDevice(), upload_fence_handle_, VK_NULL_HANDLE);
    upload_fence_handle_ = VK_NULL_HANDLE;
  }

  allocator_.Destroy();
  is_transfer_queue_ = false;
  is_transfer_ = false;
}

internal::UpdateContext MeshHandler::PrepareUpdate(
    const frame::FramePacket* packet) {
  COMET_PROFILE("MeshHandler::PrepareUpdate");
  internal::UpdateContext update_context{};

  for (const auto& geometry : *packet->added_geometries) {
    update_context.new_vertex_size +=
        geometry.vertices->GetSize() * sizeof(geometry::SkinnedVertex);
    update_context.new_index_size +=
        geometry.indices->GetSize() * sizeof(geometry::Index);
  }

  for (const auto& mesh : *packet->dirty_meshes) {
    update_context.dirty_vertex_size +=
        mesh.vertices->GetSize() * sizeof(geometry::SkinnedVertex);
    update_context.dirty_index_size +=
        mesh.indices->GetSize() * sizeof(geometry::Index);
  }

  update_context.total_vertex_size =
      update_context.new_vertex_size + update_context.dirty_vertex_size;
  update_context.total_index_size =
      update_context.new_index_size + update_context.dirty_index_size;

  const auto total_size{update_context.total_vertex_size +
                        update_context.total_index_size};

  auto& device{context_->GetDevice()};
  const auto command_pool_handle{context_->GetTransferCommandPoolHandle()};

  update_context.device = &device;
  update_context.command_pool_handle = command_pool_handle;
  update_context.command_buffer_handle =
      GenerateOneTimeCommand(device, update_context.command_pool_handle);

  if (staging_buffer_.size < total_size) {
    ResizeBuffer(
        staging_buffer_, device, command_pool_handle,
        context_->GetAllocatorHandle(), total_size,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VMA_MEMORY_USAGE_CPU_TO_GPU, device.GetTransferQueueHandle(), 0, 0,
        VK_SHARING_MODE_EXCLUSIVE, VK_NULL_HANDLE, "staging_buffer_");
  }

  vertex_buffer_.Resize(update_context.total_vertex_size /
                        sizeof(geometry::SkinnedVertex));
  index_buffer_.Resize(update_context.total_index_size /
                       sizeof(geometry::Index));

  const auto upload_count{packet->added_geometries->GetSize() +
                          packet->dirty_meshes->GetSize()};

  update_context.vertex_copy_regions.Reserve(upload_count);
  update_context.index_copy_regions.Reserve(upload_count);

  update_context.current_staging_index_offset =
      update_context.total_vertex_size;

  MapBuffer(staging_buffer_);

  return update_context;
}

void MeshHandler::FinishUpdate(internal::UpdateContext& update_context) {
  COMET_PROFILE("MeshHandler::FinishUpdate");
  UnmapBuffer(staging_buffer_);
  UploadMeshProxies(update_context);

  auto* release_barriers{COMET_FRAME_ARRAY_WITH_CAPACITY(
      VkBufferMemoryBarrier, kDefaultAcquireBarrierCount_)};

  if (is_transfer_ && is_transfer_queue_) {
    const auto transfer_queue_index{
        update_context.device->GetTransferQueueIndex()};
    const auto graphics_queue_index{
        update_context.device->GetGraphicsQueueIndex()};

    AddBufferMemoryBarrier(vertex_buffer_.GetBuffer(), release_barriers,
                           VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_NONE,
                           transfer_queue_index, graphics_queue_index);

    AddBufferMemoryBarrier(index_buffer_.GetBuffer(), release_barriers,
                           VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_NONE,
                           transfer_queue_index, graphics_queue_index);

    COMET_ASSERT(release_barriers != nullptr, "MeshHandler::FinishUpdate",
                 "release barriers are null");

    ApplyBufferMemoryBarriers(
        *release_barriers, update_context.command_buffer_handle,
        VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
  }

  VkPipelineStageFlags wait_stage{VK_PIPELINE_STAGE_TRANSFER_BIT};
  const auto transfer_value{context_->GetTransferTimelineValue()};

  const auto timeline_semaphore_info{init::GenerateTimelineSemaphoreSubmitInfo(
      1, &transfer_value, 0, VK_NULL_HANDLE)};

  SubmitOneTimeCommandAsync(
      update_context.command_buffer_handle,
      update_context.device->GetTransferQueueHandle(), upload_fence_handle_,
      context_->GetTransferSemaphoreHandle(), VK_NULL_HANDLE, &wait_stage,
      &timeline_semaphore_info);

  WaitAndRecycleOneTimeCommand(
      *update_context.device, update_context.command_pool_handle,
      update_context.command_buffer_handle, upload_fence_handle_);
}

void MeshHandler::AddMeshProxies(const frame::AddedGeometries* geometries,
                                 internal::UpdateContext& update_context) {
  COMET_PROFILE("MeshHandler::AddMeshProxies");

  if (geometries->IsEmpty()) {
    return;
  }

  auto* memory{static_cast<u8*>(staging_buffer_.mapped_memory)};

  for (const auto& geometry : *geometries) {
    if (!geometry.mesh_handle) {
      COMET_LOG_WARNING(LoggerType::Rendering, "MeshHandler::AddMeshProxies",
                        "invalid mesh handle");
      continue;
    }

    const auto proxy_index{static_cast<usize>(geometry.mesh_handle.GetIndex())};

    if (proxy_index >= proxies_.GetSize()) {
      proxies_.Resize(proxy_index + 1);
    }

    auto& proxy{proxies_[proxy_index]};

    if (proxy.is_alive) {
      COMET_LOG_WARNING(LoggerType::Rendering, "MeshHandler::AddMeshProxies",
                        "mesh proxy already exists", "mesh_handle",
                        geometry.mesh_handle);
      continue;
    }

    const auto vertex_size{static_cast<VkDeviceSize>(
        geometry.vertices->GetSize() * sizeof(geometry::SkinnedVertex))};
    const auto index_size{static_cast<VkDeviceSize>(
        geometry.indices->GetSize() * sizeof(geometry::Index))};

    memory::CopyMemory(memory + update_context.current_staging_vertex_offset,
                       geometry.vertices->GetData(), vertex_size);
    memory::CopyMemory(memory + update_context.current_staging_index_offset,
                       geometry.indices->GetData(), index_size);

    const auto vertex_offset{
        vertex_buffer_.Claim(geometry.vertices->GetSize())};
    const auto index_offset{index_buffer_.Claim(geometry.indices->GetSize())};

    update_context.vertex_copy_regions.EmplaceLast(
        update_context.current_staging_vertex_offset,
        vertex_offset * sizeof(geometry::SkinnedVertex), vertex_size);
    update_context.index_copy_regions.EmplaceLast(
        update_context.current_staging_index_offset,
        index_offset * sizeof(geometry::Index), index_size);

    update_context.current_staging_vertex_offset += vertex_size;
    update_context.current_staging_index_offset += index_size;

    proxy.is_alive = true;
    proxy.mesh_handle = geometry.mesh_handle;
    proxy.vertex_count = static_cast<u32>(geometry.vertices->GetSize());
    proxy.index_count = static_cast<u32>(geometry.indices->GetSize());
    proxy.vertex_offset = static_cast<u32>(vertex_offset);
    proxy.index_offset = static_cast<u32>(index_offset);
  }
}

void MeshHandler::UpdateMeshProxies(const frame::DirtyMeshes* meshes,
                                    internal::UpdateContext& update_context) {
  COMET_PROFILE("MeshHandler::UpdateMeshProxies");

  if (meshes->IsEmpty()) {
    return;
  }

  auto* memory{static_cast<u8*>(staging_buffer_.mapped_memory)};

  for (const auto& mesh : *meshes) {
    if (!mesh.mesh_handle) {
      COMET_LOG_WARNING(LoggerType::Rendering, "MeshHandler::UpdateMeshProxies",
                        "invalid mesh handle");
      continue;
    }

    const auto proxy_index{static_cast<usize>(mesh.mesh_handle.GetIndex())};

    if (proxy_index >= proxies_.GetSize() || !proxies_[proxy_index].is_alive) {
      COMET_LOG_WARNING(LoggerType::Rendering, "MeshHandler::UpdateMeshProxies",
                        "mesh proxy not found", "mesh_handle",
                        mesh.mesh_handle);
      continue;
    }

    auto& proxy{proxies_[proxy_index]};

    const auto new_vertex_size{static_cast<VkDeviceSize>(
        mesh.vertices->GetSize() * sizeof(geometry::SkinnedVertex))};
    const auto new_index_size{static_cast<VkDeviceSize>(
        mesh.indices->GetSize() * sizeof(geometry::Index))};

    const auto vertex_offset{vertex_buffer_.CheckOrMove(
        proxy.vertex_offset, proxy.vertex_count, new_vertex_size)};
    const auto index_offset{index_buffer_.CheckOrMove(
        proxy.index_offset, proxy.index_count, new_index_size)};

    proxy.vertex_count = static_cast<u32>(mesh.vertices->GetSize());
    proxy.index_count = static_cast<u32>(mesh.indices->GetSize());
    proxy.vertex_offset = static_cast<u32>(vertex_offset);
    proxy.index_offset = static_cast<u32>(index_offset);

    memory::CopyMemory(memory + update_context.current_staging_vertex_offset,
                       mesh.vertices->GetData(), new_vertex_size);
    memory::CopyMemory(memory + update_context.current_staging_index_offset,
                       mesh.indices->GetData(), new_index_size);

    update_context.vertex_copy_regions.EmplaceLast(
        update_context.current_staging_vertex_offset,
        vertex_offset * sizeof(geometry::SkinnedVertex), new_vertex_size);
    update_context.index_copy_regions.EmplaceLast(
        update_context.current_staging_index_offset,
        index_offset * sizeof(geometry::Index), new_index_size);

    update_context.current_staging_vertex_offset += new_vertex_size;
    update_context.current_staging_index_offset += new_index_size;
  }
}

void MeshHandler::DestroyMeshProxies(
    const frame::RemovedGeometries* geometries) {
  COMET_PROFILE("MeshHandler::DestroyMeshProxies");

  for (const auto& geometry : *geometries) {
    if (!geometry.mesh_handle) {
      COMET_LOG_WARNING(LoggerType::Rendering,
                        "MeshHandler::DestroyMeshProxies",
                        "invalid mesh handle");
      continue;
    }

    const auto proxy_index{static_cast<usize>(geometry.mesh_handle.GetIndex())};

    if (proxy_index >= proxies_.GetSize() || !proxies_[proxy_index].is_alive) {
      COMET_LOG_WARNING(
          LoggerType::Rendering, "MeshHandler::DestroyMeshProxies",
          "mesh proxy not found", "mesh_handle", geometry.mesh_handle);
      continue;
    }

    auto& proxy{proxies_[proxy_index]};
    vertex_buffer_.Release(proxy.vertex_offset, proxy.vertex_count);
    index_buffer_.Release(proxy.index_offset, proxy.index_count);
    proxy = {};
  }
}

void MeshHandler::UploadMeshProxies(
    const internal::UpdateContext& update_context) {
  COMET_PROFILE("MeshHandler::UploadMeshProxies");

  is_transfer_ =
      !vertex_buffer_.Upload(update_context.command_buffer_handle,
                             staging_buffer_,
                             update_context.vertex_copy_regions) ||
      !index_buffer_.Upload(update_context.command_buffer_handle,
                            staging_buffer_, update_context.index_copy_regions);
}
}  // namespace vk
}  // namespace rendering
}  // namespace comet