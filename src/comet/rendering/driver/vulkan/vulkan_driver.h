// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_VULKAN_VULKAN_DRIVER_H_
#define COMET_COMET_RENDERING_DRIVER_VULKAN_VULKAN_DRIVER_H_

#include "comet_precompile.h"

// Add specific debug header first to log VMA's messages.
#include "comet/rendering/driver/vulkan/vulkan_debug.h"

#include "vk_mem_alloc.h"
#include "vulkan/vulkan.h"

#include "comet/event/event.h"
#include "comet/rendering/driver/driver.h"
#include "comet/rendering/driver/vulkan/vulkan_buffer.h"
#include "comet/rendering/driver/vulkan/vulkan_common_types.h"
#include "comet/rendering/driver/vulkan/vulkan_debug.h"
#include "comet/rendering/driver/vulkan/vulkan_descriptor.h"
#include "comet/rendering/driver/vulkan/vulkan_device.h"
#include "comet/rendering/driver/vulkan/vulkan_initializers.h"
#include "comet/rendering/driver/vulkan/vulkan_image.h"
#include "comet/rendering/driver/vulkan/vulkan_material.h"
#include "comet/rendering/driver/vulkan/vulkan_mesh.h"
#include "comet/rendering/driver/vulkan/vulkan_proxy.h"
#include "comet/rendering/driver/vulkan/vulkan_swapchain.h"
#include "comet/rendering/rendering_common.h"
#include "comet/rendering/window/glfw/vulkan/vulkan_glfw_window.h"
#include "comet/resource/model_resource.h"
#include "comet/resource/texture_resource.h"

namespace comet {
namespace rendering {
namespace vk {
class VulkanDriver : public Driver {
 public:
  VulkanDriver();
  VulkanDriver(const VulkanDriver&) = delete;
  VulkanDriver(VulkanDriver&&) = delete;
  VulkanDriver& operator=(const VulkanDriver&) = delete;
  VulkanDriver& operator=(VulkanDriver&&) = delete;
  ~VulkanDriver() = default;

  void Initialize() override;
  void Destroy() override;

  void Update(time::Interpolation interpolation,
              entity::EntityManager& entity_manager) override;

  void SetSize(WindowSize width, WindowSize height);

  bool IsInitialized() const override;
  Window& GetWindow() override;
  VkRenderPass GetDefaultRenderPass() const noexcept;
  const VulkanDevice& GetDevice() const noexcept;

 private:
  f32 clear_color_[4]{0.0f, 0.0f, 0.0f, 1.0f};
  static const std::vector<const char*> kDeviceExtensions_;
  static constexpr auto kDefaultMaxObjectCount_{10000};

  bool is_initialized_{false};
  bool is_vsync_{false};
  u8 max_frames_in_flight_{2};
  u8 current_frame_{0};
  uindex max_object_count_{kDefaultMaxObjectCount_};
  VulkanGlfwWindow window_;
  VulkanDevice device_{};
  VkInstance instance_{VK_NULL_HANDLE};
  VmaAllocator allocator_{VK_NULL_HANDLE};
  VulkanSwapchain swapchain_{};
  VulkanDescriptorSetLayoutHandler descriptor_set_layout_handler_{};
  std::vector<VkFramebuffer> frame_buffers_;
  AllocatedImage allocated_color_image_{};
  VkImageView color_image_view_{VK_NULL_HANDLE};
  AllocatedImage allocated_depth_image_{};
  VkImageView depth_image_view_{VK_NULL_HANDLE};
  VkRenderPass render_pass_{VK_NULL_HANDLE};
  std::vector<FrameData> frame_data_{};
  UploadContext upload_context_{};
  VkCommandPool transfer_command_pool_{VK_NULL_HANDLE};
  VulkanDescriptorAllocator descriptor_allocator_{};
  VkDescriptorSetLayout global_descriptor_set_layout_{VK_NULL_HANDLE};
  VkDescriptorSetLayout object_descriptor_set_layout_{VK_NULL_HANDLE};
  VkDescriptorSetLayout single_texture_descriptor_set_layout_{VK_NULL_HANDLE};
  SceneData scene_data_;
  VkPipelineLayout pipeline_layout_{VK_NULL_HANDLE};
  VkPipeline graphics_pipeline_{VK_NULL_HANDLE};
  VkSampler texture_sampler_{VK_NULL_HANDLE};
  VulkanMaterialHandler material_handler_{};

  std::vector<VulkanRenderProxy> proxies_{};
  std::unordered_map<VulkanMaterialId, VulkanMaterial> materials_{};
  std::unordered_map<VulkanMeshId, VulkanMesh> meshes_{};
  std::unordered_map<VulkanTextureId, VulkanTexture> textures_{};

  void InitializeVulkanInstance();
  void InitializeAllocator();
  void InitializeDefaultRenderPass();
  void InitializeCommands();
  void InitializeSamplers();
  void InitializeColorResources();
  void InitializeDepthResources();
  void InitializeFrameBuffers();
  void InitializeSyncStructures();
  void InitializeDescriptors();
  void InitializePipelines();

  void DestroyRenderProxies();
  void DestroyTextures();
  void DestroyPipelines();
  void DestroyDescriptors();
  void DestroySyncStructures();
  void DestroyFrameBuffers();
  void DestroyDepthResources();
  void DestroyColorResources();
  void DestroySamplers();
  void DestroyCommands();
  void DestroyDefaultRenderPass();
  void DestroyAllocator();
  void DestroyInstance();

  void OnEvent(const event::Event&);
  void ApplyWindowResize();

  bool PreDraw();
  void PostDraw();
  void Draw();
  void DrawRenderProxies(VkCommandBuffer command_buffer);

  VulkanMesh* AddVulkanMesh(const resource::MeshResource* resource);

  VulkanMesh* TryGetVulkanMesh(VulkanMeshId mesh_id);
  VulkanMesh* GetVulkanMesh(VulkanMeshId mesh_id);
  VulkanTexture* UploadVulkanTexture(const resource::TextureResource* resource);
  void GenerateMipmaps(const VulkanTexture& texture);

  VulkanRenderProxy* TryGetVulkanRenderProxy(
      const resource::MeshResource* resource);

  std::vector<const char*> GetRequiredExtensions();
  void UploadVulkanMesh(VulkanMesh& mesh);
  void CopyBuffer(VkBuffer src_buffer, VkBuffer dst_buffer, VkDeviceSize size);
  void CopyBufferToImage(CommandBuffer& command_buffer, VkBuffer buffer,
                         VkImage image, u32 width, u32 height);
  void TransitionImageLayout(
      const CommandBuffer& command_buffer, VkImage image, VkFormat format,
      VkImageLayout old_layout, VkImageLayout new_layout, u32 mip_levels,
      u32 src_queue_family_index = VK_QUEUE_FAMILY_IGNORED,
      u32 dst_queue_family_index = VK_QUEUE_FAMILY_IGNORED);
  VulkanMaterialDescr GenerateVulkanMaterial(
      const resource::MaterialResource& resource);

  bool IsVulkanMesh(const resource::MeshResource* resource);
  VkCommandPool GetTransferCommandPool();

#ifdef COMET_VULKAN_DEBUG_MODE
  static constexpr std::array<const char*, 1> kValidationLayers_{
      "VK_LAYER_KHRONOS_validation"};
  VkDebugUtilsMessengerEXT debug_messenger_{VK_NULL_HANDLE};

  void InitializeDebugMessenger();
  void DestroyDebugMessenger();
  bool AreValidationLayersSupported();
  static VKAPI_ATTR VkBool32 VKAPI_CALL LogVulkanValidationMessage(
      VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
      VkDebugUtilsMessageTypeFlagsEXT message_type,
      const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
      void* user_data);
#endif  // COMET_VULKAN_DEBUG_MODE
};
}  // namespace vk
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_VULKAN_VULKAN_DRIVER_H_
