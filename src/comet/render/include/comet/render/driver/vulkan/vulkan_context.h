#ifndef COMET_RENDER_DRIVER_VULKAN_VULKAN_CONTEXT_H_
#define COMET_RENDER_DRIVER_VULKAN_VULKAN_CONTEXT_H_

// External. ///////////////////////////////////////////////////////////////////
#include "vma/vk_mem_alloc.h"
#include "vulkan/vulkan.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/essentials.h"
#include "comet/core/memory/allocator/platform_allocator.h"
#include "comet/core/memory/memory.h"
#include "comet/core/type/array.h"
#include "comet/rendering/driver/vulkan/type/vulkan_frame.h"
#include "comet/rendering/driver/vulkan/vulkan_device.h"

namespace comet {
namespace rendering {
namespace vk {
struct ContextDescr {
  u8 vulkan_major_version{0};
  u8 vulkan_minor_version{0};
  u8 vulkan_patch_version{0};
  u8 vulkan_variant_version{0};
  FrameInFlightIndex max_frames_in_flight{2};
  bool is_sampler_anisotropy{false};
  bool is_sample_rate_shading{false};
  usize max_object_count{0};
  VkInstance instance_handle{VK_NULL_HANDLE};
  const Device* device{nullptr};
};

class Context {
 public:
  Context() = delete;
  explicit Context(const ContextDescr& descr);
  Context(const Context&) = delete;
  Context(Context&&) = delete;
  Context& operator=(const Context&) = delete;
  Context& operator=(Context&&) = delete;
  ~Context();

  void Initialize();

  void InitializeAllocator();
  void InitializeFrameData();
  void InitializeCommands();
  void InitializeSyncStructures();

  void BindImageData(const ImageData* image_data);
  void UnbindImageData();

  void Destroy();

  void DestroyAllocator();
  void DestroyFrameData();
  void DestroyCommands();
  void DestroySyncStructures();

  void HandlePreSwapchainReload();
  void HandlePostSwapchainReload();

  void GoToNextFrame() noexcept;

  FrameData& GetFrameData(
      FrameInFlightIndex frame = kInvalidFrameInFlightIndex);
  const FrameData& GetFrameData(
      FrameInFlightIndex frame = kInvalidFrameInFlightIndex) const;

  u8 GetVulkanMajorVersion() const noexcept;
  u8 GetVulkanMinorVersion() const noexcept;
  u8 GetVulkanPatchVersion() const noexcept;
  u8 GetVulkanVariantVersion() const noexcept;

  usize GetMaxObjectCount() const noexcept;

  bool IsSamplerAnisotropy() const noexcept;
  bool IsSampleRateShading() const noexcept;

  ImageIndex GetImageIndex() const;
  ImageIndex GetImageCount() const;

  VkSemaphore GetRenderSemaphoreHandle() const;

  FrameIndex GetFrameCount() const noexcept;

  FrameInFlightIndex GetFrameInFlightIndex() const noexcept;
  FrameInFlightIndex GetMaxFramesInFlight() const noexcept;

  VkInstance GetInstanceHandle() const noexcept;

  const Device& GetDevice() const noexcept;
  VkPhysicalDevice GetPhysicalDeviceHandle() const noexcept;

  VmaAllocator GetAllocatorHandle() const noexcept;

  VkCommandPool GetGraphicsCommandPoolHandle(
      FrameInFlightIndex frame_index = kInvalidFrameInFlightIndex) const;

  VkCommandPool GetUploadCommandPoolHandle(
      FrameInFlightIndex frame_index = kInvalidFrameInFlightIndex) const;
  const QueueContext& GetUploadQueueContext() const noexcept;
  const VkSemaphore* GetUploadSemaphoreHandle() const;

  VkQueue GetGraphicsQueueHandle() const noexcept;
  VkQueue GetPresentQueueHandle() const noexcept;
  VkQueue GetTransferQueueHandle() const noexcept;
  VkQueue GetUploadQueueHandle() const noexcept;

  u64 GetUploadTimelineValue() const noexcept;
  u64 AdvanceUploadTimelineValue() noexcept;

  VkCommandBuffer GetUploadCommandBufferHandle(
      FrameInFlightIndex frame_index = kInvalidFrameInFlightIndex) const;
  VkFence GetUploadFenceHandle(
      FrameInFlightIndex frame_index = kInvalidFrameInFlightIndex) const;

  bool IsInitialized() const noexcept;

 private:
  u8 vulkan_major_version_{0};
  u8 vulkan_minor_version_{0};
  u8 vulkan_patch_version_{0};
  u8 vulkan_variant_version_{0};

  bool is_initialized_{false};
  bool is_sampler_anisotropy_{false};
  bool is_sample_rate_shading_{false};

  FrameIndex frame_count_{0};
  FrameInFlightIndex frame_in_flight_index_{0};
  FrameInFlightIndex max_frames_in_flight_{2};

  u64 upload_timeline_value_{0};
  usize max_object_count_{0};

  memory::PlatformAllocator allocator_{memory::kEngineMemoryTagRendering};
  Array<FrameData> frame_data_{};

  VkInstance instance_handle_{VK_NULL_HANDLE};
  VmaAllocator allocator_handle_{VK_NULL_HANDLE};
  VkCommandPool upload_command_pool_handle_{VK_NULL_HANDLE};
  VkSemaphore upload_semaphore_handle_{VK_NULL_HANDLE};

  const ImageData* image_data_{nullptr};
  const Device* device_{nullptr};
};
}  // namespace vk
}  // namespace rendering
}  // namespace comet

#endif  // COMET_RENDER_DRIVER_VULKAN_VULKAN_CONTEXT_H_