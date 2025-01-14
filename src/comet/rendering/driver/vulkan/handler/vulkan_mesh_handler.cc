// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "comet_pch.h"

#include "vulkan_mesh_handler.h"

#include "comet/core/logger.h"
#include "comet/math/math_commons.h"
#include "comet/profiler/profiler.h"
#include "comet/rendering/driver/vulkan/data/vulkan_buffer.h"
#include "comet/rendering/driver/vulkan/utils/vulkan_buffer_utils.h"
#include "comet/rendering/driver/vulkan/utils/vulkan_command_buffer_utils.h"
#include "comet/rendering/driver/vulkan/utils/vulkan_initializer_utils.h"
#include "comet/rendering/driver/vulkan/vulkan_debug.h"
#include "comet/rendering/rendering_common.h"

namespace comet {
namespace rendering {
namespace vk {
MeshHandler::MeshHandler(const MeshHandlerDescr& descr) : Handler{descr} {}

void MeshHandler::Initialize() {
  Handler::Initialize();
  allocator_.Initialize();
  mesh_to_proxy_map_ = {&allocator_, kDefaultProxyCount_};
  proxies_ = {&allocator_, kDefaultProxyCount_};
  free_vertex_regions_ = {&allocator_, kDefaultProxyCount_};
  free_index_regions_ = {&allocator_, kDefaultProxyCount_};

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

  uploaded_vertex_buffer_ = GenerateBuffer(
      allocator_handle, kDefaultStagingBufferSize_,
      VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT |
          VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
      VMA_MEMORY_USAGE_GPU_ONLY, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 0,
      VK_SHARING_MODE_EXCLUSIVE, "uploaded_vertex_buffer_");

  uploaded_index_buffer_ = GenerateBuffer(
      allocator_handle, kDefaultStagingBufferSize_,
      VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT |
          VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
      VMA_MEMORY_USAGE_GPU_ONLY, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 0,
      VK_SHARING_MODE_EXCLUSIVE, "uploaded_index_buffer_");

  free_vertex_regions_.EmplaceBack(static_cast<u32>(0),
                                   kDefaultStagingBufferSize_);
  free_index_regions_.EmplaceBack(static_cast<u32>(0),
                                  kDefaultStagingBufferSize_);
}

void MeshHandler::Shutdown() {
  ClearAllMeshProxies();

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
  VkDeviceSize offset{0};

  vkCmdBindVertexBuffers(command_buffer_handle, 0, 1,
                         &uploaded_vertex_buffer_.handle, &offset);

  vkCmdBindIndexBuffer(command_buffer_handle, uploaded_index_buffer_.handle, 0,
                       VK_INDEX_TYPE_UINT32);
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

    // TODO(m4jr0): Fix synchronization issues.
    device.WaitIdle();

    AddBufferMemoryBarrier(uploaded_vertex_buffer_, acquire_barriers,
                           VK_ACCESS_NONE,
                           VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT |
                               VK_ACCESS_INDIRECT_COMMAND_READ_BIT,
                           transfer_queue_index, graphics_queue_index);

    AddBufferMemoryBarrier(
        uploaded_index_buffer_, acquire_barriers, VK_ACCESS_NONE,
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

u32 MeshHandler::AllocateFromFreeList(Array<internal::FreeRegion>& free_list,
                                      VkDeviceSize size) {
  COMET_PROFILE("MeshHandler::AllocateFromFreeList");

  for (auto it{free_list.begin()}; it != free_list.end(); ++it) {
    if (it->size < size) {
      continue;
    }

    auto offset{it->offset};

    if (it->size > size) {
      it->offset += static_cast<u32>(size);
      it->size -= size;
    } else {
      free_list.RemoveFromPos(it);
    }

    return offset;
  }

  return internal::FreeRegion::kInvalidOffset;
}

void MeshHandler::FreeToFreeList(Array<internal::FreeRegion>& free_list,
                                 u32 offset, VkDeviceSize size) {
  COMET_PROFILE("MeshHandler::FreeToFreeList");
  free_list.EmplaceBack(offset, size);

  std::sort(free_list.begin(), free_list.end(),
            [](const internal::FreeRegion& a, const internal::FreeRegion& b) {
              return a.offset < b.offset;
            });

  for (usize i{0}; i < free_list.GetSize() - 1; ++i) {
    if (free_list[i].offset + free_list[i].size != free_list[i + 1].offset) {
      continue;
    }

    free_list[i].size += free_list[i + 1].size;
    free_list.RemoveFromIndex(i + 1);
    --i;
  }
}

internal::UpdateContext MeshHandler::PrepareUpdate(
    const frame::FramePacket* packet) {
  COMET_PROFILE("MeshHandler::PrepareUpdate");
  internal::UpdateContext update_context{};

  for (const auto& geometry : *packet->added_geometries) {
    update_context.new_vertex_size +=
        geometry.vertices->GetSize() * sizeof(geometry::Vertex);
    update_context.new_index_size +=
        geometry.indices->GetSize() * sizeof(geometry::Index);
  }

  for (const auto& mesh : *packet->dirty_meshes) {
    update_context.dirty_vertex_size +=
        mesh.vertices->GetSize() * sizeof(geometry::Vertex);
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

  if (uploaded_vertex_buffer_.size < update_context.total_vertex_size) {
    ResizeVertexBuffer(update_context.total_vertex_size);
  }

  if (uploaded_index_buffer_.size < update_context.total_index_size) {
    ResizeIndexBuffer(update_context.total_index_size);
  }

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

      AddBufferMemoryBarrier(uploaded_vertex_buffer_, release_barriers,
                             VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_NONE,
                             transfer_queue_index, graphics_queue_index);

      AddBufferMemoryBarrier(uploaded_index_buffer_, release_barriers,
                             VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_NONE,
                             transfer_queue_index, graphics_queue_index);

      ApplyBufferMemoryBarriers(
          &release_barriers, update_context.command_buffer_handle,
          VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
    }
  }

  context_->GetFrameData().is_transfer = true;

  SubmitOneTimeCommand(
      update_context.command_buffer_handle, update_context.command_pool_handle,
      *update_context.device, update_context.device->GetTransferQueueHandle(),
      upload_fence_handle_, VK_NULL_HANDLE,
      context_->GetTransferSemaphoreHandle());
}

void MeshHandler::AddMeshProxies(const frame::AddedGeometries* geometries,
                                 internal::UpdateContext& update_context) {
  COMET_PROFILE("MeshHandler::AddMeshProxies");
  if (geometries->IsEmpty()) {
    return;
  }

  auto* memory{static_cast<u8*>(staging_buffer_.mapped_memory)};

  for (const auto& geometry : *geometries) {
    memory::CopyMemory(memory + update_context.current_staging_vertex_offset,
                       geometry.vertices->GetData(),
                       geometry.vertices->GetSize() * sizeof(geometry::Vertex));
    memory::CopyMemory(memory + update_context.current_staging_index_offset,
                       geometry.indices->GetData(),
                       geometry.indices->GetSize() * sizeof(geometry::Index));

    auto vertex_size{static_cast<VkDeviceSize>(geometry.vertices->GetSize() *
                                               sizeof(geometry::Vertex))};
    auto index_size{static_cast<VkDeviceSize>(geometry.indices->GetSize() *
                                              sizeof(geometry::Index))};

    auto vertex_byte_offset{
        AllocateFromFreeList(free_vertex_regions_, vertex_size)};
    auto index_byte_offset{
        AllocateFromFreeList(free_index_regions_, index_size)};

    if (vertex_byte_offset == internal::FreeRegion::kInvalidOffset) {
      ResizeVertexBuffer(math::Max(uploaded_vertex_buffer_.size + vertex_size,
                                   uploaded_vertex_buffer_.size * 2));
      vertex_byte_offset =
          AllocateFromFreeList(free_vertex_regions_, vertex_size);
    }

    if (index_byte_offset == internal::FreeRegion::kInvalidOffset) {
      ResizeIndexBuffer(math::Max(uploaded_index_buffer_.size + index_size,
                                  uploaded_index_buffer_.size * 2));
      index_byte_offset = AllocateFromFreeList(free_index_regions_, index_size);
    }

    COMET_ASSERT(vertex_byte_offset != internal::FreeRegion::kInvalidOffset,
                 "Not enough memory to upload vertices!");
    COMET_ASSERT(index_byte_offset != internal::FreeRegion::kInvalidOffset,
                 "Not enough memory to upload indices!");

    update_context.vertex_copy_regions.EmplaceBack(
        update_context.current_staging_vertex_offset, vertex_byte_offset,
        vertex_size);
    update_context.index_copy_regions.EmplaceBack(
        update_context.current_staging_index_offset, index_byte_offset,
        index_size);

    update_context.current_staging_vertex_offset += vertex_size;
    update_context.current_staging_index_offset += index_size;

    mesh_to_proxy_map_[geometry.mesh_id] = proxies_.GetSize();
    auto& proxy{proxies_.EmplaceBack()};
    proxy.vertex_count = static_cast<u32>(geometry.vertices->GetSize());
    proxy.index_count = static_cast<u32>(geometry.indices->GetSize());
    proxy.vertex_offset = vertex_byte_offset / sizeof(geometry::Vertex);
    proxy.index_offset = index_byte_offset / sizeof(geometry::Index);
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

    auto new_vertex_size{static_cast<VkDeviceSize>(mesh.vertices->GetSize() *
                                                   sizeof(geometry::Vertex))};
    auto new_index_size{static_cast<VkDeviceSize>(mesh.indices->GetSize() *
                                                  sizeof(geometry::Index))};
    auto vertex_byte_offset{proxy.vertex_offset * sizeof(geometry::Vertex)};
    auto index_byte_offset{proxy.index_offset * sizeof(geometry::Index)};

    if (new_vertex_size > proxy.vertex_count * sizeof(geometry::Vertex)) {
      FreeToFreeList(free_vertex_regions_, proxy.vertex_offset,
                     proxy.vertex_count * sizeof(geometry::Vertex));

      vertex_byte_offset =
          AllocateFromFreeList(free_vertex_regions_, new_vertex_size);

      if (vertex_byte_offset == internal::FreeRegion::kInvalidOffset) {
        ResizeVertexBuffer(
            math::Max(uploaded_vertex_buffer_.size + new_vertex_size,
                      uploaded_vertex_buffer_.size * 2));
        vertex_byte_offset =
            AllocateFromFreeList(free_vertex_regions_, new_vertex_size);
      }
    }

    if (new_index_size > proxy.index_count * sizeof(geometry::Index)) {
      FreeToFreeList(free_index_regions_, proxy.index_offset,
                     proxy.index_count * sizeof(geometry::Index));

      index_byte_offset =
          AllocateFromFreeList(free_index_regions_, new_index_size);

      if (index_byte_offset == internal::FreeRegion::kInvalidOffset) {
        ResizeIndexBuffer(
            math::Max(uploaded_index_buffer_.size + new_index_size,
                      uploaded_index_buffer_.size * 2));
        index_byte_offset =
            AllocateFromFreeList(free_index_regions_, new_index_size);
      }
    }

    COMET_ASSERT(vertex_byte_offset != internal::FreeRegion::kInvalidOffset,
                 "Not enough memory to upload vertices!");
    COMET_ASSERT(index_byte_offset != internal::FreeRegion::kInvalidOffset,
                 "Not enough memory to upload indices!");

    proxy.vertex_count = static_cast<u32>(mesh.vertices->GetSize());
    proxy.index_count = static_cast<u32>(mesh.indices->GetSize());
    proxy.vertex_offset =
        static_cast<u32>(vertex_byte_offset / sizeof(geometry::Vertex));
    proxy.index_offset =
        static_cast<u32>(index_byte_offset / sizeof(geometry::Index));

    memory::CopyMemory(memory + update_context.current_staging_vertex_offset,
                       mesh.vertices->GetData(), new_vertex_size);
    memory::CopyMemory(memory + update_context.current_staging_index_offset,
                       mesh.indices->GetData(), new_index_size);

    update_context.vertex_copy_regions.EmplaceBack(
        update_context.current_staging_vertex_offset, vertex_byte_offset,
        new_vertex_size);
    update_context.index_copy_regions.EmplaceBack(
        update_context.current_staging_index_offset, index_byte_offset,
        new_index_size);

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

    FreeToFreeList(free_vertex_regions_, proxy.vertex_offset,
                   proxy.vertex_count * sizeof(geometry::Vertex));
    FreeToFreeList(free_index_regions_, proxy.index_offset,
                   proxy.index_count * sizeof(geometry::Index));

    mesh_to_proxy_map_.Remove(geometry.mesh_id);
  }
}

void MeshHandler::UploadMeshProxies(
    const internal::UpdateContext& update_context) {
  COMET_PROFILE("MeshHandler::UploadMeshProxies");

  is_transfer_ = !update_context.vertex_copy_regions.IsEmpty() ||
                 !update_context.index_copy_regions.IsEmpty();

  if (!update_context.vertex_copy_regions.IsEmpty()) {
    vkCmdCopyBuffer(
        update_context.command_buffer_handle, staging_buffer_.handle,
        uploaded_vertex_buffer_.handle,
        static_cast<u32>(update_context.vertex_copy_regions.GetSize()),
        update_context.vertex_copy_regions.GetData());
  }

  if (!update_context.index_copy_regions.IsEmpty()) {
    vkCmdCopyBuffer(
        update_context.command_buffer_handle, staging_buffer_.handle,
        uploaded_index_buffer_.handle,
        static_cast<u32>(update_context.index_copy_regions.GetSize()),
        update_context.index_copy_regions.GetData());
  }
}

void MeshHandler::ClearAllMeshProxies() {
  COMET_PROFILE("MeshHandler::ClearAllMeshProxies");
  proxies_ = {};
  mesh_to_proxy_map_ = {};

  free_vertex_regions_ = {};
  free_index_regions_ = {};

  if (IsBufferInitialized(uploaded_vertex_buffer_)) {
    DestroyBuffer(uploaded_vertex_buffer_);
  }

  if (IsBufferInitialized(uploaded_index_buffer_)) {
    DestroyBuffer(uploaded_index_buffer_);
  }

  if (IsBufferInitialized(staging_buffer_)) {
    DestroyBuffer(staging_buffer_);
  }
}

void MeshHandler::ResizeVertexBuffer(VkDeviceSize new_size) {
  COMET_PROFILE("MeshHandler::ResizeVertexBuffer");

  if (new_size <= uploaded_vertex_buffer_.size) {
    return;
  }

  auto old_capacity{uploaded_vertex_buffer_.size};
  auto& device{context_->GetDevice()};

  ResizeBuffer(
      uploaded_vertex_buffer_, device, context_->GetTransferCommandPoolHandle(),
      context_->GetAllocatorHandle(), new_size,
      VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT |
          VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
      VMA_MEMORY_USAGE_AUTO, device.GetGraphicsQueueHandle(),
      VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT, 0, VK_SHARING_MODE_EXCLUSIVE,
      VK_NULL_HANDLE, "uploaded_vertex_buffer_");

  free_vertex_regions_.EmplaceBack(static_cast<u32>(old_capacity),
                                   new_size - old_capacity);
}

void MeshHandler::ResizeIndexBuffer(VkDeviceSize new_size) {
  COMET_PROFILE("MeshHandler::ResizeIndexBuffer");

  if (new_size <= uploaded_index_buffer_.size) {
    return;
  }

  auto old_capacity{uploaded_index_buffer_.size};
  auto& device{context_->GetDevice()};

  ResizeBuffer(
      uploaded_index_buffer_, device, context_->GetTransferCommandPoolHandle(),
      context_->GetAllocatorHandle(), new_size,
      VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT |
          VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
      VMA_MEMORY_USAGE_AUTO, device.GetGraphicsQueueHandle(),
      VK_ACCESS_INDEX_READ_BIT, 0, VK_SHARING_MODE_EXCLUSIVE, VK_NULL_HANDLE,
      "uploaded_index_buffer_");

  free_index_regions_.EmplaceBack(static_cast<u32>(old_capacity),
                                  new_size - old_capacity);
}
}  // namespace vk
}  // namespace rendering
}  // namespace comet
