// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_VULKAN_DATA_VULKAN_REGION_GPU_BUFFER_H_
#define COMET_COMET_RENDERING_DRIVER_VULKAN_DATA_VULKAN_REGION_GPU_BUFFER_H_

#include "vma/vk_mem_alloc.h"
#include "vulkan/vulkan.h"

#include "comet/core/essentials.h"
#include "comet/core/memory/allocator/allocator.h"
#include "comet/core/memory/memory_utils.h"
#include "comet/core/type/region_map.h"
#include "comet/profiler/profiler.h"
#include "comet/rendering/driver/vulkan/data/vulkan_buffer.h"
#include "comet/rendering/driver/vulkan/utils/vulkan_buffer_utils.h"
#include "comet/rendering/driver/vulkan/vulkan_context.h"

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
    COMET_ASSERT(!is_initialized_,
                 "Destructor called for region GPU buffer, but it is still "
                 "initialized!");
  }

  void Initialize() {
    COMET_ASSERT(
        !is_initialized_,
        "Tried to initialize region GPU buffer, but it is already done!");
    Resize(element_count_);
    is_initialized_ = true;
  }

  void Destroy() {
    COMET_ASSERT(
        is_initialized_,
        "Tried to destroy region GPU buffer, but it is not initialized!");
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
      COMET_LOG_RENDERING_WARNING(
          "Region GPU buffer has to be resized from ",
          element_count_ * sizeof(T), " to ", new_element_count * sizeof(T),
          " bytes. This will cause performance issues.");
      Resize(new_element_count);
      block_offset = region_map_.Claim(claimed_size);
    }

    COMET_ASSERT(block_offset != kInvalidSize,
                 "Not enough memory for region GPU buffer!");
    return static_cast<u32>(block_offset / sizeof(T));
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

    auto& device{context_->GetDevice()};
    element_count_ = new_element_count;
    auto buffer_size{element_count_ * sizeof(T)};

    ResizeBuffer(
        buffer_, device, context_->GetTransferCommandPoolHandle(),
        context_->GetAllocatorHandle(), static_cast<VkDeviceSize>(buffer_size),
        usage_, vma_memory_usage_, device.GetTransferQueueHandle(),
        memory_property_flags_, vma_flags_, sharing_mode_, VK_NULL_HANDLE
#ifdef COMET_RENDERING_USE_DEBUG_LABELS
        ,
        debug_label_
#endif  // COMET_RENDERING_USE_DEBUG_LABELS
    );

    region_map_.Resize(buffer_size);
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

#endif  // COMET_COMET_RENDERING_DRIVER_VULKAN_DATA_VULKAN_REGION_GPU_BUFFER_H_
