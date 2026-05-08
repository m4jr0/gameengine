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
#include "comet/core/type/array.h"
#include "comet/profiler/profiler.h"
#include "comet/rendering/driver/vulkan/utils/vulkan_buffer_utils.h"
#include "comet/rendering/driver/vulkan/vulkan_context.h"
#include "comet/rendering/driver/vulkan/vulkan_debug.h"

namespace comet {
namespace rendering {
namespace vk {
MeshHandler::MeshHandler(const MeshHandlerDescr& descr)
    : Handler{descr}, proxies_{&registry_allocator_, kDefaultProxyCount_} {}

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
  const auto current_frame{context_->GetFrameInFlightIndex()};
  auto& frame_data{context_->GetFrameData(current_frame)};

  const auto command_buffer_handle{frame_data.command_buffer_handle};
  vertex_buffer_.Bind(command_buffer_handle);
  index_buffer_.Bind(command_buffer_handle);
}

void MeshHandler::AcquireFromTransferQueueIfNeeded() {
  const auto current_frame{context_->GetFrameInFlightIndex()};
  auto& frame_data{context_->GetFrameData(current_frame)};

  if (!frame_data.requires_upload_ownership_acquire) {
    return;
  }

  auto& device{context_->GetDevice()};

  COMET_ASSERT(!device.IsUploadQueueGraphics(),
               "MeshHandler::AcquireFromTransferQueueIfNeeded",
               "unexpected ownership acquire on graphics upload queue");

  auto* acquire_barriers{COMET_FRAME_ARRAY_WITH_CAPACITY(
      VkBufferMemoryBarrier, kDefaultAcquireBarrierCount_)};

  const auto& upload_queue{device.GetUploadQueueContext()};
  const auto& graphics_queue{device.GetGraphicsQueueContext()};
  const auto upload_queue_index{upload_queue.family_index};
  const auto graphics_queue_index{graphics_queue.family_index};

  COMET_ASSERT(upload_queue.IsValid(),
               "MeshHandler::AcquireFromTransferQueueIfNeeded",
               "upload queue context is invalid");
  COMET_ASSERT(graphics_queue.IsValid(),
               "MeshHandler::AcquireFromTransferQueueIfNeeded",
               "graphics queue context is invalid");

  AddBufferMemoryBarrier(
      vertex_buffer_.GetBuffer(), acquire_barriers, VK_ACCESS_NONE,
      VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT | VK_ACCESS_INDIRECT_COMMAND_READ_BIT,
      upload_queue_index, graphics_queue_index);

  AddBufferMemoryBarrier(
      index_buffer_.GetBuffer(), acquire_barriers, VK_ACCESS_NONE,
      VK_ACCESS_INDEX_READ_BIT | VK_ACCESS_INDIRECT_COMMAND_READ_BIT,
      upload_queue_index, graphics_queue_index);

  ApplyBufferMemoryBarriers(
      *acquire_barriers, frame_data.command_buffer_handle,
      VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
      VK_PIPELINE_STAGE_VERTEX_INPUT_BIT | VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT);

  frame_data.requires_upload_ownership_acquire = false;
  frame_data.has_upload_submission = false;
}

void MeshHandler::ReleasePendingUploadResources(
    FrameInFlightIndex frame_index) {
  vertex_buffer_.ReleasePendingBuffers(frame_index);
  index_buffer_.ReleasePendingBuffers(frame_index);
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

  const auto proxy_handle{proxies_.TryGetHandle(handle)};

  if (!proxy_handle) {
    return nullptr;
  }

  return proxies_.TryGet(proxy_handle);
}

void MeshHandler::OnInitialize() {
  allocator_.Initialize();
  proxies_.Initialize();

  auto* allocator_handle{context_->GetAllocatorHandle()};

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
  memory::PlatformAllocator tmp_allocator{memory::kEngineMemoryTagRendering};

  auto handles_to_destroy{Array<MeshProxyRegistry::ItemHandle>::WithCapacity(
      &tmp_allocator, proxies_.GetLiveCount())};

  proxies_.ForEachLive(
      [&handles_to_destroy](MeshProxyRegistry::ItemHandle handle,
                            const MeshProxy*) {
        handles_to_destroy.PushLast(handle);
      });

  for (const auto handle : handles_to_destroy) {
    auto* proxy{proxies_.Drain(handle)};

    if (proxy == nullptr) {
      continue;
    }

    vertex_buffer_.Release(proxy->vertex_offset, proxy->vertex_count);
    index_buffer_.Release(proxy->index_offset, proxy->index_count);

    allocator_.Deallocate(proxy);
  }

  proxies_.Destroy();

  vertex_buffer_.Destroy();
  index_buffer_.Destroy();

  if (IsBufferInitialized(staging_buffer_)) {
    DestroyBuffer(staging_buffer_);
  }

  allocator_.Destroy();
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
  const auto& upload_queue{context_->GetUploadQueueContext()};
  const auto& graphics_queue{device.GetGraphicsQueueContext()};

  const auto current_frame{context_->GetFrameInFlightIndex()};
  auto& frame_data{context_->GetFrameData(current_frame)};
  auto command_buffer_handle{frame_data.upload_command_buffer_handle};

  COMET_ASSERT(command_buffer_handle != VK_NULL_HANDLE,
               "MeshHandler::PrepareUpdate",
               "upload command buffer handle is invalid");
  COMET_ASSERT(frame_data.upload_fence_handle != VK_NULL_HANDLE,
               "MeshHandler::PrepareUpdate", "upload fence handle is invalid");

  update_context.upload_queue = &upload_queue;
  update_context.graphics_queue = &graphics_queue;
  update_context.command_buffer_handle = command_buffer_handle;

  if (staging_buffer_.size < total_size) {
    auto result{EnsureBufferCapacity(
        staging_buffer_, frame_data.upload_command_buffer_handle,
        context_->GetAllocatorHandle(), total_size,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VMA_MEMORY_USAGE_CPU_TO_GPU, 0, 0, VK_SHARING_MODE_EXCLUSIVE, false,
        "staging_buffer_")};

    if (result.old_buffer.handle != VK_NULL_HANDLE) {
      DestroyBuffer(result.old_buffer);
    }
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

  const auto upload_result{UploadMeshProxies(update_context)};

  const auto current_frame{context_->GetFrameInFlightIndex()};
  auto& frame_data{context_->GetFrameData(current_frame)};

  if (!context_->GetDevice().IsUploadQueueGraphics()) {
    auto* release_barriers{COMET_FRAME_ARRAY_WITH_CAPACITY(
        VkBufferMemoryBarrier, kDefaultReleaseBarrierCount_)};

    bool has_release_barrier{false};

    const auto upload_queue_index{update_context.upload_queue->family_index};
    const auto graphics_queue_index{
        update_context.graphics_queue->family_index};

    if (upload_result.uploaded_vertex) {
      AddBufferMemoryBarrier(vertex_buffer_.GetBuffer(), release_barriers,
                             VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_NONE,
                             upload_queue_index, graphics_queue_index);
      has_release_barrier = true;
    }

    if (upload_result.uploaded_index) {
      AddBufferMemoryBarrier(index_buffer_.GetBuffer(), release_barriers,
                             VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_NONE,
                             upload_queue_index, graphics_queue_index);
      has_release_barrier = true;
    }

    if (has_release_barrier) {
      ApplyBufferMemoryBarriers(
          *release_barriers, update_context.command_buffer_handle,
          VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
    }
  }

  const bool uploaded_anything =
      upload_result.uploaded_vertex || upload_result.uploaded_index;

  frame_data.requires_upload_ownership_acquire |=
      uploaded_anything && !context_->GetDevice().IsUploadQueueGraphics();

  frame_data.has_upload_submission =
      frame_data.has_upload_submission || uploaded_anything;
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

    if (const auto proxy_handle{proxies_.TryAcquire(geometry.mesh_handle)};
        proxy_handle) {
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

    auto* proxy{allocator_.AllocateOneAndPopulate<MeshProxy>()};

    proxy->is_alive = true;
    proxy->mesh_handle = geometry.mesh_handle;
    proxy->vertex_count = static_cast<u32>(geometry.vertices->GetSize());
    proxy->index_count = static_cast<u32>(geometry.indices->GetSize());
    proxy->vertex_offset = static_cast<u32>(vertex_offset);
    proxy->index_offset = static_cast<u32>(index_offset);

    const auto proxy_handle{proxies_.Create(geometry.mesh_handle, proxy)};
    COMET_ASSERT(proxy_handle, "MeshHandler::AddMeshProxies",
                 "mesh proxy registry creation failed", "mesh_handle",
                 geometry.mesh_handle);
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

    const auto proxy_handle{proxies_.TryGetHandle(mesh.mesh_handle)};

    if (!proxy_handle) {
      COMET_LOG_WARNING(LoggerType::Rendering, "MeshHandler::UpdateMeshProxies",
                        "mesh proxy not found", "mesh_handle",
                        mesh.mesh_handle);
      continue;
    }

    auto* proxy{proxies_.Get(proxy_handle)};

    const auto new_vertex_size{static_cast<VkDeviceSize>(
        mesh.vertices->GetSize() * sizeof(geometry::SkinnedVertex))};
    const auto new_index_size{static_cast<VkDeviceSize>(
        mesh.indices->GetSize() * sizeof(geometry::Index))};

    const auto vertex_offset{vertex_buffer_.CheckOrMove(
        proxy->vertex_offset, proxy->vertex_count, mesh.vertices->GetSize())};
    const auto index_offset{index_buffer_.CheckOrMove(
        proxy->index_offset, proxy->index_count, mesh.indices->GetSize())};

    proxy->vertex_count = static_cast<u32>(mesh.vertices->GetSize());
    proxy->index_count = static_cast<u32>(mesh.indices->GetSize());
    proxy->vertex_offset = static_cast<u32>(vertex_offset);
    proxy->index_offset = static_cast<u32>(index_offset);

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

    const auto proxy_handle{proxies_.TryGetHandle(geometry.mesh_handle)};

    if (!proxy_handle) {
      COMET_LOG_WARNING(
          LoggerType::Rendering, "MeshHandler::DestroyMeshProxies",
          "mesh proxy not found", "mesh_handle", geometry.mesh_handle);
      continue;
    }

    auto* proxy{proxies_.Get(proxy_handle)};

    if (!proxies_.Release(proxy_handle)) {
      continue;
    }

    vertex_buffer_.Release(proxy->vertex_offset, proxy->vertex_count);
    index_buffer_.Release(proxy->index_offset, proxy->index_count);

    proxies_.Remove(proxy_handle);
    allocator_.Deallocate(proxy);
  }
}

internal::MeshUploadResult MeshHandler::UploadMeshProxies(
    const internal::UpdateContext& update_context) {
  COMET_PROFILE("MeshHandler::UploadMeshProxies");

  internal::MeshUploadResult result{};
  result.uploaded_vertex = vertex_buffer_.Upload(
      update_context.command_buffer_handle, staging_buffer_,
      update_context.vertex_copy_regions);
  result.uploaded_index =
      index_buffer_.Upload(update_context.command_buffer_handle,
                           staging_buffer_, update_context.index_copy_regions);

  return result;
}
}  // namespace vk
}  // namespace rendering
}  // namespace comet