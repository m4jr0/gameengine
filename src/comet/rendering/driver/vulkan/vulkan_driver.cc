// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#define VMA_IMPLEMENTATION

#include "vulkan_driver.h"

#include <set>

#include "boost/format.hpp"

#include "comet/core/engine.h"
#include "comet/event/event_manager.h"
#include "comet/event/input_event.h"
#include "comet/event/runtime_event.h"
#include "comet/event/window_event.h"
#include "comet/rendering/driver/vulkan/vulkan_utils.h"

#ifdef _WIN32
#include "debug_windows.h"
#endif  // _WIN32

namespace comet {
namespace rendering {
namespace vk {
const std::vector<const char*> VulkanDriver::kDeviceExtensions_ = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME};

VulkanDriver::VulkanDriver(const VulkanDriverDescr& descr)
    : is_specific_transfer_queue_requested_(
          descr.is_specific_transfer_queue_requested),
      max_frames_in_flight_(descr.max_frames_in_flight) {
  WindowDescr window_descr{};
  window_descr.width = descr.width;
  window_descr.height = descr.height;
  window_descr.name = descr.name;
  window_ = VulkanGlfwWindow(window_descr);
  frame_data_.resize(max_frames_in_flight_);
}

void VulkanDriver::Initialize() {
  COMET_LOG_RENDERING_DEBUG("Initializing Vulkan driver.");
  window_.Initialize();

  if (!window_.IsInitialized()) {
    return;
  }

  event::EventManager& event_manager =
      core::Engine::GetEngine().GetEventManager();

  event_manager.Register(COMET_EVENT_BIND_FUNCTION(VulkanDriver::OnEvent),
                         event::WindowResizeEvent::kStaticType_);

  InitializeVulkanInstance();
#ifndef NDEBUG
  InitializeDebugMessenger();
#endif  // !NDEBUG
  InitializeSurface();
  ChoosePhysicalDevice();
  InitializeDevice();
  InitializeAllocator();
  InitializeSwapchain();
  InitializeDefaultRenderPass();
  InitializeCommands();
  InitializeColorResources();
  InitializeDepthResources();
  InitializeFrameBuffers();
  InitializeGraphicsPipeline();

  is_initialized_ = true;
}

void VulkanDriver::Destroy() {
  is_initialized_ = false;

  if (device_ != VK_NULL_HANDLE) {
    vkDeviceWaitIdle(device_);
  }

  DestroyGraphicsPipeline();
  DestroyFrameBuffers();
  DestroyDepthResources();
  DestroyColorResources();
  DestroyCommands();
  DestroyDefaultRenderPass();
  DestroySwapchain();
  DestroyAllocator();
  DestroyDevice();
  DestroySurface();
#ifndef NDEBUG
  DestroyDebugMessenger();
#endif  // !NDEBUG
  DestroyInstance();

  if (window_.IsInitialized()) {
    window_.Destroy();
  }
}

void VulkanDriver::Update(time::Interpolation interpolation,
                          game_object::GameObjectManager& game_object_manager) {
  // Code.
}

void VulkanDriver::LoadShaderModule(std::string& path, VkShaderModule* out) {
  // Code.
}

void VulkanDriver::LoadMeshes() {
  // Code.
}

void VulkanDriver::LoadImages() {
  // Code.
}

void VulkanDriver::UploadMesh(Mesh& mesh) {
  // Code.
}

void VulkanDriver::SetSize(unsigned int width, unsigned int height) {
  window_.SetSize(width, height);

  // Code.
}

void VulkanDriver::OnEvent(const event::Event& event) {
  const auto& event_type = event.GetType();

  if (event_type == event::WindowResizeEvent::kStaticType_) {
    const auto& window_event =
        static_cast<const event::WindowResizeEvent&>(event);
    SetSize(window_event.GetWidth(), window_event.GetHeight());
  }
}

bool VulkanDriver::IsInitialized() const { return is_initialized_; }

Window& VulkanDriver::GetWindow() { return window_; }

std::vector<const char*> VulkanDriver::GetRequiredExtensions() {
  std::uint32_t glfw_extension_count = 0;
  const char** glfw_extensions =
      glfwGetRequiredInstanceExtensions(&glfw_extension_count);

  std::vector<const char*> extensions(glfw_extensions,
                                      glfw_extensions + glfw_extension_count);

#ifndef NDEBUG
  extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif  // !NDEBUG

  return extensions;
}

QueueFamilyIndices VulkanDriver::FindQueueFamilies(VkPhysicalDevice device) {
  QueueFamilyIndices indices;

  std::uint32_t queue_family_count = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count,
                                           nullptr);

  if (queue_family_count == 0) {
    return indices;
  }

  std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count,
                                           queue_families.data());

  std::size_t queue_index = 0;

  for (const auto& queue_family : queue_families) {
    VkBool32 is_present_support = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(device, queue_index, surface_,
                                         &is_present_support);
    // At first, we explicitly try to find a queue family specialized for
    // transfer operations.
    if (is_specific_transfer_queue_requested_ &&
        !indices.transfer_family.has_value() &&
        (queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0 &&
        queue_family.queueFlags & VK_QUEUE_TRANSFER_BIT) {
      indices.transfer_family = queue_index;
    }

    if (!indices.graphics_family.has_value() &&
        queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      indices.graphics_family = queue_index;

      if (!indices.transfer_family.has_value() &&
          !is_specific_transfer_queue_requested_) {
        indices.transfer_family = queue_index;
      }
    }

    if (!indices.present_family.has_value() && is_present_support) {
      indices.present_family = queue_index;
    }

    if (indices.IsComplete()) {
      break;
    }

    queue_index++;
  }

  // If no specific queue is found, fall back to the graphics queue family, if
  // possible.
  if (!indices.transfer_family.has_value() &&
      indices.graphics_family.has_value()) {
    indices.transfer_family = indices.graphics_family.value();
  }

  return indices;
}

SwapChainSupportDetails VulkanDriver::QuerySwapChainSupportDetails(
    VkPhysicalDevice device) {
  SwapChainSupportDetails details;

  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface_,
                                            &details.capabilities);

  std::uint32_t format_count;
  vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface_, &format_count,
                                       nullptr);

  if (format_count != 0) {
    details.formats.resize(format_count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface_, &format_count,
                                         details.formats.data());
  }

  std::uint32_t present_mode_count;
  vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface_,
                                            &present_mode_count, nullptr);

  if (present_mode_count != 0) {
    details.present_modes.resize(present_mode_count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(
        device, surface_, &present_mode_count, details.present_modes.data());
  }

  return details;
}

void VulkanDriver::CreateImage(std::uint32_t width, std::uint32_t height,
                               std::uint32_t mip_levels,
                               VkSampleCountFlagBits num_samples,
                               VkFormat format, VkImageTiling tiling,
                               VkImageUsageFlags usage_flags,
                               VkMemoryPropertyFlags properties,
                               AllocatedImage& allocated_image) {
  const std::vector<std::uint32_t> family_indices{
      queue_family_indices_.transfer_family.value(),
      queue_family_indices_.graphics_family.value()};

  VkSharingMode sharing_mode = VK_SHARING_MODE_EXCLUSIVE;
  std::uint32_t queue_family_index_count = 0;
  const std::uint32_t* queue_family_indices_pointer = nullptr;

  const std::vector<std::uint32_t> queue_family_indices{
      queue_family_indices_.transfer_family.value(),
      queue_family_indices_.graphics_family.value()};

  if (queue_family_indices_.IsSpecificTransferFamily()) {
    sharing_mode = VK_SHARING_MODE_CONCURRENT;
    queue_family_index_count = 2;
    queue_family_indices_pointer = queue_family_indices.data();
  }

  auto create_info = init::GetImageCreateInfo(
      width, height, mip_levels, num_samples, format, tiling, usage_flags,
      sharing_mode, queue_family_indices_pointer, queue_family_index_count);

  VmaAllocationCreateInfo alloc_info = {};
  alloc_info.usage = VMA_MEMORY_USAGE_AUTO;
  alloc_info.requiredFlags = properties;

  if (vmaCreateImage(allocator_, &create_info, &alloc_info,
                     &allocated_image.image, &allocated_image.allocation,
                     nullptr) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create image");
  }
}

VkImageView VulkanDriver::CreateImageView(VkImage image, VkFormat format,
                                          VkImageAspectFlags aspect_flags,
                                          std::uint32_t mip_levels) {
  auto create_info =
      init::GetImageViewCreateInfo(image, format, aspect_flags, 1);
  VkImageView image_view;

  if (vkCreateImageView(device_, &create_info, nullptr, &image_view) !=
      VK_SUCCESS) {
    throw std::runtime_error("Failed to create image view");
  }

  return image_view;
}

PhysicalDeviceDescr VulkanDriver::GetPhysicalDeviceDescription(
    VkPhysicalDevice device) {
  PhysicalDeviceDescr descr;
  descr.device = device;
  vkGetPhysicalDeviceProperties(device, &descr.properties);
  vkGetPhysicalDeviceFeatures(device, &descr.features);
  descr.score = 0;

  // First, mandatory properties and features.
  if (!descr.features.samplerAnisotropy) {
    return descr;
  }

  if (!descr.features.geometryShader) {
    return descr;
  }

  if (!utils::AreDeviceExtensionAvailable(device, kDeviceExtensions_)) {
    return descr;
  }

  auto indices = FindQueueFamilies(device);

  if (!indices.IsComplete()) {
    return descr;
  }

  auto swapchain_support_details = QuerySwapChainSupportDetails(device);

  if (swapchain_support_details.formats.empty() ||
      swapchain_support_details.present_modes.empty()) {
    return descr;
  }

  // Other properties and features.
  if (descr.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
    descr.score += 1000;
  }

  descr.msaa_samples = utils::GetMaxUsableSampleCount(descr.device);
  descr.score += 5 * static_cast<PhysicalDeviceScore>(descr.msaa_samples);
  descr.score += descr.properties.limits.maxImageDimension2D;

  return descr;
}

void VulkanDriver::InitializeSurface() {
  window_.InitializeSurface(instance_, surface_);
}

void VulkanDriver::InitializeDevice() {
  auto unique_queue_family_indices = queue_family_indices_.GetUniqueIndices();
  std::vector<VkDeviceQueueCreateInfo> queue_create_info{};
  auto queue_priority = 1.0f;

  for (auto queue_family_index : unique_queue_family_indices) {
    queue_create_info.push_back(
        init::GetDeviceQueueCreateInfo(queue_family_index, queue_priority));
  }

  VkPhysicalDeviceFeatures device_features{};
  device_features.samplerAnisotropy = VK_TRUE;
  device_features.sampleRateShading = VK_TRUE;

  auto create_info = init::GetDeviceCreateInfo(
      queue_create_info, device_features, kDeviceExtensions_);

  if (vkCreateDevice(physical_device_descr_.device, &create_info, nullptr,
                     &device_) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create logical device");
  }

  vkGetDeviceQueue(device_, queue_family_indices_.graphics_family.value(), 0,
                   &graphics_queue_);

  vkGetDeviceQueue(device_, queue_family_indices_.present_family.value(), 0,
                   &present_queue_);

  if (!queue_family_indices_.IsSpecificTransferFamily()) {
    return;
  }

  vkGetDeviceQueue(device_, queue_family_indices_.transfer_family.value(), 0,
                   &transfer_queue_);
}

void VulkanDriver::InitializeVulkanInstance() {
  COMET_LOG_RENDERING_DEBUG("Initializing Vulkan instance.");
  unsigned int extension_count = 0;
  vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);
  std::vector<VkExtensionProperties> extensions(extension_count);
  vkEnumerateInstanceExtensionProperties(nullptr, &extension_count,
                                         extensions.data());

  COMET_LOG_RENDERING_DEBUG("Available Vulkan extensions:");

  for (const auto& extension : extensions) {
    COMET_LOG_RENDERING_DEBUG("\t", extension.extensionName);
  }

  const auto& conf = core::Engine::GetEngine().GetConfigurationManager();
  VkApplicationInfo app_info{};

  app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;

  auto app_name = conf.Get<std::string>("application_name");
  auto app_major_version = static_cast<std::uint32_t>(
      conf.Get<unsigned int>("application_major_version"));
  auto app_minor_version = static_cast<std::uint32_t>(
      conf.Get<unsigned int>("application_minor_version"));
  auto app_patch_version = static_cast<std::uint32_t>(
      conf.Get<unsigned int>("application_patch_version"));

  app_info.pApplicationName = app_name.c_str();
  app_info.applicationVersion = VK_MAKE_API_VERSION(
      0, app_major_version, app_minor_version, app_patch_version);

  auto engine_name = conf.Get<std::string>("engine_name");
  auto engine_major_version = static_cast<std::uint32_t>(
      conf.Get<unsigned int>("engine_major_version"));
  auto engine_minor_version = static_cast<std::uint32_t>(
      conf.Get<unsigned int>("engine_minor_version"));
  auto engine_patch_version = static_cast<std::uint32_t>(
      conf.Get<unsigned int>("engine_patch_version"));

  app_info.pEngineName = engine_name.c_str();
  app_info.engineVersion = VK_MAKE_API_VERSION(
      0, engine_major_version, engine_minor_version, engine_patch_version);

  auto vk_variant_version = static_cast<std::uint32_t>(
      conf.Get<unsigned int>("rendering_vulkan_variant_version"));
  auto vk_major_version = static_cast<std::uint32_t>(
      conf.Get<unsigned int>("rendering_vulkan_major_version"));
  auto vk_minor_version = static_cast<std::uint32_t>(
      conf.Get<unsigned int>("rendering_vulkan_minor_version"));
  auto vk_patch_version = static_cast<std::uint32_t>(
      conf.Get<unsigned int>("rendering_vulkan_patch_version"));

  app_info.apiVersion = VK_MAKE_API_VERSION(
      vk_variant_version, vk_major_version, vk_minor_version, vk_patch_version);

  VkInstanceCreateInfo create_info{};
  create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  create_info.pApplicationInfo = &app_info;
#ifndef NDEBUG
  VkValidationFeatureEnableEXT enabled_validation_features[] = {
      VK_VALIDATION_FEATURE_ENABLE_DEBUG_PRINTF_EXT};
  VkValidationFeaturesEXT validation_features = {};
  validation_features.sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT;
  validation_features.enabledValidationFeatureCount = 1;
  validation_features.pEnabledValidationFeatures = enabled_validation_features;
  create_info.pNext = &validation_features;
#endif  // !NDEBUG

  const auto required_extensions = GetRequiredExtensions();
  const std::uint32_t required_extension_count =
      static_cast<std::uint32_t>(required_extensions.size());

  COMET_LOG_RENDERING_DEBUG("Required extensions:");

  for (std::size_t i = 0; i < required_extension_count; ++i) {
    const char* required_extension = required_extensions[i];
    COMET_LOG_RENDERING_DEBUG("\t", required_extension);
    bool is_found = false;

    for (const auto& extension : extensions) {
      if (std::strcmp(required_extension, extension.extensionName) == 0) {
        is_found = true;
        break;
      }
    }

    if (!is_found) {
      throw std::runtime_error(
          std::string("Required extension is not available: ") +
          required_extension);
    }
  }

  create_info.enabledExtensionCount = required_extension_count;
  create_info.ppEnabledExtensionNames = required_extensions.data();

#ifdef NDEBUG
  create_info.enabledLayerCount = 0;
  create_info.pNext = nullptr;
#else
  if (!AreValidationLayersSupported()) {
    throw std::runtime_error("At least one validation layer is not available");
  }

  create_info.enabledLayerCount =
      static_cast<std::uint32_t>(kValidationLayers_.size());
  create_info.ppEnabledLayerNames = kValidationLayers_.data();
  auto debug_create_info = init::GetDebugUtilsMessengerCreateInfo(
      VulkanDriver::LogVulkanValidationMessage);
  create_info.pNext =
      static_cast<VkDebugUtilsMessengerCreateInfoEXT*>(&debug_create_info);
#endif

  if (vkCreateInstance(&create_info, nullptr, &instance_) != VK_SUCCESS) {
    Destroy();
    throw std::runtime_error("Failed to create instance");
  }
}

void VulkanDriver::InitializeAllocator() {
  const auto& conf = core::Engine::GetEngine().GetConfigurationManager();

  auto vk_variant_version = static_cast<std::uint32_t>(
      conf.Get<unsigned int>("rendering_vulkan_variant_version"));
  auto vk_major_version = static_cast<std::uint32_t>(
      conf.Get<unsigned int>("rendering_vulkan_major_version"));
  auto vk_minor_version = static_cast<std::uint32_t>(
      conf.Get<unsigned int>("rendering_vulkan_minor_version"));
  auto vk_patch_version = static_cast<std::uint32_t>(
      conf.Get<unsigned int>("rendering_vulkan_patch_version"));

  VmaAllocatorCreateInfo allocatorCreateInfo = {};
  allocatorCreateInfo.vulkanApiVersion = VK_MAKE_API_VERSION(
      vk_variant_version, vk_major_version, vk_minor_version, vk_patch_version);
  allocatorCreateInfo.physicalDevice = physical_device_descr_.device;
  allocatorCreateInfo.device = device_;
  allocatorCreateInfo.instance = instance_;

  vmaCreateAllocator(&allocatorCreateInfo, &allocator_);
}

void VulkanDriver::InitializeSwapchain() {
  auto details = QuerySwapChainSupportDetails(physical_device_descr_.device);

  auto surface_format = ChooseSwapSurfaceFormat(details.formats);
  auto present_mode = ChooseSwapPresentMode(details.present_modes);
  auto extent = ChooseSwapExtent(details.capabilities);

  if (extent.width == 0 || extent.height == 0) {
    return;
  }

  auto image_count = details.capabilities.minImageCount + 1;

  if (details.capabilities.maxImageCount > 0 &&
      image_count > details.capabilities.maxImageCount) {
    image_count = details.capabilities.maxImageCount;
  }

  auto queue_family_unique_indices = queue_family_indices_.GetUniqueIndices();

  auto create_info = init::GetSwapchainCreateInfo(
      surface_, surface_format, extent, present_mode, details,
      queue_family_unique_indices, image_count);

  if (vkCreateSwapchainKHR(device_, &create_info, nullptr, &swapchain_) !=
      VK_SUCCESS) {
    throw std::runtime_error("Failed to create swap chain");
  }

  vkGetSwapchainImagesKHR(device_, swapchain_, &image_count, nullptr);
  swapchain_images_.resize(image_count);
  vkGetSwapchainImagesKHR(device_, swapchain_, &image_count,
                          swapchain_images_.data());

  swapchain_image_format_ = surface_format.format;
  swapchain_extent_ = extent;

  InitializeSwapchainImageViews();
}

void VulkanDriver::InitializeSwapchainImageViews() {
  swapchain_image_views_.resize(swapchain_images_.size());

  for (std::size_t i = 0; i < swapchain_images_.size(); ++i) {
    swapchain_image_views_[i] =
        CreateImageView(swapchain_images_[i], swapchain_image_format_,
                        VK_IMAGE_ASPECT_COLOR_BIT, 1);
  }
}

void VulkanDriver::InitializeDefaultRenderPass() {
  VkAttachmentDescription color_attachment{};
  color_attachment.format = swapchain_image_format_;
  color_attachment.samples = physical_device_descr_.msaa_samples;
  color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  color_attachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkAttachmentReference color_attachment_ref{};
  color_attachment_ref.attachment = 0;
  color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkAttachmentDescription depth_attachment{};
  depth_attachment.format = ChooseDepthFormat();
  depth_attachment.samples = physical_device_descr_.msaa_samples;
  depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  depth_attachment.finalLayout =
      VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  VkAttachmentReference depth_attachment_ref{};
  depth_attachment_ref.attachment = 1;
  depth_attachment_ref.layout =
      VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  VkAttachmentDescription color_attachment_resolve{};
  color_attachment_resolve.format = swapchain_image_format_;
  color_attachment_resolve.samples = VK_SAMPLE_COUNT_1_BIT;
  color_attachment_resolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  color_attachment_resolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  color_attachment_resolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  color_attachment_resolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  color_attachment_resolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  color_attachment_resolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  VkAttachmentReference color_attachment_resolve_ref{};
  color_attachment_resolve_ref.attachment = 2;
  color_attachment_resolve_ref.layout =
      VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkSubpassDescription subpass{};
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &color_attachment_ref;
  subpass.pDepthStencilAttachment = &depth_attachment_ref;
  subpass.pResolveAttachments = &color_attachment_resolve_ref;

  VkSubpassDependency dependency{};
  dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
  dependency.dstSubpass = 0;
  dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                            VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
  dependency.srcAccessMask = 0;
  dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                            VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
  dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
                             VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

  std::array<VkAttachmentDescription, 3> attachments = {
      color_attachment, depth_attachment, color_attachment_resolve};

  VkRenderPassCreateInfo render_pass_info{};
  render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  render_pass_info.attachmentCount =
      static_cast<std::uint32_t>(attachments.size());
  render_pass_info.pAttachments = attachments.data();
  render_pass_info.subpassCount = 1;
  render_pass_info.pSubpasses = &subpass;
  render_pass_info.dependencyCount = 1;
  render_pass_info.pDependencies = &dependency;

  if (vkCreateRenderPass(device_, &render_pass_info, nullptr, &render_pass_) !=
      VK_SUCCESS) {
    throw std::runtime_error("Failed to create render pass");
  }
}

void VulkanDriver::InitializeCommands() {
  auto pool_info = init::GetCommandPoolCreateInfo(
      queue_family_indices_.graphics_family.value(),
      VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

  for (std::size_t i = 0; i < max_frames_in_flight_; ++i) {
    if (vkCreateCommandPool(device_, &pool_info, nullptr,
                            &frame_data_[i].command_pool) != VK_SUCCESS) {
      throw std::runtime_error("Failed to create frame command pool");
    }

    auto allocate_info =
        init::GetCommandBufferAllocateInfo(frame_data_[i].command_pool, 1);

    if (vkAllocateCommandBuffers(device_, &allocate_info,
                                 &frame_data_[i].command_buffer) !=
        VK_SUCCESS) {
      throw std::runtime_error("Failed to allocate frame command buffer");
    }
  }

  pool_info.flags = 0;

  if (vkCreateCommandPool(device_, &pool_info, nullptr,
                          &upload_context_.command_pool) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create upload command pool");
  }

  if (!queue_family_indices_.IsSpecificTransferFamily()) {
    return;
  }

  pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT |
                    VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
  pool_info.queueFamilyIndex = queue_family_indices_.transfer_family.value();

  if (vkCreateCommandPool(device_, &pool_info, nullptr,
                          &transfer_command_pool_) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create transfer command pool");
  }
}

void VulkanDriver::InitializeColorResources() {
  VkFormat color_format = swapchain_image_format_;

  CreateImage(swapchain_extent_.width, swapchain_extent_.height, 1,
              physical_device_descr_.msaa_samples, color_format,
              VK_IMAGE_TILING_OPTIMAL,
              VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT |
                  VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
              VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, allocated_color_image_);

  // One 1 mip level, since it is Vulkan enforces the usage of only one when
  // there are more than one sample per pixel (and we don't need mimaps anyway).
  color_image_view_ = CreateImageView(
      allocated_color_image_.image, color_format, VK_IMAGE_ASPECT_COLOR_BIT, 1);
}

void VulkanDriver::InitializeDepthResources() {
  auto depth_format = ChooseDepthFormat();

  CreateImage(swapchain_extent_.width, swapchain_extent_.height, 1,
              physical_device_descr_.msaa_samples, depth_format,
              VK_IMAGE_TILING_OPTIMAL,
              VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
              VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, allocated_depth_image_);

  depth_image_view_ = CreateImageView(
      allocated_depth_image_.image, depth_format, VK_IMAGE_ASPECT_DEPTH_BIT, 1);

  CommandBuffer command_buffer{device_,
                               frame_data_[current_frame_].command_pool};
  command_buffer.Allocate();
  command_buffer.Record();

  TransitionImageLayout(command_buffer, allocated_depth_image_.image,
                        depth_format, VK_IMAGE_LAYOUT_UNDEFINED,
                        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 1);

  command_buffer.Submit(graphics_queue_);
  command_buffer.Free();
}

void VulkanDriver::InitializeFrameBuffers() {
  swapchain_frame_buffers_.resize(swapchain_images_.size());

  auto create_info =
      init::GetFrameBufferCreateInfo(render_pass_, swapchain_extent_);
  create_info.width = swapchain_extent_.width;
  create_info.height = swapchain_extent_.height;
  create_info.layers = 1;

  for (std::size_t i = 0; i < swapchain_images_.size(); ++i) {
    std::array<VkImageView, 3> attachments = {
        color_image_view_, depth_image_view_, swapchain_image_views_[i]};

    create_info.attachmentCount =
        static_cast<std::uint32_t>(attachments.size());
    create_info.pAttachments = attachments.data();

    if (vkCreateFramebuffer(device_, &create_info, nullptr,
                            &swapchain_frame_buffers_[i]) != VK_SUCCESS) {
      throw std::runtime_error("Failed to create framebuffer");
    }
  }
}

void VulkanDriver::InitializeSyncStructures() {
  auto fence_create_info =
      init::GetFenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT);
  auto semaphore_create_info = init::GetSemaphoreCreateInfo();

  for (auto& frame_data : frame_data_) {
    if (vkCreateFence(device_, &fence_create_info, nullptr,
                      &frame_data.render_fence) != VK_SUCCESS) {
      throw std::runtime_error("Unable to create frame fence");
    }

    if (vkCreateSemaphore(device_, &semaphore_create_info, nullptr,
                          &frame_data.present_semaphore) != VK_SUCCESS) {
      throw std::runtime_error("Unable to create frame present semaphore");
    }

    if (vkCreateSemaphore(device_, &semaphore_create_info, nullptr,
                          &frame_data.render_semaphore) != VK_SUCCESS) {
      throw std::runtime_error("Unable to create frame render semaphore");
    }
  }

  fence_create_info.flags = 0;

  if (vkCreateFence(device_, &fence_create_info, nullptr,
                    &upload_context_.upload_fence) != VK_SUCCESS) {
    throw std::runtime_error("Unable to create upload fence");
  }
}

void VulkanDriver::InitializeGraphicsPipeline() {
  // Code.
}

void VulkanDriver::InitializePipelines() {
  // Code.
}

void VulkanDriver::InitializeScene() {
  // Code.
}

void VulkanDriver::InitializeDescriptors() {
  // Code.
}

void VulkanDriver::DestroyImageViews() {
  for (auto& image : swapchain_image_views_) {
    if (image == VK_NULL_HANDLE) {
      continue;
    }

    vkDestroyImageView(device_, image, nullptr);
    image = VK_NULL_HANDLE;
  }
}

void VulkanDriver::DestroyDefaultRenderPass() {
  if (render_pass_ != VK_NULL_HANDLE) {
    vkDestroyRenderPass(device_, render_pass_, nullptr);
    render_pass_ = VK_NULL_HANDLE;
  }
}

void VulkanDriver::DestroyGraphicsPipeline() {
  // Code.
}

void VulkanDriver::DestroySyncStructures() {
  for (auto& frame_data : frame_data_) {
    if (frame_data.render_fence != VK_NULL_HANDLE) {
      vkDestroyFence(device_, frame_data.render_fence, nullptr);
      frame_data.render_fence = VK_NULL_HANDLE;
    }

    if (frame_data.present_semaphore != VK_NULL_HANDLE) {
      vkDestroySemaphore(device_, frame_data.present_semaphore, nullptr);
      frame_data.present_semaphore = VK_NULL_HANDLE;
    }

    if (frame_data.render_semaphore != VK_NULL_HANDLE) {
      vkDestroySemaphore(device_, frame_data.render_semaphore, nullptr);
      frame_data.render_semaphore = VK_NULL_HANDLE;
    }
  }

  if (upload_context_.upload_fence != VK_NULL_HANDLE) {
    vkDestroyFence(device_, upload_context_.upload_fence, nullptr);
    upload_context_.upload_fence = VK_NULL_HANDLE;
  }
}

void VulkanDriver::DestroyDepthResources() {
  if (depth_image_view_ != VK_NULL_HANDLE) {
    vkDestroyImageView(device_, depth_image_view_, nullptr);
    depth_image_view_ = VK_NULL_HANDLE;
  }

  if (allocated_depth_image_.image != VK_NULL_HANDLE) {
    vkDestroyImage(device_, allocated_depth_image_.image, nullptr);
    allocated_depth_image_.image = VK_NULL_HANDLE;
  }

  if (allocated_depth_image_.allocation != VK_NULL_HANDLE) {
    vmaFreeMemory(allocator_, allocated_depth_image_.allocation);
    allocated_depth_image_.allocation = VK_NULL_HANDLE;
  }
}

void VulkanDriver::DestroyColorResources() {
  if (color_image_view_ != VK_NULL_HANDLE) {
    vkDestroyImageView(device_, color_image_view_, nullptr);
    color_image_view_ = VK_NULL_HANDLE;
  }

  if (allocated_color_image_.image != VK_NULL_HANDLE) {
    vkDestroyImage(device_, allocated_color_image_.image, nullptr);
    allocated_color_image_.image = VK_NULL_HANDLE;
  }

  if (allocated_color_image_.allocation != VK_NULL_HANDLE) {
    vmaFreeMemory(allocator_, allocated_color_image_.allocation);
    allocated_color_image_.allocation = VK_NULL_HANDLE;
  }
}

void VulkanDriver::DestroyCommands() {
  for (auto& frame_data : frame_data_) {
    if (frame_data.command_pool != VK_NULL_HANDLE) {
      vkDestroyCommandPool(device_, frame_data.command_pool, nullptr);
      frame_data.command_pool = VK_NULL_HANDLE;
    }
  }

  if (transfer_command_pool_ != VK_NULL_HANDLE) {
    vkDestroyCommandPool(device_, transfer_command_pool_, nullptr);
    transfer_command_pool_ = VK_NULL_HANDLE;
  }

  if (upload_context_.command_pool != VK_NULL_HANDLE) {
    vkDestroyCommandPool(device_, upload_context_.command_pool, nullptr);
    upload_context_.command_pool = VK_NULL_HANDLE;
  }
}

void VulkanDriver::DestroyFrameBuffers() {
  for (auto& frame_buffer : swapchain_frame_buffers_) {
    if (frame_buffer == VK_NULL_HANDLE) {
      continue;
    }

    vkDestroyFramebuffer(device_, frame_buffer, nullptr);
    frame_buffer = VK_NULL_HANDLE;
  }
}

void VulkanDriver::DestroySwapchain() {
  if (device_ == nullptr) {
    return;
  }

  DestroyImageViews();

  if (swapchain_ != VK_NULL_HANDLE) {
    vkDestroySwapchainKHR(device_, swapchain_, nullptr);
    swapchain_ = VK_NULL_HANDLE;
  }
}

void VulkanDriver::DestroyAllocator() {
  if (allocator_ == VK_NULL_HANDLE) {
    return;
  }

  vmaDestroyAllocator(allocator_);
  allocator_ = VK_NULL_HANDLE;
}

void VulkanDriver::DestroySurface() {
  if (instance_ == VK_NULL_HANDLE || surface_ == VK_NULL_HANDLE) {
    return;
  }

  vkDestroySurfaceKHR(instance_, surface_, nullptr);
  surface_ = VK_NULL_HANDLE;
}

void VulkanDriver::DestroyInstance() {
  if (instance_ == VK_NULL_HANDLE) {
    return;
  }

  vkDestroyInstance(instance_, nullptr);
  instance_ = VK_NULL_HANDLE;
}

void VulkanDriver::DestroyDevice() {
  if (device_ == VK_NULL_HANDLE) {
    return;
  }

  vkDestroyDevice(device_, nullptr);
  device_ = VK_NULL_HANDLE;
}

void VulkanDriver::ChoosePhysicalDevice() {
  if (instance_ == nullptr) {
    throw std::runtime_error("Instance is null");
  }

  std::uint32_t device_count = 0;
  vkEnumeratePhysicalDevices(instance_, &device_count, nullptr);

  if (device_count == 0) {
    throw std::runtime_error("Failed to find GPUs with Vulkan support");
  }

  std::vector<VkPhysicalDevice> devices(device_count);
  vkEnumeratePhysicalDevices(instance_, &device_count, devices.data());

  std::multimap<PhysicalDeviceScore, PhysicalDeviceDescr> candidates;

  for (const auto& device : devices) {
    auto descr = GetPhysicalDeviceDescription(device);
    candidates.insert(std::make_pair(descr.score, descr));
  }

  if (candidates.rbegin()->first > 0) {
    physical_device_descr_ = candidates.rbegin()->second;
    queue_family_indices_ = FindQueueFamilies(physical_device_descr_.device);
  } else {
    throw std::runtime_error("Failed to find a suitable GPU");
  }
}

VkSurfaceFormatKHR VulkanDriver::ChooseSwapSurfaceFormat(
    const std::vector<VkSurfaceFormatKHR>& formats) {
  for (const auto& available_format : formats) {
    // TODO(m4jr0): Retrieve the "best" settings from a configuration file.
    if (available_format.format == VK_FORMAT_B8G8R8A8_SRGB &&
        available_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
      return available_format;
    }
  }

  // TODO(m4jr0): Rank the other available formats and return the best one.
  return formats[0];
}

VkPresentModeKHR VulkanDriver::ChooseSwapPresentMode(
    const std::vector<VkPresentModeKHR>& present_modes) {
  for (const auto& available_present_mode : present_modes) {
    // TODO(m4jr0): Retrieve the "sync" mode from the settings.
    if (available_present_mode == VK_PRESENT_MODE_MAILBOX_KHR) {
      return available_present_mode;
    }
  }

  // Return present mode guaranteed to be available.
  return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VulkanDriver::ChooseSwapExtent(
    const VkSurfaceCapabilitiesKHR& capabilities) {
  if (capabilities.currentExtent.width !=
      std::numeric_limits<std::uint32_t>::max()) {
    return capabilities.currentExtent;
  } else {
    VkExtent2D actual_extent = {
        static_cast<std::uint32_t>(window_.GetWidth()),
        static_cast<std::uint32_t>(window_.GetHeight()),
    };

    actual_extent.width =
        std::clamp(actual_extent.width, capabilities.minImageExtent.width,
                   capabilities.maxImageExtent.width);

    actual_extent.height =
        std::clamp(actual_extent.height, capabilities.minImageExtent.height,
                   capabilities.maxImageExtent.height);

    return actual_extent;
  }
}

VkFormat VulkanDriver::ChooseFormat(const std::vector<VkFormat>& candidates,
                                    VkImageTiling tiling,
                                    VkFormatFeatureFlags features) {
  for (auto format : candidates) {
    VkFormatProperties props;
    vkGetPhysicalDeviceFormatProperties(physical_device_descr_.device, format,
                                        &props);

    if (tiling == VK_IMAGE_TILING_LINEAR &&
        (props.linearTilingFeatures & features) == features) {
      return format;
    } else if (tiling == VK_IMAGE_TILING_OPTIMAL &&
               (props.optimalTilingFeatures & features) == features) {
      return format;
    }
  }

  throw std::runtime_error("Failed to find supported format");
}

VkFormat VulkanDriver::ChooseDepthFormat() {
  // TODO(m4jr0): Retrieve settings from configuration.
  return ChooseFormat({VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT,
                       VK_FORMAT_D24_UNORM_S8_UINT},
                      VK_IMAGE_TILING_OPTIMAL,
                      VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

void VulkanDriver::TransitionImageLayout(const CommandBuffer& command_buffer,
                                         VkImage image, VkFormat format,
                                         VkImageLayout old_layout,
                                         VkImageLayout new_layout,
                                         std::uint32_t mip_levels,
                                         std::uint32_t src_queue_family_index,
                                         std::uint32_t dst_queue_family_index) {
  VkImageMemoryBarrier barrier{};
  VkPipelineStageFlags source_stage;
  VkPipelineStageFlags destination_stage;
  VkCommandPool command_pool = VK_NULL_HANDLE;
  VkQueue queue = VK_NULL_HANDLE;

  if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED &&
      new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    destination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    command_pool = GetTransferCommandPool();
    queue = GetTransferQueue();
  } else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
             new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    command_pool = frame_data_[current_frame_].command_pool;
    queue = graphics_queue_;
  } else if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED &&
             new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    destination_stage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    command_pool = frame_data_[current_frame_].command_pool;
    queue = graphics_queue_;
  } else {
    throw std::runtime_error("Unsupported layout transition");
  }

  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  barrier.oldLayout = old_layout;
  barrier.newLayout = new_layout;
  barrier.srcQueueFamilyIndex = src_queue_family_index;
  barrier.dstQueueFamilyIndex = dst_queue_family_index;
  barrier.image = image;

  if (new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

    if (utils::HasStencilComponent(format)) {
      barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
    }
  } else {
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  }

  barrier.subresourceRange.baseMipLevel = 0;
  barrier.subresourceRange.levelCount = mip_levels;
  barrier.subresourceRange.baseArrayLayer = 0;
  barrier.subresourceRange.layerCount = 1;

  vkCmdPipelineBarrier(command_buffer.GetHandle(), source_stage,
                       destination_stage, 0, 0, nullptr, 0, nullptr, 1,
                       &barrier);
}

VkQueue VulkanDriver::GetTransferQueue() {
  if (transfer_queue_ == VK_NULL_HANDLE) {
    return graphics_queue_;
  }

  return transfer_queue_;
}

VkCommandPool VulkanDriver::GetTransferCommandPool() {
  if (transfer_command_pool_ == VK_NULL_HANDLE) {
    return frame_data_[current_frame_].command_pool;
  }

  return transfer_command_pool_;
}

#ifndef NDEBUG
void VulkanDriver::InitializeDebugMessenger() {
  auto create_info = init::GetDebugUtilsMessengerCreateInfo(
      VulkanDriver::LogVulkanValidationMessage);

  if (utils::CreateDebugUtilsMessengerEXT(instance_, &create_info, nullptr,
                                          &debug_messenger_) != VK_SUCCESS) {
    throw std::runtime_error("Failed to set up debug messenger");
  }
}

void VulkanDriver::DestroyDebugMessenger() {
  if (debug_messenger_ == VK_NULL_HANDLE) {
    return;
  }

  utils::DestroyDebugUtilsMessengerEXT(instance_, debug_messenger_, nullptr);
  debug_messenger_ = VK_NULL_HANDLE;
}

const std::vector<const char*> VulkanDriver::kValidationLayers_ = {
    "VK_LAYER_KHRONOS_validation"};

bool VulkanDriver::AreValidationLayersSupported() {
  unsigned int layer_count;
  vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
  std::vector<VkLayerProperties> available_layers(layer_count);
  vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());

  for (const char* layer_name : kValidationLayers_) {
    bool is_layer_found = false;

    for (const auto& layer_properties : available_layers) {
      if (std::strcmp(layer_name, layer_properties.layerName) == 0) {
        is_layer_found = true;
        break;
      }
    }

    if (!is_layer_found) {
      COMET_LOG_RENDERING_ERROR("Unavailable validation layer: ", layer_name,
                                ".");
      return false;
    }
  }

  return true;
}

VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDriver::LogVulkanValidationMessage(
    VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
    VkDebugUtilsMessageTypeFlagsEXT message_type,
    const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
    void* user_data) {
  std::string message_type_str;

  switch (message_type) {
    case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT:
      message_type_str = "General";
      break;
    case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT:
      message_type_str = "Validation";
      break;
    case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT:
      message_type_str = "Performance";
      break;
    default:
      message_type_str = "???";
      break;
  }

  switch (message_severity) {
    default:
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
      COMET_LOG_RENDERING_DEBUG("[Validation | ", message_type_str, "] ",
                                callback_data->pMessage);
      break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
      COMET_LOG_RENDERING_INFO("[Validation | ", message_type_str, "] ",
                               callback_data->pMessage);
      break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
      COMET_LOG_RENDERING_WARNING("[Validation | ", message_type_str, "] ",
                                  callback_data->pMessage);
      break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
      COMET_LOG_RENDERING_ERROR("[Validation | ", message_type_str, "] ",
                                callback_data->pMessage);
      break;
  }

  return VK_FALSE;
}
#endif
}  // namespace vk
}  // namespace rendering
}  // namespace comet
