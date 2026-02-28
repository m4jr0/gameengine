// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_VULKAN_VULKAN_DRIVER_H_
#define COMET_COMET_RENDERING_DRIVER_VULKAN_VULKAN_DRIVER_H_

// External. ///////////////////////////////////////////////////////////////////
#include "vulkan/vulkan.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/essentials.h"

// Add specific debug header first to log VMA's messages. //////////////////////
#include "comet/rendering/driver/vulkan/vulkan_debug.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/frame/frame_packet.h"
#include "comet/core/frame/frame_utils.h"
#include "comet/core/memory/memory.h"
#include "comet/core/type/array.h"
#include "comet/event/event.h"
#include "comet/rendering/driver/driver.h"
#include "comet/rendering/driver/vulkan/data/vulkan_command_buffer.h"
#include "comet/rendering/driver/vulkan/handler/vulkan_descriptor_handler.h"
#include "comet/rendering/driver/vulkan/handler/vulkan_material_handler.h"
#include "comet/rendering/driver/vulkan/handler/vulkan_mesh_handler.h"
#include "comet/rendering/driver/vulkan/handler/vulkan_pipeline_handler.h"
#include "comet/rendering/driver/vulkan/handler/vulkan_render_pass_handler.h"
#include "comet/rendering/driver/vulkan/handler/vulkan_render_proxy_handler.h"
#include "comet/rendering/driver/vulkan/handler/vulkan_shader_handler.h"
#include "comet/rendering/driver/vulkan/handler/vulkan_shader_module_handler.h"
#include "comet/rendering/driver/vulkan/handler/vulkan_texture_handler.h"
#include "comet/rendering/driver/vulkan/handler/vulkan_view_handler.h"
#include "comet/rendering/driver/vulkan/vulkan_context.h"
#include "comet/rendering/window/glfw/vulkan/vulkan_glfw_window.h"

namespace comet {
namespace rendering {
namespace vk {
struct VulkanDriverDescr : DriverDescr {
  u8 vulkan_major_version{0};
  u8 vulkan_minor_version{0};
  u8 vulkan_patch_version{0};
  u8 vulkan_variant_version{0};
  u8 max_frames_in_flight{0};
};

class VulkanDriver : public Driver {
 public:
  explicit VulkanDriver(const VulkanDriverDescr& descr);
  VulkanDriver(const VulkanDriver&) = delete;
  VulkanDriver(VulkanDriver&&) = delete;
  VulkanDriver& operator=(const VulkanDriver&) = delete;
  VulkanDriver& operator=(VulkanDriver&&) = delete;
  virtual ~VulkanDriver() = default;

  void Initialize() override;
  void Shutdown() override;
  void Update(frame::FramePacket* packet) override;
  DriverType GetType() const noexcept override;

  void SetSize(WindowSize width, WindowSize height);
  Window* GetWindow() override;
  u32 GetDrawCount() const override;

 private:
  void InitializeVulkanInstance();
  void InitializeHandlers();

  void DestroyHandlers();
  void DestroyInstance();

  void ApplyWindowResize();

  void PreDraw(const frame::FramePacket* packet);
  void PostDraw(const frame::FramePacket* packet);
  void Draw(frame::FramePacket* packet);

  void PrepareRenderBarriers(const CommandData& command_data);
  void PrepareViewportAndScissor(const CommandData& command_data);

  frame::FrameArray<const schar*> GetRequiredExtensions();

  static constexpr auto kDefaultMaxObjectCount_{10000};

  u8 vulkan_major_version_{0};
  u8 vulkan_minor_version_{0};
  u8 vulkan_patch_version_{0};
  u8 vulkan_variant_version_{0};
  u8 max_frames_in_flight_{2};

  memory::UniquePtr<VulkanGlfwWindow> window_{nullptr};
  memory::UniquePtr<Swapchain> swapchain_{nullptr};
  memory::UniquePtr<Context> context_{nullptr};

  VkInstance instance_handle_{VK_NULL_HANDLE};
  memory::UniquePtr<Device> device_{nullptr};
  memory::UniquePtr<DescriptorHandler> descriptor_handler_{nullptr};
  memory::UniquePtr<MaterialHandler> material_handler_{nullptr};
  memory::UniquePtr<MeshHandler> mesh_handler_{nullptr};
  memory::UniquePtr<PipelineHandler> pipeline_handler_{nullptr};
  memory::UniquePtr<RenderPassHandler> render_pass_handler_{nullptr};
  memory::UniquePtr<RenderProxyHandler> render_proxy_handler_{nullptr};
  memory::UniquePtr<ShaderHandler> shader_handler_{nullptr};
  memory::UniquePtr<ShaderModuleHandler> shader_module_handler_{nullptr};
  memory::UniquePtr<TextureHandler> texture_handler_{nullptr};
  memory::UniquePtr<ViewHandler> view_handler_{nullptr};

#ifdef COMET_DEBUG_RENDERING
  void InitializeDebugMessenger();
  void DestroyDebugMessenger();
  void InitializeDebugReportCallback();
  void DestroyDebugReportCallback();
  bool AreValidationLayersSupported();
  static VKAPI_ATTR VkBool32 VKAPI_CALL LogVulkanValidationMessage(
      VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
      VkDebugUtilsMessageTypeFlagsEXT message_type,
      const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
      void* user_data);
  static VKAPI_ATTR VkBool32 VKAPI_CALL LogVulkanDebugReportMessage(
      VkFlags message_flags, VkDebugReportObjectTypeEXT object_type,
      u64 source_object, usize location, int32_t message_code,
      const schar* layer_prefix, const schar* message, void* user_data);

  static constexpr StaticArray<const schar*, 1> kValidationLayers_{
      "VK_LAYER_KHRONOS_validation"};
  VkDebugUtilsMessengerEXT debug_messenger_handle_{VK_NULL_HANDLE};
  VkDebugReportCallbackEXT debug_report_callback_handle_{VK_NULL_HANDLE};
#endif  // COMET_DEBUG_RENDERING
};
}  // namespace vk
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_VULKAN_VULKAN_DRIVER_H_
