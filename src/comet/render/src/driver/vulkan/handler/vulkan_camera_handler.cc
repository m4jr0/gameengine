// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_render_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/render/driver/vulkan/handler/vulkan_camera_handler.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/memory/memory_utils.h"
#include "comet/core/math/vector.h"
#include "comet/runtime/profiler/profiler.h"
#include "comet/render/driver/vulkan/type/vulkan_camera.h"
#include "comet/render/driver/vulkan/utils/vulkan_buffer_utils.h"

namespace comet {
namespace render {
namespace vk {
CameraHandler::CameraHandler(const CameraHandlerDescr& descr)
    : Handler{descr} {}

void CameraHandler::Update(const frame::FramePacket* packet) {
  COMET_PROFILE("CameraHandler::Update");
  COMET_ASSERT(packet != nullptr, "CameraHandler::Update",
               "frame packet is null");
  COMET_ASSERT(packet->camera_views != nullptr, "CameraHandler::Update",
               "camera views are null");

  const auto frame_index{context_->GetFrameInFlightIndex()};

  EnsureBuffersInitialized(frame_index);
  UploadCameraDatas(frame_index, packet);
}

CameraGpuData CameraHandler::GetGpuData(
    FrameInFlightIndex frame_index) const noexcept {
  const auto& buffer{ssbo_camera_datas_[frame_index]};

  return {
      .ssbo_camera_datas_handle = buffer.handle,
      .ssbo_camera_datas_size = buffer.size,
  };
}

void CameraHandler::PopulateCameraReadBarrier(
    FrameInFlightIndex frame_index, Array<VkBufferMemoryBarrier>& out) const {
  const auto& buffer{ssbo_camera_datas_[frame_index]};

  if (buffer.handle == VK_NULL_HANDLE) {
    return;
  }

  const auto& upload_queue{context_->GetDevice().GetUploadQueueContext()};
  const auto& graphics_queue{context_->GetDevice().GetGraphicsQueueContext()};

  AddBufferMemoryBarrier(buffer, &out, VK_ACCESS_NONE,
                         VK_ACCESS_SHADER_READ_BIT, upload_queue.family_index,
                         graphics_queue.family_index);
}

void CameraHandler::OnInitialize() {
  const auto frame_count{context_->GetMaxFramesInFlight()};

  staging_ssbo_camera_datas_ =
      Array<Buffer>::WithCapacity(&platform_allocator_, frame_count);
  ssbo_camera_datas_ =
      Array<Buffer>::WithCapacity(&platform_allocator_, frame_count);

  staging_ssbo_camera_datas_.Resize(frame_count);
  ssbo_camera_datas_.Resize(frame_count);

  pending_buffer_destroys_ = Array<Array<Buffer>>::WithCapacity(
      &platform_allocator_, context_->GetMaxFramesInFlight());
  pending_buffer_destroys_.Resize(context_->GetMaxFramesInFlight());

  for (auto& buffers : pending_buffer_destroys_) {
    buffers = Array<Buffer>{&platform_allocator_};
  }
}

void CameraHandler::OnShutdown() {
  for (auto& buffers : pending_buffer_destroys_) {
    for (auto& buffer : buffers) {
      if (IsBufferInitialized(buffer)) {
        DestroyBuffer(buffer);
      }
    }

    buffers.Release();
  }

  pending_buffer_destroys_.Release();

  for (auto& buffer : staging_ssbo_camera_datas_) {
    if (IsBufferInitialized(buffer)) {
      DestroyBuffer(buffer);
    }
  }

  for (auto& buffer : ssbo_camera_datas_) {
    if (IsBufferInitialized(buffer)) {
      DestroyBuffer(buffer);
    }
  }

  staging_ssbo_camera_datas_.Release();
  ssbo_camera_datas_.Release();
}

void CameraHandler::EnsureBuffersInitialized(FrameInFlightIndex frame_index) {
  auto* allocator{context_->GetAllocatorHandle()};

  if (!IsBufferInitialized(staging_ssbo_camera_datas_[frame_index])) {
    RecreateBuffer(staging_ssbo_camera_datas_[frame_index], allocator,
                   sizeof(GpuCameraData), VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                   VMA_MEMORY_USAGE_CPU_TO_GPU, 0, 0, VK_SHARING_MODE_EXCLUSIVE,
                   "staging_ssbo_camera_datas_init");
  }

  if (!IsBufferInitialized(ssbo_camera_datas_[frame_index])) {
    RecreateBuffer(
        ssbo_camera_datas_[frame_index], allocator, sizeof(GpuCameraData),
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
        VMA_MEMORY_USAGE_GPU_ONLY, 0, 0, VK_SHARING_MODE_EXCLUSIVE,
        "ssbo_camera_datas_init");
  }
}

void CameraHandler::UploadCameraDatas(FrameInFlightIndex frame_index,
                                      const frame::FramePacket* packet) {
  const auto camera_count{packet->camera_views->GetSize()};

  if (camera_count == 0) {
    return;
  }

  const auto required_size{
      static_cast<VkDeviceSize>(camera_count * sizeof(GpuCameraData))};

  auto* allocator{context_->GetAllocatorHandle()};
  auto& frame_data{context_->GetFrameData(frame_index)};

  const auto staging_result{EnsureBufferCapacity(
      staging_ssbo_camera_datas_[frame_index],
      frame_data.upload_command_buffer_handle, allocator, required_size,
      VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, 0, 0,
      VK_SHARING_MODE_EXCLUSIVE, false, "staging_ssbo_camera_datas_")};

  if (staging_result.old_buffer.handle != VK_NULL_HANDLE) {
    pending_buffer_destroys_[frame_index].PushLast(staging_result.old_buffer);
  }

  const auto gpu_result{EnsureBufferCapacity(
      ssbo_camera_datas_[frame_index], frame_data.upload_command_buffer_handle,
      allocator, required_size,
      VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
      VMA_MEMORY_USAGE_GPU_ONLY, 0, 0, VK_SHARING_MODE_EXCLUSIVE, false,
      "ssbo_camera_datas_")};

  if (gpu_result.old_buffer.handle != VK_NULL_HANDLE) {
    pending_buffer_destroys_[frame_index].PushLast(gpu_result.old_buffer);
  }

  {
    ScopedMappedBuffer mapped{staging_ssbo_camera_datas_[frame_index]};
    auto* memory{static_cast<GpuCameraData*>(
        staging_ssbo_camera_datas_[frame_index].mapped_memory)};

    for (usize i{0}; i < camera_count; ++i) {
      const auto& src{(*packet->camera_views)[i].data};
      auto& dst{memory[i]};

      dst.projection = src.projection_matrix;
      dst.view = src.view_matrix;
      dst.view_position = math::Vec4{src.view_position.x, src.view_position.y,
                                     src.view_position.z, 0.0f};
    }
  }

  VkBufferCopy copy{};
  copy.size = required_size;

  vkCmdCopyBuffer(frame_data.upload_command_buffer_handle,
                  staging_ssbo_camera_datas_[frame_index].handle,
                  ssbo_camera_datas_[frame_index].handle, 1, &copy);

  AddUploadReleaseBarrier(frame_index, ssbo_camera_datas_[frame_index]);

  frame_data.has_upload_submission = true;
  frame_data.requires_upload_ownership_acquire |=
      !context_->GetDevice().IsUploadQueueGraphics();
}

void CameraHandler::AddUploadReleaseBarrier(FrameInFlightIndex frame_index,
                                            const Buffer& buffer) {
  if (context_->GetDevice().IsUploadQueueGraphics()) {
    return;
  }

  auto& frame_data{context_->GetFrameData(frame_index)};
  auto* barriers{COMET_FRAME_ARRAY_WITH_CAPACITY(VkBufferMemoryBarrier, 1)};

  const auto& upload_queue{context_->GetDevice().GetUploadQueueContext()};
  const auto& graphics_queue{context_->GetDevice().GetGraphicsQueueContext()};

  AddBufferMemoryBarrier(buffer, barriers, VK_ACCESS_TRANSFER_WRITE_BIT,
                         VK_ACCESS_NONE, upload_queue.family_index,
                         graphics_queue.family_index);

  ApplyBufferMemoryBarriers(*barriers, frame_data.upload_command_buffer_handle,
                            VK_PIPELINE_STAGE_TRANSFER_BIT,
                            VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
}

}  // namespace vk
}  // namespace render
}  // namespace comet