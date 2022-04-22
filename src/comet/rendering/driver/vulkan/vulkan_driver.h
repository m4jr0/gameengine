// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_VULKAN_VULKAN_DRIVER_H_
#define COMET_COMET_RENDERING_DRIVER_VULKAN_VULKAN_DRIVER_H_

#include "comet_precompile.h"

#include "vk_mem_alloc.h"
#include "vulkan/vulkan.h"

#include "comet/event/event.h"
#include "comet/rendering/driver/driver.h"
#include "comet/rendering/driver/vulkan/vulkan_initializers.h"
#include "comet/rendering/driver/vulkan/vulkan_mesh.h"
#include "comet/rendering/window/glfw/vulkan/vulkan_glfw_window.h"

namespace comet {
namespace rendering {
namespace vk {
struct VulkanDriverDescr : DriverDescr {
  bool is_specific_transfer_queue_requested = true;
  unsigned int max_frames_in_flight = 2;
};

class VulkanDriver : public Driver {
 public:
  explicit VulkanDriver(const VulkanDriverDescr& descr);
  VulkanDriver(const VulkanDriver&) = delete;
  VulkanDriver(VulkanDriver&&) = delete;
  VulkanDriver& operator=(const VulkanDriver&) = delete;
  VulkanDriver& operator=(VulkanDriver&&) = delete;
  virtual ~VulkanDriver() = default;

  virtual void Initialize() override;
  virtual void Destroy() override;
  virtual void Update(
      time::Interpolation interpolation,
      game_object::GameObjectManager& game_object_manager) override;

  void LoadShaderModule(std::string& path, VkShaderModule* out);
  void LoadMeshes();
  void LoadImages();
  void UploadMesh(Mesh& mesh);

  void SetSize(unsigned int width, unsigned int height);
  void OnEvent(const event::Event&);

  virtual bool IsInitialized() const override;
  virtual Window& GetWindow() override;

 private:
  static const std::vector<const char*> kDeviceExtensions_;

  bool is_initialized_{false};
  bool is_specific_transfer_queue_requested_{true};
  unsigned int max_frames_in_flight_{2};
  unsigned int current_frame_{0};
  VulkanGlfwWindow window_;
  VkInstance instance_{VK_NULL_HANDLE};
  VkDevice device_{VK_NULL_HANDLE};
  VmaAllocator allocator_{VK_NULL_HANDLE};
  PhysicalDeviceDescr physical_device_descr_;
  QueueFamilyIndices queue_family_indices_;
  VkQueue graphics_queue_{VK_NULL_HANDLE};  // Will be destroyed automatically.
  VkQueue present_queue_{VK_NULL_HANDLE};   // Will be destroyed automatically.
  VkQueue transfer_queue_{VK_NULL_HANDLE};  // Will be destroyed automatically.
  VkSurfaceKHR surface_{VK_NULL_HANDLE};
  VkSwapchainKHR swapchain_{VK_NULL_HANDLE};
  std::vector<VkImage> swapchain_images_{};  // Will be destroyed automatically.
  VkFormat swapchain_image_format_;
  VkExtent2D swapchain_extent_{};
  std::vector<VkFramebuffer> swapchain_frame_buffers_;
  std::vector<VkImageView> swapchain_image_views_;
  AllocatedImage allocated_color_image_{};
  VkImageView color_image_view_{VK_NULL_HANDLE};
  AllocatedImage allocated_depth_image_{};
  VkImageView depth_image_view_{VK_NULL_HANDLE};
  VkRenderPass render_pass_{VK_NULL_HANDLE};
  std::vector<FrameData> frame_data_{};
  UploadContext upload_context_{};
  VkCommandPool transfer_command_pool_{VK_NULL_HANDLE};

  std::vector<const char*> GetRequiredExtensions();
  QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);
  PhysicalDeviceDescr GetPhysicalDeviceDescription(VkPhysicalDevice device);
  SwapChainSupportDetails QuerySwapChainSupportDetails(VkPhysicalDevice device);
  void CreateImage(std::uint32_t width, std::uint32_t height,
                   std::uint32_t mip_levels, VkSampleCountFlagBits num_samples,
                   VkFormat format, VkImageTiling tiling,
                   VkImageUsageFlags usage, VkMemoryPropertyFlags properties,
                   AllocatedImage& allocated_image);
  VkImageView CreateImageView(VkImage image, VkFormat format,
                              VkImageAspectFlags aspect_flags,
                              std::uint32_t mip_levels);

  void InitializeSurface();
  void InitializeDevice();
  void InitializeVulkanInstance();
  void InitializeAllocator();
  void InitializeSwapchain();
  void InitializeSwapchainImageViews();
  void InitializeDefaultRenderPass();
  void InitializeCommands();
  void InitializeColorResources();
  void InitializeDepthResources();
  void InitializeFrameBuffers();
  void InitializeSyncStructures();
  void InitializeGraphicsPipeline();
  void InitializePipelines();
  void InitializeScene();
  void InitializeDescriptors();

  void DestroyGraphicsPipeline();
  void DestroySyncStructures();
  void DestroyFrameBuffers();
  void DestroyDepthResources();
  void DestroyColorResources();
  void DestroyCommands();
  void DestroyDefaultRenderPass();
  void DestroyImageViews();
  void DestroySwapchain();
  void DestroyAllocator();
  void DestroyDevice();
  void DestroySurface();
  void DestroyInstance();

  void ChoosePhysicalDevice();
  VkSurfaceFormatKHR ChooseSwapSurfaceFormat(
      const std::vector<VkSurfaceFormatKHR>& formats);
  VkPresentModeKHR ChooseSwapPresentMode(
      const std::vector<VkPresentModeKHR>& present_modes);
  VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
  VkFormat ChooseFormat(const std::vector<VkFormat>& candidates,
                        VkImageTiling tiling, VkFormatFeatureFlags features);
  VkFormat ChooseDepthFormat();
  void TransitionImageLayout(
      const CommandBuffer& command_buffer, VkImage image, VkFormat format,
      VkImageLayout old_layout, VkImageLayout new_layout,
      std::uint32_t mip_levels,
      std::uint32_t src_queue_family_index = VK_QUEUE_FAMILY_IGNORED,
      std::uint32_t dst_queue_family_index = VK_QUEUE_FAMILY_IGNORED);

  VkQueue GetTransferQueue();
  VkCommandPool GetTransferCommandPool();

#ifndef NDEBUG
  static const std::vector<const char*> kValidationLayers_;
  VkDebugUtilsMessengerEXT debug_messenger_{VK_NULL_HANDLE};

  void InitializeDebugMessenger();
  void DestroyDebugMessenger();
  bool AreValidationLayersSupported();
  static VKAPI_ATTR VkBool32 VKAPI_CALL LogVulkanValidationMessage(
      VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
      VkDebugUtilsMessageTypeFlagsEXT message_type,
      const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
      void* user_data);
#endif  // !NDEBUG
};
}  // namespace vk
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_VULKAN_VULKAN_DRIVER_H_
