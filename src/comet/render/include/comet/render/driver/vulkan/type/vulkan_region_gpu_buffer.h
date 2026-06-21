// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RENDER_DRIVER_VULKAN_TYPE_VULKAN_REGION_GPU_BUFFER_H_
#define COMET_RENDER_DRIVER_VULKAN_TYPE_VULKAN_REGION_GPU_BUFFER_H_

// External. ///////////////////////////////////////////////////////////////////
#include "vma/vk_mem_alloc.h"
#include "vulkan/vulkan.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/essentials.h"
#include "comet/core/logger/logging.h"
#include "comet/core/memory/allocator/allocator.h"
#include "comet/core/memory/memory_utils.h"
#include "comet/core/type/region_map.h"
#include "comet/profiler/profiler.h"
#include "comet/render/driver/vulkan/type/vulkan_buffer.h"
#include "comet/render/driver/vulkan/utils/vulkan_buffer_utils.h"
#include "comet/render/driver/vulkan/vulkan_context.h"

namespace comet {
namespace rendering {
namespace vk {
template <typename T>
struct RegionGpuBuffer {
  RegionGpuBuffer() = default;

  RegionGpuBuffer(
      memory::Allocator* allocator, Context* context, VkBufferUsageFlags usage,
      usize element_block_count, usize element_count = 0,
      VkMemoryPropertyFlags memory_property_flags = 0,
      VmaMemoryUsage vma_memory_usage = VMA_MEMORY_USAGE_AUTO,
      VmaAllocationCreateFlags vma_flags = 0,
      VkSharingMode sharing_mode = VK_SHARING_MODE_EXCLUSIVE,
      [[maybe_unused]] const schar* debug_label = "region_gpu_buffer")
      : usage_{usage | VK_BUFFER_USAGE_TRANSFER_SRC_BIT |
               VK_BUFFER_USAGE_TRANSFER_DST_BIT},
        context_{context},
        vma_memory_usage_{vma_memory_usage},
        memory_property_flags_{memory_property_flags},
        vma_flags_{vma_flags},
        sharing_mode_{sharing_mode},
        element_block_count_{element_block_count},
        element_count_{element_count},
        region_map_{allocator, element_block_count_ * sizeof(T),
                    element_count_ * sizeof(T)} {
#ifdef COMET_RENDERING_USE_DEBUG_LABELS
    if (debug_label == nullptr) {
      debug_label = "";
    }

    auto debug_label_len{math::Min(GetLength(debug_label), kMaxDebugLabelLen_)};
    Copy(debug_label_, debug_label, debug_label_len);
#endif  // COMET_RENDERING_USE_DEBUG_LABELS
  }

  RegionGpuBuffer(const RegionGpuBuffer&) = delete;
  RegionGpuBuffer(RegionGpuBuffer&&) = default;
  RegionGpuBuffer& operator=(const RegionGpuBuffer&) = delete;
  RegionGpuBuffer& operator=(RegionGpuBuffer&&) = default;

  ~RegionGpuBuffer() {
    COMET_ASSERT(!is_initialized_, "RegionGpuBuffer::~RegionGpuBuffer",
                 "region gpu buffer still initialized");
  }

  void Initialize() {
    COMET_ASSERT(!is_initialized_, "RegionGpuBuffer::Initialize",
                 "region gpu buffer already initialized");

    pending_buffer_destroys_ = Array<Array<Buffer>>{&platform_allocator_};
    pending_buffer_destroys_.Resize(context_->GetMaxFramesInFlight());

    for (usize i{0}; i < pending_buffer_destroys_.GetSize(); ++i) {
      pending_buffer_destroys_[i] = Array<Buffer>{&platform_allocator_};
    }

    Resize(element_count_);
    is_initialized_ = true;
  }

  void Destroy() {
    COMET_ASSERT(is_initialized_, "RegionGpuBuffer::Destroy",
                 "region gpu buffer not initialized");

    for (auto& pending_buffers : pending_buffer_destroys_) {
      for (auto& buffer : pending_buffers) {
        if (IsBufferInitialized(buffer)) {
          DestroyBuffer(buffer);
        }
      }

      pending_buffers.Release();
    }

    pending_buffer_destroys_.Release();

    region_map_.Destroy();

    if (IsBufferInitialized(buffer_)) {
      DestroyBuffer(buffer_);
    }

    is_initialized_ = false;
  }

  usize Claim(usize claimed_count) {
    COMET_PROFILE("RegionGpuBuffer<T>::Claim");
    auto claimed_size{claimed_count * sizeof(T)};
    auto block_offset{region_map_.Claim(claimed_size)};

    if (block_offset == kInvalidSize) {
      auto new_element_count{
          math::Max(element_count_ + claimed_count, element_count_ * 2)};

      COMET_LOG_WARNING(LoggerType::Rendering, "RegionGpuBuffer::Claim",
                        "region gpu buffer resize required", "old_size",
                        element_count_ * sizeof(T), "new_size",
                        new_element_count * sizeof(T));

      Resize(new_element_count);
      block_offset = region_map_.Claim(claimed_size);
    }

    COMET_ASSERT(block_offset != kInvalidSize, "RegionGpuBuffer::Claim",
                 "region gpu buffer is out of memory", "claimed_count",
                 claimed_count, "element_size", sizeof(T));

    return block_offset / sizeof(T);
  }

  void Release(usize index_offset, usize released_count) {
    COMET_PROFILE("RegionGpuBuffer<T>::Release");
    region_map_.Release(index_offset * sizeof(T), released_count * sizeof(T));
  }

  usize CheckOrMove(u32 old_index_offset, usize old_count, usize new_count) {
    COMET_PROFILE("RegionGpuBuffer<T>::CheckOrMove");
    if (old_count >= new_count) {
      return old_index_offset;
    }

    Release(old_index_offset, old_count);
    return Claim(new_count);
  }

  bool Upload(VkCommandBuffer command_buffer_handle,
              const Buffer& source_buffer,
              const Array<VkBufferCopy>& copy_regions) {
    COMET_PROFILE("RegionGpuBuffer<T>::Upload");
    if (copy_regions.IsEmpty()) {
      return false;
    }

    vkCmdCopyBuffer(command_buffer_handle, source_buffer.handle, buffer_.handle,
                    static_cast<u32>(copy_regions.GetSize()),
                    copy_regions.GetData());

    return true;
  }

  void Resize(usize new_element_count) {
    COMET_PROFILE("RegionGpuBuffer<T>::Resize");
    new_element_count =
        memory::RoundUpToMultiple(new_element_count, element_block_count_);

    if (new_element_count <= element_count_ && is_initialized_) {
      return;
    }

    const auto current_frame{context_->GetFrameInFlightIndex()};
    auto& frame_data{context_->GetFrameData(current_frame)};

    element_count_ = new_element_count;
    const auto buffer_size{element_count_ * sizeof(T)};

    const auto result{EnsureBufferCapacity(
        buffer_, frame_data.upload_command_buffer_handle,
        context_->GetAllocatorHandle(), static_cast<VkDeviceSize>(buffer_size),
        usage_, vma_memory_usage_, memory_property_flags_, vma_flags_,
        sharing_mode_, true
#ifdef COMET_RENDERING_USE_DEBUG_LABELS
        ,
        debug_label_
#else
        ,
        nullptr
#endif
        )};

    if (result.old_buffer.handle != VK_NULL_HANDLE) {
      pending_buffer_destroys_[current_frame].PushLast(result.old_buffer);
    }

    if (result.has_transfer_work) {
      frame_data.has_upload_submission = true;
      frame_data.requires_upload_ownership_acquire |=
          !context_->GetDevice().IsUploadQueueGraphics();

      if (!context_->GetDevice().IsUploadQueueGraphics()) {
        auto* release_barriers{
            COMET_FRAME_ARRAY_WITH_CAPACITY(VkBufferMemoryBarrier, 1)};

        const auto& upload_queue{context_->GetDevice().GetUploadQueueContext()};
        const auto& graphics_queue{
            context_->GetDevice().GetGraphicsQueueContext()};

        AddBufferMemoryBarrier(buffer_, release_barriers,
                               VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_NONE,
                               upload_queue.family_index,
                               graphics_queue.family_index);

        ApplyBufferMemoryBarriers(*release_barriers,
                                  frame_data.upload_command_buffer_handle,
                                  VK_PIPELINE_STAGE_TRANSFER_BIT,
                                  VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
      }
    }

    region_map_.Resize(buffer_size);
  }

  void ReleasePendingBuffers(FrameInFlightIndex frame_index) {
    COMET_ASSERT(frame_index < pending_buffer_destroys_.GetSize(),
                 "RegionGpuBuffer::ReleasePendingBuffers",
                 "frame index out of bounds", "frame_index", frame_index,
                 "buffer_count", pending_buffer_destroys_.GetSize());

    auto& pending_buffers{pending_buffer_destroys_[frame_index]};

    for (auto& buffer : pending_buffers) {
      if (IsBufferInitialized(buffer)) {
        DestroyBuffer(buffer);
      }
    }

    pending_buffers.Clear();
  }

  const Buffer& GetBuffer() const noexcept { return buffer_; }

 protected:
#ifdef COMET_RENDERING_USE_DEBUG_LABELS
  inline static constexpr usize kMaxDebugLabelLen_{31};
  schar debug_label_[kMaxDebugLabelLen_ + 1]{'\0'};
#endif  // COMET_RENDERING_USE_DEBUG_LABELS
  VkBufferUsageFlags usage_{0};
  Buffer buffer_{};
  Context* context_{nullptr};

 private:
  memory::PlatformAllocator platform_allocator_{
      memory::kEngineMemoryTagRender};

  Array<Array<Buffer>> pending_buffer_destroys_{};

  bool is_initialized_{false};
  VmaMemoryUsage vma_memory_usage_{VMA_MEMORY_USAGE_AUTO};
  VkMemoryPropertyFlags memory_property_flags_{0};
  VmaAllocationCreateFlags vma_flags_{0};
  VkSharingMode sharing_mode_{VK_SHARING_MODE_EXCLUSIVE};
  usize element_block_count_{0};
  usize element_count_{0};
  RegionMap region_map_{};
};

struct VertexGpuBuffer : public RegionGpuBuffer<geometry::SkinnedVertex> {
 public:
  VertexGpuBuffer()
      : VertexGpuBuffer(nullptr, nullptr, kDefaultElementCount_) {}

  VertexGpuBuffer(
      memory::Allocator* allocator, Context* context,
      usize element_block_count = kDefaultElementCount_,
      usize element_count = 0, VkMemoryPropertyFlags memory_property_flags = 0,
      VmaMemoryUsage vma_memory_usage = VMA_MEMORY_USAGE_AUTO,
      VmaAllocationCreateFlags vma_flags = 0,
      VkSharingMode sharing_mode = VK_SHARING_MODE_EXCLUSIVE,
      [[maybe_unused]] const schar* debug_label = "vertex_gpu_buffer")
      : RegionGpuBuffer<geometry::SkinnedVertex>(
            allocator, context, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            element_block_count, element_count, memory_property_flags,
            vma_memory_usage, vma_flags, sharing_mode, debug_label) {}

  void Bind(VkCommandBuffer command_buffer_handle) {
    VkDeviceSize offset{0};
    vkCmdBindVertexBuffers(command_buffer_handle, 0, 1, &this->buffer_.handle,
                           &offset);
  }

 private:
  inline static constexpr usize kDefaultElementCount_{8192};
};

struct IndexGpuBuffer : public RegionGpuBuffer<geometry::Index> {
 public:
  IndexGpuBuffer() : IndexGpuBuffer(nullptr, nullptr, kDefaultElementCount_) {}

  IndexGpuBuffer(memory::Allocator* allocator, Context* context,
                 usize element_block_count = kDefaultElementCount_,
                 usize element_count = 0,
                 VkMemoryPropertyFlags memory_property_flags = 0,
                 VmaMemoryUsage vma_memory_usage = VMA_MEMORY_USAGE_AUTO,
                 VmaAllocationCreateFlags vma_flags = 0,
                 VkSharingMode sharing_mode = VK_SHARING_MODE_EXCLUSIVE,
                 [[maybe_unused]] const schar* debug_label = "index_gpu_buffer")
      : RegionGpuBuffer<geometry::Index>(
            allocator, context, VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
            element_block_count, element_count, memory_property_flags,
            vma_memory_usage, vma_flags, sharing_mode, debug_label) {}

  void Bind(VkCommandBuffer command_buffer_handle) {
    vkCmdBindIndexBuffer(command_buffer_handle, this->buffer_.handle, 0,
                         VK_INDEX_TYPE_UINT32);
  }

 private:
  inline static constexpr usize kDefaultElementCount_{16384};
};
}  // namespace vk
}  // namespace rendering
}  // namespace comet

#endif  // COMET_RENDER_DRIVER_VULKAN_TYPE_VULKAN_REGION_GPU_BUFFER_H_
