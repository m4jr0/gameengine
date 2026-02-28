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

#include "comet/core/logger.h"
#include "comet/profiler/profiler.h"
#include "comet/rendering/driver/vulkan/data/vulkan_buffer.h"
#include "comet/rendering/driver/vulkan/utils/vulkan_buffer_utils.h"
#include "comet/rendering/driver/vulkan/utils/vulkan_command_buffer_utils.h"
#include "comet/rendering/driver/vulkan/utils/vulkan_initializer_utils.h"
#include "comet/rendering/driver/vulkan/vulkan_context.h"

namespace comet {
namespace rendering {
namespace vk {
MeshHandler::MeshHandler(const MeshHandlerDescr& descr) : Handler{descr} {}

void MeshHandler::Initialize() {
  Handler::Initialize();
  allocator_.Initialize();
  mesh_to_proxy_map_ = {&allocator_, kDefaultProxyCount_};
  proxies_ = {&allocator_, kDefaultProxyCount_};

  auto fence_info{init::GenerateFenceCreateInfo()};
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

void MeshHandler::Shutdown() {
  proxies_ = {};
  mesh_to_proxy_map_ = {};

  vertex_buffer_.Destroy();
  index_buffer_.Destroy();

  if (IsBufferInitialized(staging_buffer_)) {
    DestroyBuffer(staging_buffer_);
  }

  if (upload_fence_handle_ != VK_NULL_HANDLE) {
    vkDestroyFence(context_->GetDevice(), upload_fence_handle_, VK_NULL_HANDLE);
  }

  allocator_.Destroy();
  is_transfer_queue_ = false;
  is_transfer_ = false;
  Handler::Shutdown();
}

void MeshHandler::Update(const frame::FramePacket* packet) {
  COMET_PROFILE("MeshHandler::Update");

  if (packet->added_geometries->IsEmpty() && packet->dirty_meshes->IsEmpty() &&
      packet->removed_geometries->IsEmpty()) {
    return;
  }

  // Removing meshes is CPU-side only.
  DestroyMeshProxies(packet->removed_geometries);
  auto update_context{PrepareUpdate(packet)};
  UpdateMeshProxies(packet->dirty_meshes, update_context);
  AddMeshProxies(packet->added_geometries, update_context);
  FinishUpdate(update_context);
}

void MeshHandler::Bind() {
  auto command_buffer_handle{context_->GetFrameData().command_buffer_handle};
  vertex_buffer_.Bind(command_buffer_handle);
  index_buffer_.Bind(command_buffer_handle);
}

void MeshHandler::Wait() {
  if (!is_transfer_) {
    return;
  }

  if (is_transfer_queue_) {
    auto* acquire_barriers{
        COMET_FRAME_ARRAY(VkBufferMemoryBarrier, kDefaultAcquireBarrierCount_)};
    auto& device{context_->GetDevice()};
    auto transfer_queue_index{device.GetTransferQueueIndex()};
    auto graphics_queue_index{device.GetGraphicsQueueIndex()};

    AddBufferMemoryBarrier(vertex_buffer_.GetBuffer(), acquire_barriers,
                           VK_ACCESS_NONE,
                           VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT |
                               VK_ACCESS_INDIRECT_COMMAND_READ_BIT,
                           transfer_queue_index, graphics_queue_index);

    AddBufferMemoryBarrier(
        index_buffer_.GetBuffer(), acquire_barriers, VK_ACCESS_NONE,
        VK_ACCESS_INDEX_READ_BIT | VK_ACCESS_INDIRECT_COMMAND_READ_BIT,
        transfer_queue_index, graphics_queue_index);

    ApplyBufferMemoryBarriers(&acquire_barriers,
                              context_->GetFrameData().command_buffer_handle,
                              VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                              VK_PIPELINE_STAGE_VERTEX_INPUT_BIT |
                                  VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT);
  }

  is_transfer_ = false;
}

MeshProxyHandle MeshHandler::GetHandle(geometry::MeshId mesh_id) const {
  auto* index{mesh_to_proxy_map_.TryGet(mesh_id)};
  return index != nullptr ? static_cast<MeshProxyHandle>(*index)
                          : kInvalidMeshProxyHandle;
}

const MeshProxy* MeshHandler::Get(MeshProxyHandle handle) const {
  return &proxies_[static_cast<usize>(handle)];
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

  auto total_size{update_context.total_vertex_size +
                  update_context.total_index_size};

  auto& device{context_->GetDevice()};
  auto command_pool_handle{context_->GetTransferCommandPoolHandle()};

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

  auto upload_count{packet->added_geometries->GetSize() +
                    packet->dirty_meshes->GetSize()};

  update_context.vertex_copy_regions.Reserve(upload_count);
  update_context.index_copy_regions.Reserve(upload_count);

  // Indices are copied at the end of the staging buffer.
  update_context.current_staging_index_offset =
      update_context.total_vertex_size;

  MapBuffer(staging_buffer_);

  return update_context;
}

void MeshHandler::FinishUpdate(internal::UpdateContext& update_context) {
  COMET_PROFILE("MeshHandler::FinishUpdate");
  UnmapBuffer(staging_buffer_);
  UploadMeshProxies(update_context);

  auto* release_barriers{
      COMET_FRAME_ARRAY(VkBufferMemoryBarrier, kDefaultAcquireBarrierCount_)};

  if (is_transfer_) {
    if (is_transfer_queue_) {
      auto transfer_queue_index{update_context.device->GetTransferQueueIndex()};
      auto graphics_queue_index{update_context.device->GetGraphicsQueueIndex()};

      AddBufferMemoryBarrier(vertex_buffer_.GetBuffer(), release_barriers,
                             VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_NONE,
                             transfer_queue_index, graphics_queue_index);

      AddBufferMemoryBarrier(index_buffer_.GetBuffer(), release_barriers,
                             VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_NONE,
                             transfer_queue_index, graphics_queue_index);

      ApplyBufferMemoryBarriers(
          &release_barriers, update_context.command_buffer_handle,
          VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
    }
  }

  VkPipelineStageFlags wait_stage{VK_PIPELINE_STAGE_TRANSFER_BIT};
  auto transfer_value{context_->GetTransferTimelineValue()};

  auto timeline_semaphore_info{init::GenerateTimelineSemaphoreSubmitInfo(
      1, &transfer_value, 0, VK_NULL_HANDLE)};

  SubmitOneTimeCommand(
      update_context.command_buffer_handle, update_context.command_pool_handle,
      *update_context.device, update_context.device->GetTransferQueueHandle(),
      upload_fence_handle_, context_->GetTransferSemaphoreHandle(),
      VK_NULL_HANDLE, &wait_stage, &timeline_semaphore_info);
}

void MeshHandler::AddMeshProxies(const frame::AddedGeometries* geometries,
                                 internal::UpdateContext& update_context) {
  COMET_PROFILE("MeshHandler::AddMeshProxies");
  if (geometries->IsEmpty()) {
    return;
  }

  auto* memory{static_cast<u8*>(staging_buffer_.mapped_memory)};

  for (const auto& geometry : *geometries) {
    auto vertex_size{static_cast<VkDeviceSize>(
        geometry.vertices->GetSize() * sizeof(geometry::SkinnedVertex))};
    auto index_size{static_cast<VkDeviceSize>(geometry.indices->GetSize() *
                                              sizeof(geometry::Index))};

    memory::CopyMemory(memory + update_context.current_staging_vertex_offset,
                       geometry.vertices->GetData(), vertex_size);
    memory::CopyMemory(memory + update_context.current_staging_index_offset,
                       geometry.indices->GetData(), index_size);

    auto vertex_offset{vertex_buffer_.Claim(geometry.vertices->GetSize())};
    auto index_offset{index_buffer_.Claim(geometry.indices->GetSize())};

    update_context.vertex_copy_regions.EmplaceBack(
        update_context.current_staging_vertex_offset,
        vertex_offset * sizeof(geometry::SkinnedVertex), vertex_size);
    update_context.index_copy_regions.EmplaceBack(
        update_context.current_staging_index_offset,
        index_offset * sizeof(geometry::Index), index_size);

    update_context.current_staging_vertex_offset += vertex_size;
    update_context.current_staging_index_offset += index_size;

    mesh_to_proxy_map_[geometry.mesh_id] = proxies_.GetSize();
    auto& proxy{proxies_.EmplaceBack()};
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
    auto* proxy_id{mesh_to_proxy_map_.TryGet(mesh.mesh_id)};

    if (proxy_id == nullptr) {
      COMET_LOG_RENDERING_WARNING("Tried to update non-existing mesh #",
                                  mesh.mesh_id, "!");
      continue;
    }

    COMET_ASSERT(*proxy_id < proxies_.GetSize(), "Proxy index out of bounds!");
    auto& proxy{proxies_[*proxy_id]};

    auto new_vertex_size{static_cast<VkDeviceSize>(
        mesh.vertices->GetSize() * sizeof(geometry::SkinnedVertex))};
    auto new_index_size{static_cast<VkDeviceSize>(mesh.indices->GetSize() *
                                                  sizeof(geometry::Index))};

    auto vertex_offset{vertex_buffer_.CheckOrMove(
        proxy.vertex_offset, proxy.vertex_count, new_vertex_size)};

    auto index_offset{index_buffer_.CheckOrMove(
        proxy.index_offset, proxy.index_count, new_index_size)};

    proxy.vertex_count = static_cast<u32>(mesh.vertices->GetSize());
    proxy.index_count = static_cast<u32>(mesh.indices->GetSize());
    proxy.vertex_offset = static_cast<u32>(vertex_offset);
    proxy.index_offset = static_cast<u32>(index_offset);

    memory::CopyMemory(memory + update_context.current_staging_vertex_offset,
                       mesh.vertices->GetData(), new_vertex_size);
    memory::CopyMemory(memory + update_context.current_staging_index_offset,
                       mesh.indices->GetData(), new_index_size);

    update_context.vertex_copy_regions.EmplaceBack(
        update_context.current_staging_vertex_offset,
        vertex_offset * sizeof(geometry::SkinnedVertex), new_vertex_size);
    update_context.index_copy_regions.EmplaceBack(
        update_context.current_staging_index_offset,
        index_offset * sizeof(geometry::Index), new_index_size);

    update_context.current_staging_vertex_offset += new_vertex_size;
    update_context.current_staging_index_offset += new_index_size;
  }
}

void MeshHandler::DestroyMeshProxies(
    const frame::RemovedGeometries* geometries) {
  COMET_PROFILE("MeshHandler::DestroyMeshProxies");

  for (auto& geometry : *geometries) {
    auto* proxy_id{mesh_to_proxy_map_.TryGet(geometry.mesh_id)};

    if (proxy_id == nullptr) {
      COMET_LOG_RENDERING_WARNING("Tried to remove non-existing mesh #",
                                  geometry.mesh_id, "!");
      continue;
    }

    auto& proxy{proxies_[*proxy_id]};
    vertex_buffer_.Release(proxy.vertex_offset, proxy.vertex_count);
    index_buffer_.Release(proxy.index_offset, proxy.index_count);
    mesh_to_proxy_map_.Remove(geometry.mesh_id);
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
