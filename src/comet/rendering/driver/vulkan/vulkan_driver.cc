// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#define VMA_IMPLEMENTATION

#include "vulkan_driver.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include "comet/core/conf/configuration_manager.h"
#include "comet/core/engine.h"
#include "comet/entity/component/mesh_component.h"
#include "comet/entity/component/transform_component.h"
#include "comet/event/event_manager.h"
#include "comet/event/input_event.h"
#include "comet/event/runtime_event.h"
#include "comet/event/window_event.h"
#include "comet/rendering/driver/vulkan/vulkan_shader.h"
#include "comet/utils/file_system.h"
#include "comet/utils/memory/memory.h"

namespace comet {
namespace rendering {
namespace vk {
VulkanDriver::VulkanDriver()
    : max_frames_in_flight_{COMET_CONF_U8(
          conf::kRenderingVulkanMaxFramesInFlight)},
      is_vsync_{COMET_CONF_BOOL(conf::kRenderingIsVsync)},
      clear_color_{COMET_CONF_F32(conf::kRenderingClearColorR),
                   COMET_CONF_F32(conf::kRenderingClearColorG),
                   COMET_CONF_F32(conf::kRenderingClearColorB),
                   COMET_CONF_F32(conf::kRenderingClearColorA)},
      frame_data_{max_frames_in_flight_} {
  WindowDescr window_descr{};
  window_descr.width =
      static_cast<WindowSize>(COMET_CONF_U16(conf::kRenderingWindowWidth));
  window_descr.height =
      static_cast<WindowSize>(COMET_CONF_U16(conf::kRenderingWindowHeight));
  window_descr.name = COMET_CONF_STR(conf::kApplicationName);
  window_ = VulkanGlfwWindow(window_descr);

  material_handler_.SetDevice(GetDevice());
  material_handler_.SetDescriptorBuilderDescr(VulkanDescriptorBuilderDescr{
      &descriptor_set_layout_handler_, &descriptor_allocator_});
}

void VulkanDriver::Initialize() {
  COMET_LOG_RENDERING_DEBUG("Initializing Vulkan driver.");
  window_.Initialize();

  COMET_ASSERT(window_.IsInitialized(),
               "Vulkan GLFW window is not initialized!");

  InitializeVulkanInstance();

  auto& event_manager{Engine::Get().GetEventManager()};

  event_manager.Register(COMET_EVENT_BIND_FUNCTION(VulkanDriver::OnEvent),
                         event::WindowResizeEvent::kStaticType_);

#ifdef COMET_VULKAN_DEBUG_MODE
  InitializeDebugMessenger();
#endif  // COMET_VULKAN_DEBUG_MODE

  window_.AttachSurface(instance_);

  std::vector<const char*> required_extensions{VK_KHR_SWAPCHAIN_EXTENSION_NAME};

  VulkanDeviceDescr device_descr{};
  device_descr.is_sampler_anisotropy =
      COMET_CONF_BOOL(conf::kRenderingIsSamplerAnisotropy);
  device_descr.is_sample_rate_shading =
      COMET_CONF_BOOL(conf::kRenderingIsSampleRateShading);
  device_descr.physical_device =
      GetBestPhysicalDevice(instance_, window_, required_extensions);
  device_descr.surface = window_;

  device_.SetRequiredExtensions(std::move(required_extensions));
  device_.Initialize(device_descr);
  InitializeAllocator();

  VulkanSwapchainDescr swapchain_descr{};

  swapchain_descr.instance = instance_;
  swapchain_descr.physical_device = device_.GetPhysicalDevice();
  swapchain_descr.device = device_;
  swapchain_descr.surface = window_;
  swapchain_descr.width = window_.GetWidth();
  swapchain_descr.height = window_.GetHeight();
  swapchain_descr.is_vsync = is_vsync_;

  swapchain_.Set(swapchain_descr);
  swapchain_.Initialize();

  InitializeDefaultRenderPass();
  material_handler_.SetRenderPass(GetDefaultRenderPass());
  material_handler_.Initialize();
  InitializeCommands();
  InitializeSamplers();
  InitializeColorResources();
  InitializeDepthResources();
  InitializeFrameBuffers();
  InitializeSyncStructures();
  InitializeDescriptors();

  is_initialized_ = true;
}

void VulkanDriver::Destroy() {
  is_initialized_ = false;

  if (device_ != VK_NULL_HANDLE) {
    vkDeviceWaitIdle(device_);
  }

  DestroyRenderProxies();
  DestroyTextures();
  DestroyDescriptors();
  DestroySyncStructures();
  DestroyFrameBuffers();
  DestroyDepthResources();
  DestroyColorResources();
  DestroySamplers();
  DestroyCommands();
  material_handler_.Destroy();
  DestroyDefaultRenderPass();
  swapchain_.Destroy();
  DestroyAllocator();
  device_.Destroy();
#ifdef COMET_VULKAN_DEBUG_MODE
  DestroyDebugMessenger();
#endif  // COMET_VULKAN_DEBUG_MODE

  if (window_.IsInitialized()) {
    window_.DetachSurface(instance_);
  }

  DestroyInstance();

  if (window_.IsInitialized()) {
    window_.Destroy();
  }
}

void VulkanDriver::Update(time::Interpolation interpolation,
                          entity::EntityManager& entity_manager) {
  if (!swapchain_.IsPresentationAvailable()) {
    return;
  } else if (swapchain_.IsReloadNeeded()) {
    ApplyWindowResize();
    return;
  }

  // TODO(m4jr0): Remove temporary code.
  // Proxies should be managed with proper memory management and occlusion
  // culling.
  const auto view{
      entity_manager
          .GetView<entity::MeshComponent, entity::TransformComponent>()};

  for (const auto entity_id : view) {
    auto* mesh_cmp{
        entity_manager.GetComponent<entity::MeshComponent>(entity_id)};
    auto* transform_cmp{
        entity_manager.GetComponent<entity::TransformComponent>(entity_id)};

    COMET_ASSERT(mesh_cmp->material != nullptr,
                 "Material bound to mesh component is null!");

    // Discard materials without textures.
    if (mesh_cmp->material->texture_tuples.size() == 0) {
      continue;
    }

    auto* proxy{TryGetVulkanRenderProxy(mesh_cmp->mesh)};

    if (proxy != nullptr) {
      proxy->transform += (transform_cmp->global - proxy->transform) *
                          static_cast<f32>(interpolation);
      continue;
    }

    VulkanMesh* mesh{TryGetVulkanMesh(GenerateMeshId(mesh_cmp->mesh))};

    if (mesh == nullptr) {
      mesh = AddVulkanMesh(mesh_cmp->mesh);
      UploadVulkanMesh(*mesh);
    }

    auto* material{material_handler_.Generate(
        GenerateVulkanMaterial(*mesh_cmp->material))};
    proxies_.emplace_back(GenerateVulkanRenderProxy(
        *mesh, *material,
        transform_cmp->global * static_cast<f32>(interpolation)));
  }

  if (PreDraw()) {
    Draw();
    PostDraw();
  }

  ++current_frame_ %= max_frames_in_flight_;
}

void VulkanDriver::SetSize(WindowSize width, WindowSize height) {
  swapchain_.SetSize(width, height);
}

bool VulkanDriver::IsInitialized() const { return is_initialized_; }

Window& VulkanDriver::GetWindow() { return window_; }

VkRenderPass VulkanDriver::GetDefaultRenderPass() const noexcept {
  return render_pass_;
}

const VulkanDevice& VulkanDriver::GetDevice() const noexcept { return device_; }

void VulkanDriver::InitializeVulkanInstance() {
  COMET_LOG_RENDERING_DEBUG("Initializing Vulkan instance.");
  u32 extension_count{0};
  vkEnumerateInstanceExtensionProperties(VK_NULL_HANDLE, &extension_count,
                                         VK_NULL_HANDLE);
  std::vector<VkExtensionProperties> extensions(extension_count);
  vkEnumerateInstanceExtensionProperties(VK_NULL_HANDLE, &extension_count,
                                         extensions.data());

  COMET_LOG_RENDERING_DEBUG("Available Vulkan extensions:");

  for (const auto& extension : extensions) {
    COMET_LOG_RENDERING_DEBUG("\t", extension.extensionName);
  }

  VkApplicationInfo app_info{};
  app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;

  auto app_name{COMET_CONF_STR(conf::kApplicationName)};
  auto app_major_version{COMET_CONF_U8(conf::kRenderingVulkanMajorVersion)};
  auto app_minor_version{COMET_CONF_U8(conf::kRenderingVulkanMinorVersion)};
  auto app_patch_version{COMET_CONF_U8(conf::kRenderingVulkanPatchVersion)};

  app_info.pApplicationName = app_name.c_str();
  app_info.applicationVersion = VK_MAKE_API_VERSION(
      0, app_major_version, app_minor_version, app_patch_version);

  app_info.pEngineName = version::kCometName;
  app_info.engineVersion = VK_MAKE_API_VERSION(0, version::kCometVersionMajor,
                                               version::kCometVersionMinor,
                                               version::kCometVersionPatch);

  auto vk_variant_version{COMET_CONF_U8(conf::kRenderingVulkanVariantVersion)};
  auto vk_major_version{COMET_CONF_U8(conf::kRenderingVulkanMajorVersion)};
  auto vk_minor_version{COMET_CONF_U8(conf::kRenderingVulkanMinorVersion)};
  auto vk_patch_version{COMET_CONF_U8(conf::kRenderingVulkanPatchVersion)};

  app_info.apiVersion = VK_MAKE_API_VERSION(
      vk_variant_version, vk_major_version, vk_minor_version, vk_patch_version);

  VkInstanceCreateInfo create_info{};
  create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  create_info.pApplicationInfo = &app_info;
#ifdef COMET_VULKAN_DEBUG_MODE
  VkValidationFeatureEnableEXT enabled_validation_features[]{
      VK_VALIDATION_FEATURE_ENABLE_DEBUG_PRINTF_EXT};
  VkValidationFeaturesEXT validation_features{};
  validation_features.sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT;
  validation_features.enabledValidationFeatureCount = 1;
  validation_features.pEnabledValidationFeatures = enabled_validation_features;
  create_info.pNext = &validation_features;
#endif  // COMET_VULKAN_DEBUG_MODE

  const auto required_extensions{GetRequiredExtensions()};
  const u32 required_extension_count{
      static_cast<u32>(required_extensions.size())};

  COMET_LOG_RENDERING_DEBUG("Required extensions:");

  for (uindex i{0}; i < required_extension_count; ++i) {
    const char* required_extension{required_extensions[i]};
    COMET_LOG_RENDERING_DEBUG("\t", required_extension);
    auto is_found{false};

    for (const auto& extension : extensions) {
      if (std::strcmp(required_extension, extension.extensionName) == 0) {
        is_found = true;
        break;
      }
    }

    COMET_ASSERT(is_found,
                 "Required extension is not available: ", required_extension);
  }

  create_info.enabledExtensionCount = required_extension_count;
  create_info.ppEnabledExtensionNames = required_extensions.data();

#ifndef COMET_VULKAN_DEBUG_MODE
  create_info.enabledLayerCount = 0;
  create_info.pNext = VK_NULL_HANDLE;
#else
  COMET_ASSERT(AreValidationLayersSupported(),
               "At least one validation layer is not available!");

  create_info.enabledLayerCount = static_cast<u32>(kValidationLayers_.size());
  create_info.ppEnabledLayerNames = kValidationLayers_.data();
  auto debug_create_info{init::GetDebugUtilsMessengerCreateInfo(
      VulkanDriver::LogVulkanValidationMessage)};
  create_info.pNext =
      static_cast<VkDebugUtilsMessengerCreateInfoEXT*>(&debug_create_info);
#endif  // !COMET_VULKAN_DEBUG_MODE

  COMET_CHECK_VK(vkCreateInstance(&create_info, VK_NULL_HANDLE, &instance_),
                 "Failed to create instance!");
}

void VulkanDriver::InitializeAllocator() {
  auto vk_variant_version{COMET_CONF_U8(conf::kRenderingVulkanVariantVersion)};
  auto vk_major_version{COMET_CONF_U8(conf::kRenderingVulkanMajorVersion)};
  auto vk_minor_version{COMET_CONF_U8(conf::kRenderingVulkanMinorVersion)};
  auto vk_patch_version{COMET_CONF_U8(conf::kRenderingVulkanPatchVersion)};

  VmaAllocatorCreateInfo allocatorCreateInfo{};
  allocatorCreateInfo.vulkanApiVersion = VK_MAKE_API_VERSION(
      vk_variant_version, vk_major_version, vk_minor_version, vk_patch_version);
  allocatorCreateInfo.physicalDevice = device_.GetPhysicalDevice();
  allocatorCreateInfo.device = device_;
  allocatorCreateInfo.instance = instance_;

  vmaCreateAllocator(&allocatorCreateInfo, &allocator_);
}

void VulkanDriver::InitializeDefaultRenderPass() {
  const auto msaa_samples{device_.GetMsaaSamples()};

  VkAttachmentDescription color_attachment{};
  color_attachment.format = swapchain_.GetFormat();
  color_attachment.samples = msaa_samples;
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
  depth_attachment.format = device_.ChooseDepthFormat();
  depth_attachment.samples = msaa_samples;
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
  color_attachment_resolve.format = swapchain_.GetFormat();
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

  std::array<VkAttachmentDescription, 3> attachments{
      color_attachment, depth_attachment, color_attachment_resolve};

  VkRenderPassCreateInfo render_pass_info{};
  render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  render_pass_info.attachmentCount = static_cast<u32>(attachments.size());
  render_pass_info.pAttachments = attachments.data();
  render_pass_info.subpassCount = 1;
  render_pass_info.pSubpasses = &subpass;
  render_pass_info.dependencyCount = 1;
  render_pass_info.pDependencies = &dependency;

  COMET_CHECK_VK(vkCreateRenderPass(device_, &render_pass_info, VK_NULL_HANDLE,
                                    &render_pass_),
                 "Failed to create render pass!");
}

void VulkanDriver::InitializeCommands() {
  auto pool_info{init::GetCommandPoolCreateInfo(
      device_.GetQueueFamilyIndices().graphics_family.value(),
      VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT)};

  for (uindex i = 0; i < max_frames_in_flight_; ++i) {
    COMET_CHECK_VK(vkCreateCommandPool(device_, &pool_info, VK_NULL_HANDLE,
                                       &frame_data_[i].command_pool),
                   "Failed to create frame command pool!");

    auto allocate_info{
        init::GetCommandBufferAllocateInfo(frame_data_[i].command_pool, 1)};

    COMET_CHECK_VK(vkAllocateCommandBuffers(device_, &allocate_info,
                                            &frame_data_[i].command_buffer),
                   "Failed to allocate frame command buffer!");
  }

  pool_info.flags = 0;

  COMET_CHECK_VK(vkCreateCommandPool(device_, &pool_info, VK_NULL_HANDLE,
                                     &upload_context_.command_pool),
                 "Failed to create upload command pool!");

  if (!device_.GetQueueFamilyIndices().IsSpecificTransferFamily()) {
    return;
  }

  pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT |
                    VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
  pool_info.queueFamilyIndex =
      device_.GetQueueFamilyIndices().transfer_family.value();

  COMET_CHECK_VK(vkCreateCommandPool(device_, &pool_info, VK_NULL_HANDLE,
                                     &transfer_command_pool_),
                 "Failed to create transfer command pool!");
}

void VulkanDriver::InitializeSamplers() {
  // TODO(m4jr0): Handle this with settings.
  auto sampler_info{init::GetSamplerCreateInfo(VK_FILTER_LINEAR,
                                               VK_SAMPLER_ADDRESS_MODE_REPEAT)};

  sampler_info.anisotropyEnable = true;
  sampler_info.maxAnisotropy =
      device_.GetProperties().limits.maxSamplerAnisotropy;
  sampler_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
  sampler_info.unnormalizedCoordinates = VK_FALSE;
  sampler_info.compareEnable = VK_FALSE;
  sampler_info.compareOp = VK_COMPARE_OP_ALWAYS;
  sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
  sampler_info.mipLodBias = 0.0f;
  sampler_info.minLod = 0.0f;
  sampler_info.maxLod = VK_LOD_CLAMP_NONE;

  COMET_CHECK_VK(vkCreateSampler(device_, &sampler_info, VK_NULL_HANDLE,
                                 &texture_sampler_),
                 "Failed to create texture sampler!");
}

void VulkanDriver::InitializeColorResources() {
  const auto color_format{swapchain_.GetFormat()};
  const auto extent{swapchain_.GetExtent()};

  CreateImage(allocated_color_image_, device_, allocator_, extent.width,
              extent.height, 1, device_.GetMsaaSamples(), color_format,
              VK_IMAGE_TILING_OPTIMAL,
              VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT |
                  VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
              VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

  // One 1 mip level, since it is Vulkan enforces the usage of only one when
  // there are more than one sample per pixel (and we don't need mimaps
  // anyway).
  color_image_view_ =
      CreateImageView(device_, allocated_color_image_.image, color_format,
                      VK_IMAGE_ASPECT_COLOR_BIT, 1);
}

void VulkanDriver::InitializeDepthResources() {
  auto depth_format{device_.ChooseDepthFormat()};
  const auto extent{swapchain_.GetExtent()};

  CreateImage(allocated_depth_image_, device_, allocator_, extent.width,
              extent.height, 1, device_.GetMsaaSamples(), depth_format,
              VK_IMAGE_TILING_OPTIMAL,
              VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
              VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

  depth_image_view_ =
      CreateImageView(device_, allocated_depth_image_.image, depth_format,
                      VK_IMAGE_ASPECT_DEPTH_BIT, 1);

  CommandBuffer command_buffer{device_,
                               frame_data_[current_frame_].command_pool};
  command_buffer.Allocate();
  command_buffer.Record();

  TransitionImageLayout(command_buffer, allocated_depth_image_.image,
                        depth_format, VK_IMAGE_LAYOUT_UNDEFINED,
                        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 1);

  command_buffer.Submit(device_.GetGraphicsQueue());
  command_buffer.Free();
}

void VulkanDriver::InitializeFrameBuffers() {
  const auto extent{swapchain_.GetExtent()};
  const auto swapchain_images_{swapchain_.GetImages()};
  const auto swapchain_image_views_{swapchain_.GetImageViews()};
  frame_buffers_.resize(swapchain_images_.size());

  auto create_info{init::GetFrameBufferCreateInfo(render_pass_, extent)};
  create_info.width = extent.width;
  create_info.height = extent.height;
  create_info.layers = 1;

  for (uindex i{0}; i < swapchain_images_.size(); ++i) {
    std::array<VkImageView, 3> attachments = {
        color_image_view_, depth_image_view_, swapchain_image_views_[i]};

    create_info.attachmentCount = static_cast<u32>(attachments.size());
    create_info.pAttachments = attachments.data();

    COMET_CHECK_VK(vkCreateFramebuffer(device_, &create_info, VK_NULL_HANDLE,
                                       &frame_buffers_[i]),
                   "Failed to create framebuffer!");
  }
}

void VulkanDriver::InitializeSyncStructures() {
  auto fence_create_info{
      init::GetFenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT)};
  auto semaphore_create_info{init::GetSemaphoreCreateInfo()};

  for (auto& frame_data : frame_data_) {
    COMET_CHECK_VK(vkCreateFence(device_, &fence_create_info, VK_NULL_HANDLE,
                                 &frame_data.render_fence),
                   "Unable to create frame fence!");

    COMET_CHECK_VK(
        vkCreateSemaphore(device_, &semaphore_create_info, VK_NULL_HANDLE,
                          &frame_data.present_semaphore),
        "Unable to create frame present semaphore!");

    COMET_CHECK_VK(
        vkCreateSemaphore(device_, &semaphore_create_info, VK_NULL_HANDLE,
                          &frame_data.render_semaphore),
        "Unable to create frame render semaphore!");
  }

  fence_create_info.flags = 0;

  COMET_CHECK_VK(vkCreateFence(device_, &fence_create_info, VK_NULL_HANDLE,
                               &upload_context_.upload_fence),
                 "Unable to create upload fence!");
}

void VulkanDriver::InitializeDescriptors() {
  descriptor_allocator_.SetDevice(device_);
  descriptor_set_layout_handler_.SetDevice(device_);

  const auto texture_binding{init::GetDescriptorSetLayoutBinding(
      VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT,
      0)};

  VkDescriptorSetLayoutCreateInfo texture_descriptor_set_layout_info{};
  texture_descriptor_set_layout_info.sType =
      VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  texture_descriptor_set_layout_info.bindingCount = 1;
  texture_descriptor_set_layout_info.pBindings = &texture_binding;
  texture_descriptor_set_layout_info.flags = 0;
  texture_descriptor_set_layout_info.pNext = VK_NULL_HANDLE;

  single_texture_descriptor_set_layout_ =
      descriptor_set_layout_handler_.Get(texture_descriptor_set_layout_info);

  const auto scene_data_size{comet::utils::memory::AlignSize(
      sizeof(SceneData),
      device_.GetProperties().limits.minUniformBufferOffsetAlignment)};

  const auto camera_data_size{comet::utils::memory::AlignSize(
      sizeof(CameraData),
      device_.GetProperties().limits.minUniformBufferOffsetAlignment)};

  for (uindex i{0}; i < max_frames_in_flight_; ++i) {
    auto& frame_data{frame_data_[i]};
    frame_data.descriptor_allocator.SetDevice(device_);

    CreateBuffer(frame_data.buffer, scene_data_size + camera_data_size,
                 allocator_, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                 VMA_MEMORY_USAGE_CPU_TO_GPU);

    CreateBuffer(frame_data.object_buffer,
                 sizeof(ObjectData) * max_object_count_, allocator_,
                 VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                 VMA_MEMORY_USAGE_CPU_TO_GPU);

    VkDescriptorBufferInfo camera_descriptor_buffer_info{};
    camera_descriptor_buffer_info.buffer = frame_data.buffer.buffer;
    camera_descriptor_buffer_info.offset =
        0;  // Offset will be specified later.
    camera_descriptor_buffer_info.range = sizeof(CameraData);

    VkDescriptorBufferInfo scene_descriptor_buffer_info{};
    scene_descriptor_buffer_info.buffer = frame_data.buffer.buffer;
    scene_descriptor_buffer_info.offset = 0;  // Offset will be specified later.
    scene_descriptor_buffer_info.range = sizeof(SceneData);

    VulkanDescriptorBuilder::Generate(descriptor_set_layout_handler_,
                                      frame_data.descriptor_allocator)
        .Bind(0, camera_descriptor_buffer_info,
              VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
              VK_SHADER_STAGE_VERTEX_BIT)
        .Bind(1, scene_descriptor_buffer_info,
              VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
              VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
        .Build(frame_data.global_descriptor_set, global_descriptor_set_layout_);

    VkDescriptorBufferInfo object_descriptor_buffer_info{};
    object_descriptor_buffer_info.buffer = frame_data.object_buffer.buffer;
    object_descriptor_buffer_info.offset = 0;
    object_descriptor_buffer_info.range =
        sizeof(ObjectData) * max_object_count_;

    VulkanDescriptorBuilder::Generate(descriptor_set_layout_handler_,
                                      frame_data.descriptor_allocator)
        .Bind(0, object_descriptor_buffer_info,
              VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
        .Build(frame_data.object_descriptor_set, object_descriptor_set_layout_);
  }
}

void VulkanDriver::InitializePipelines() {
  VulkanShaderHandler handler{};
  handler.SetDevice(device_);

  auto vert_shader_module{handler.Get("shaders/vulkan/default.vk.vert")};
  auto frag_shader_module{handler.Get("shaders/vulkan/default.vk.frag")};

  const auto vert_shader_stage_info{init::GetPipelineShaderStageCreateInfo(
      VK_SHADER_STAGE_VERTEX_BIT, vert_shader_module.handle)};

  const auto frag_shader_stage_info{init::GetPipelineShaderStageCreateInfo(
      VK_SHADER_STAGE_FRAGMENT_BIT, frag_shader_module.handle)};

  std::array<VkPipelineShaderStageCreateInfo, 2> shader_stages{
      {vert_shader_stage_info, frag_shader_stage_info}};

  auto vertex_descr{VulkanVertex::GetVertexDescr()};

  auto vertex_input_info{init::GetPipelineVertexInputStateCreateInfo(
      vertex_descr.bindings.data(),
      static_cast<u32>(vertex_descr.bindings.size()),
      vertex_descr.attributes.data(),
      static_cast<u32>(vertex_descr.attributes.size()))};

  const auto input_assembly_info{init::GetPipelineInputAssemblyStateCreateInfo(
      VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)};
  const auto extent{swapchain_.GetExtent()};

  VkViewport viewport{};
  viewport.x = 0.0f;
  viewport.y = 0.0f;
  viewport.width = extent.width;
  viewport.height = extent.height;
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;

  VkRect2D scissor{};
  scissor.offset = {0, 0};
  scissor.extent = extent;

  VkPipelineViewportStateCreateInfo viewport_info{};
  viewport_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewport_info.viewportCount = 1;
  viewport_info.pViewports = &viewport;
  viewport_info.scissorCount = 1;
  viewport_info.pScissors = &scissor;

  auto rasterizer_info{
      init::GetPipelineRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL)};
  rasterizer_info.cullMode = VK_CULL_MODE_BACK_BIT;
  rasterizer_info.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

  auto multisampling_info{init::GetPipelineMultisampleStateCreateInfo()};
  multisampling_info.rasterizationSamples = device_.GetMsaaSamples();
  multisampling_info.sampleShadingEnable =
      VK_TRUE;  // Enable sample shading in the pipeline.
  multisampling_info.minSampleShading =
      .2f;  // Min fraction for sample shading; closer to one is smoother.

  const auto depth_stencil_info{init::GetPipelineDepthStencilStateCreateInfo(
      true, true, VK_COMPARE_OP_LESS  // New fragments should be less (lower
                                      // depth = closer).
      )};

  const auto color_blend_attachment{
      init::GetPipelineColorBlendAttachmentState()};

  const auto color_blend_info{
      init::GetPipelineColorBlendStateCreateInfo(&color_blend_attachment, 1)};

  std::array<VkDynamicState, 2> dynamic_states{
      {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_LINE_WIDTH}};

  VkPipelineDynamicStateCreateInfo dynamic_state_info{};
  dynamic_state_info.sType =
      VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  dynamic_state_info.dynamicStateCount = dynamic_states.size();
  dynamic_state_info.pDynamicStates = dynamic_states.data();

  VkPushConstantRange push_constant_range{};
  push_constant_range.offset = 0;
  push_constant_range.size = sizeof(MeshPushConstants);
  push_constant_range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

  auto pipeline_layout_info{init::GetPipelineLayoutCreateInfo()};

  std::array<VkDescriptorSetLayout, 3> descriptor_set_layouts{
      {global_descriptor_set_layout_, object_descriptor_set_layout_,
       single_texture_descriptor_set_layout_}};

  pipeline_layout_info.setLayoutCount =
      static_cast<u32>(descriptor_set_layouts.size());
  pipeline_layout_info.pSetLayouts = descriptor_set_layouts.data();
  pipeline_layout_info.pushConstantRangeCount = 1;
  pipeline_layout_info.pPushConstantRanges = &push_constant_range;

  COMET_CHECK_VK(vkCreatePipelineLayout(device_, &pipeline_layout_info,
                                        VK_NULL_HANDLE, &pipeline_layout_),
                 "Failed to created pipeline layout!");

  VkGraphicsPipelineCreateInfo pipeline_info{};
  pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipeline_info.stageCount = static_cast<u32>(shader_stages.size());
  pipeline_info.pStages = shader_stages.data();

  pipeline_info.pVertexInputState = &vertex_input_info;
  pipeline_info.pInputAssemblyState = &input_assembly_info;
  pipeline_info.pViewportState = &viewport_info;
  pipeline_info.pRasterizationState = &rasterizer_info;
  pipeline_info.pMultisampleState = &multisampling_info;
  pipeline_info.pDepthStencilState = &depth_stencil_info;
  pipeline_info.pColorBlendState = &color_blend_info;
  pipeline_info.pDynamicState = VK_NULL_HANDLE;

  pipeline_info.layout = pipeline_layout_;
  pipeline_info.renderPass = render_pass_;
  pipeline_info.subpass = 0;

  pipeline_info.basePipelineHandle = VK_NULL_HANDLE;
  pipeline_info.basePipelineIndex = -1;

  COMET_CHECK_VK(
      vkCreateGraphicsPipelines(device_, VK_NULL_HANDLE, 1, &pipeline_info,
                                VK_NULL_HANDLE, &graphics_pipeline_),
      "Failed to create graphics pipeline!");

  vkDestroyShaderModule(device_, frag_shader_module.handle, VK_NULL_HANDLE);
  vkDestroyShaderModule(device_, vert_shader_module.handle, VK_NULL_HANDLE);
}

void VulkanDriver::DestroyRenderProxies() {
  for (auto& proxy : proxies_) {
    proxy.mesh->vertex_buffer.Destroy();
  }
}

void VulkanDriver::DestroyTextures() {
  for (auto& it : textures_) {
    vkDestroyImageView(device_, it.second.view, VK_NULL_HANDLE);
    it.second.allocation.Destroy();
  }
}

void VulkanDriver::DestroyPipelines() {
  if (graphics_pipeline_ != VK_NULL_HANDLE) {
    vkDestroyPipeline(device_, graphics_pipeline_, VK_NULL_HANDLE);
    graphics_pipeline_ = VK_NULL_HANDLE;
  }

  if (pipeline_layout_ != VK_NULL_HANDLE) {
    vkDestroyPipelineLayout(device_, pipeline_layout_, VK_NULL_HANDLE);
    pipeline_layout_ = VK_NULL_HANDLE;
  }
}

void VulkanDriver::DestroyDescriptors() {
  descriptor_set_layout_handler_.Destroy();
  descriptor_allocator_.Destroy();

  for (uindex i{0}; i < max_frames_in_flight_; ++i) {
    auto& frame_data{frame_data_[i]};
    frame_data.descriptor_allocator.Destroy();

    if (frame_data.buffer.buffer != VK_NULL_HANDLE) {
      frame_data.buffer.Destroy();
    }

    if (frame_data.object_buffer.buffer != VK_NULL_HANDLE) {
      frame_data.object_buffer.Destroy();
    }
  }
}

void VulkanDriver::DestroySyncStructures() {
  for (auto& frame_data : frame_data_) {
    if (frame_data.render_fence != VK_NULL_HANDLE) {
      vkDestroyFence(device_, frame_data.render_fence, VK_NULL_HANDLE);
      frame_data.render_fence = VK_NULL_HANDLE;
    }

    if (frame_data.present_semaphore != VK_NULL_HANDLE) {
      vkDestroySemaphore(device_, frame_data.present_semaphore, VK_NULL_HANDLE);
      frame_data.present_semaphore = VK_NULL_HANDLE;
    }

    if (frame_data.render_semaphore != VK_NULL_HANDLE) {
      vkDestroySemaphore(device_, frame_data.render_semaphore, VK_NULL_HANDLE);
      frame_data.render_semaphore = VK_NULL_HANDLE;
    }
  }

  if (upload_context_.upload_fence != VK_NULL_HANDLE) {
    vkDestroyFence(device_, upload_context_.upload_fence, VK_NULL_HANDLE);
    upload_context_.upload_fence = VK_NULL_HANDLE;
  }
}

void VulkanDriver::DestroyFrameBuffers() {
  for (auto& frame_buffer : frame_buffers_) {
    if (frame_buffer == VK_NULL_HANDLE) {
      continue;
    }

    vkDestroyFramebuffer(device_, frame_buffer, VK_NULL_HANDLE);
    frame_buffer = VK_NULL_HANDLE;
  }
}

void VulkanDriver::DestroyDepthResources() {
  if (depth_image_view_ != VK_NULL_HANDLE) {
    vkDestroyImageView(device_, depth_image_view_, VK_NULL_HANDLE);
    depth_image_view_ = VK_NULL_HANDLE;
  }

  if (allocated_depth_image_.IsInitialized()) {
    allocated_depth_image_.Destroy();
  }
}

void VulkanDriver::DestroyColorResources() {
  if (color_image_view_ != VK_NULL_HANDLE) {
    vkDestroyImageView(device_, color_image_view_, VK_NULL_HANDLE);
    color_image_view_ = VK_NULL_HANDLE;
  }

  if (allocated_color_image_.IsInitialized()) {
    allocated_color_image_.Destroy();
  }
}

void VulkanDriver::DestroySamplers() {
  if (texture_sampler_ != VK_NULL_HANDLE) {
    vkDestroySampler(device_, texture_sampler_, VK_NULL_HANDLE);
    texture_sampler_ = VK_NULL_HANDLE;
  }
}

void VulkanDriver::DestroyCommands() {
  for (auto& frame_data : frame_data_) {
    if (frame_data.command_pool != VK_NULL_HANDLE) {
      vkDestroyCommandPool(device_, frame_data.command_pool, VK_NULL_HANDLE);
      frame_data.command_pool = VK_NULL_HANDLE;
    }
  }

  if (transfer_command_pool_ != VK_NULL_HANDLE) {
    vkDestroyCommandPool(device_, transfer_command_pool_, VK_NULL_HANDLE);
    transfer_command_pool_ = VK_NULL_HANDLE;
  }

  if (upload_context_.command_pool != VK_NULL_HANDLE) {
    vkDestroyCommandPool(device_, upload_context_.command_pool, VK_NULL_HANDLE);
    upload_context_.command_pool = VK_NULL_HANDLE;
  }
}

void VulkanDriver::DestroyDefaultRenderPass() {
  if (render_pass_ != VK_NULL_HANDLE) {
    vkDestroyRenderPass(device_, render_pass_, VK_NULL_HANDLE);
    render_pass_ = VK_NULL_HANDLE;
  }
}

void VulkanDriver::DestroyAllocator() {
  if (allocator_ == VK_NULL_HANDLE) {
    return;
  }

  vmaDestroyAllocator(allocator_);
  allocator_ = VK_NULL_HANDLE;
}

void VulkanDriver::DestroyInstance() {
  if (instance_ == VK_NULL_HANDLE) {
    return;
  }

  vkDestroyInstance(instance_, VK_NULL_HANDLE);
  instance_ = VK_NULL_HANDLE;
}

void VulkanDriver::OnEvent(const event::Event& event) {
  const auto& event_type{event.GetType()};

  if (event_type == event::WindowResizeEvent::kStaticType_) {
    const auto& window_event{
        static_cast<const event::WindowResizeEvent&>(event)};
    SetSize(window_event.GetWidth(), window_event.GetHeight());
  }
}

void VulkanDriver::ApplyWindowResize() {
  vkDeviceWaitIdle(device_);

  DestroyFrameBuffers();
  DestroyDepthResources();
  DestroyColorResources();
  DestroyCommands();
  material_handler_.Destroy();
  DestroyDefaultRenderPass();

  if (!swapchain_.Reload()) {
    return;
  }

  InitializeDefaultRenderPass();
  material_handler_.Initialize();
  InitializeCommands();
  InitializeDepthResources();
  InitializeColorResources();
  InitializeFrameBuffers();
}

bool VulkanDriver::PreDraw() {
  window_.Update();
  auto& frame_data{frame_data_[current_frame_]};

  COMET_CHECK_VK(vkWaitForFences(device_, 1, &frame_data.render_fence, VK_TRUE,
                                 static_cast<u64>(-1)),
                 "Something wrong happened while waiting for render fence!");

  auto result{swapchain_.AcquireNextImage(frame_data.present_semaphore)};

  if (result == VK_ERROR_OUT_OF_DATE_KHR) {
    // TODO(m4jr0): Recreate swap chain.
    ApplyWindowResize();
    return false;
  }

  COMET_ASSERT(result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR,
               "Failed to acquire swapchain image!");

  return true;
}

void VulkanDriver::PostDraw() {
  const auto result{swapchain_.QueuePresent(
      device_.GetPresentQueue(), frame_data_[current_frame_].render_semaphore,
      swapchain_.GetCurrentImageIndex())};

  if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
    // TODO(m4hjr0): Recreate swapchain.
    ApplyWindowResize();
    return;
  }

  COMET_ASSERT(result == VK_SUCCESS, "Failed to present swap chain image!");
}

void VulkanDriver::Draw() {
  auto& frame_data{frame_data_[current_frame_]};

  // Reset fence if work is submitted.
  COMET_CHECK_VK(vkResetFences(device_, 1, &frame_data.render_fence),
                 "Unable to reset render fence!");
  auto command_buffer{frame_data.command_buffer};

  const auto command_buffer_begin_info{init::GetCommandBufferBeginInfo(
      VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT)};

  COMET_CHECK_VK(
      vkBeginCommandBuffer(command_buffer, &command_buffer_begin_info),
      "Failed to begin recording command buffer");

  // Should be identical to the order of the attachments.
  std::array<VkClearValue, 2> clear_values{};
  clear_values[0].color = {
      {clear_color_[0], clear_color_[1], clear_color_[2], clear_color_[3]}};
  clear_values[1].depthStencil.depth = 1.0f;

  auto render_pass_begin_info{init::GetRenderPassBeginInfo(
      render_pass_, swapchain_.GetExtent(),
      frame_buffers_[swapchain_.GetCurrentImageIndex()])};

  render_pass_begin_info.clearValueCount =
      static_cast<u32>(clear_values.size());
  render_pass_begin_info.pClearValues = clear_values.data();

  vkCmdBeginRenderPass(command_buffer, &render_pass_begin_info,
                       VK_SUBPASS_CONTENTS_INLINE);

  const auto extent{swapchain_.GetExtent()};

  VkViewport viewport{};
  viewport.x = 0.0f;
  viewport.y = 0.0f;
  viewport.width = extent.width;
  viewport.height = extent.height;
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;

  VkRect2D scissor{};
  scissor.offset = {0, 0};
  scissor.extent = extent;

  vkCmdSetViewport(command_buffer, 0, 1, &viewport);
  vkCmdSetScissor(command_buffer, 0, 1, &scissor);

  DrawRenderProxies(command_buffer);

  vkCmdEndRenderPass(command_buffer);

  COMET_CHECK_VK(vkEndCommandBuffer(command_buffer),
                 "Failed to record command buffer!");

  VkSubmitInfo submit_info{init::GetSubmitInfo(&command_buffer)};

  std::array<VkPipelineStageFlags, 1> wait_stages{
      {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT}};

  submit_info.waitSemaphoreCount = 1;
  submit_info.pWaitSemaphores = &frame_data.present_semaphore;
  submit_info.pWaitDstStageMask = wait_stages.data();
  submit_info.signalSemaphoreCount = 1;
  submit_info.pSignalSemaphores = &frame_data.render_semaphore;

  COMET_CHECK_VK(vkQueueSubmit(device_.GetGraphicsQueue(), 1, &submit_info,
                               frame_data.render_fence),
                 "Failed to submit draw command buffer!");
}

void VulkanDriver::DrawRenderProxies(VkCommandBuffer command_buffer) {
  // TODO(m4jr0): Remove temporary code.
  auto nearest_point{0.1f};
  auto farthest_point{100.0f};
  auto fov{45.0f};

  auto width{Engine::Get().GetRenderingManager().GetWindow()->GetWidth()};
  auto height{Engine::Get().GetRenderingManager().GetWindow()->GetHeight()};

  auto projection_matrix{glm::perspective(
      glm::radians(fov), static_cast<f32>(width) / static_cast<f32>(height),
      nearest_point, farthest_point)};

  projection_matrix[1][1] *= -1;  // Axis is inverted in Vulkan.

  auto position{glm::vec3(0, 18, 15)};
  auto direction{glm::vec3(0, 0, -15)};
  auto orientation{glm::vec3(0, 1, 0)};

  auto view_matrix{glm::lookAt(position, direction, orientation)};

  CameraData camera_data{};
  camera_data.proj = std::move(projection_matrix);
  camera_data.view = std::move(view_matrix);
  camera_data.view_proj = camera_data.proj * camera_data.view;

  auto& frame_data{frame_data_[current_frame_]};

  scene_data_.ambient_color = glm::vec4{0.5};
  scene_data_.sunlight_color = glm::vec4{1.f};
  scene_data_.sunlight_direction = glm::vec4(1.f, 1.f, 1.f, 1.f);

  const auto uniform_buffer_align{
      device_.GetProperties().limits.minUniformBufferOffsetAlignment};

  const auto camera_data_size{comet::utils::memory::AlignSize(
      sizeof(CameraData), uniform_buffer_align)};

  frame_data.buffer.Map();
  frame_data.buffer.CopyTo(&camera_data, sizeof(CameraData));
  frame_data.buffer.CopyTo(&scene_data_, sizeof(SceneData), camera_data_size);
  frame_data.buffer.Unmap();

  const auto camera_offset{static_cast<u32>(0)};
  const auto scene_offset{static_cast<u32>(camera_data_size)};

  const std::array<u32, 2> offsets{camera_offset, scene_offset};

  frame_data.object_buffer.Map();
  auto* object_data{
      reinterpret_cast<ObjectData*>(frame_data.object_buffer.mapped_memory)};
  const auto proxy_count{proxies_.size()};

  for (uindex i{0}; i < proxy_count; ++i) {
    object_data[i].model = proxies_[i].transform;
  }

  frame_data.object_buffer.Unmap();

  VulkanMesh* last_mesh{nullptr};
  VkPipeline last_pipeline{VK_NULL_HANDLE};
  VkDescriptorSet last_descriptor_set{VK_NULL_HANDLE};

  for (uindex i{0}; i < proxy_count; ++i) {
    const auto& proxy{proxies_[i]};
    const auto* shader_pass_data{
        proxy.material->effect.pass_data[VulkanMeshPassType::Forward]};

    COMET_ASSERT(shader_pass_data != nullptr, "Shader pass data for material ",
                 proxy.material->id, " is null!");

    const auto current_pipeline{shader_pass_data->pipeline};
    const auto current_pipeline_layout{shader_pass_data->pipeline_layout};
    const auto current_descriptor_set{
        proxy.material->descriptor_sets[VulkanMeshPassType::Forward]};

    if (current_pipeline != last_pipeline) {
      vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                        current_pipeline);
      last_pipeline = current_pipeline;

      vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                              current_pipeline_layout, 0, 1,
                              &frame_data.global_descriptor_set,
                              static_cast<u32>(offsets.size()), offsets.data());

      vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                              current_pipeline_layout, 1, 1,
                              &frame_data.object_descriptor_set, 0,
                              VK_NULL_HANDLE);
    }

    if (current_descriptor_set != last_descriptor_set) {
      vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                              current_pipeline_layout, 2, 1,
                              &current_descriptor_set, 0, VK_NULL_HANDLE);
      last_descriptor_set = current_descriptor_set;
    }

    MeshPushConstants constants{};
    constants.render_matrix = proxy.transform;
    vkCmdPushConstants(command_buffer, current_pipeline_layout,
                       VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(MeshPushConstants),
                       &constants);

    if (proxy.mesh != last_mesh) {
      VkDeviceSize offset{0};
      vkCmdBindVertexBuffers(command_buffer, 0, 1,
                             &proxy.mesh->vertex_buffer.buffer, &offset);
      vkCmdBindIndexBuffer(
          command_buffer, proxy.mesh->vertex_buffer.buffer,
          static_cast<VkDeviceSize>(sizeof(VulkanVertex) *
                                    proxy.mesh->vertices.size()),
          VK_INDEX_TYPE_UINT32);
      last_mesh = proxy.mesh;
    }

    vkCmdDrawIndexed(command_buffer,
                     static_cast<u32>(proxy.mesh->indices.size()), 1, 0, 0, i);
  }
}

VulkanMesh* VulkanDriver::AddVulkanMesh(
    const resource::MeshResource* resource) {
  auto&& mesh{GenerateMesh(resource)};

  COMET_ASSERT(meshes_.find(mesh.id) == meshes_.cend(), "Requested mesh \"",
               COMET_STRING_ID_LABEL(mesh.id), "\" (", mesh.id,
               ") already exists!");

  const auto result{meshes_.emplace(mesh.id, std::move(mesh))};
  COMET_ASSERT(result.second, "Could not insert Vulkan mesh!");
  return &result.first->second;
}

VulkanMesh* VulkanDriver::TryGetVulkanMesh(VulkanMeshId mesh_id) {
  auto mesh_cond{meshes_.find(mesh_id)};

  if (mesh_cond == meshes_.cend()) {
    return nullptr;
  }

  return &mesh_cond->second;
}

VulkanMesh* VulkanDriver::GetVulkanMesh(VulkanMeshId mesh_id) {
  auto* mesh{TryGetVulkanMesh(mesh_id)};

  COMET_ASSERT(mesh != nullptr, "Requested mesh \"",
               COMET_STRING_ID_LABEL(mesh_id), "\" (", mesh_id,
               ") does not exist!");

  return mesh;
}

VulkanTexture* VulkanDriver::UploadVulkanTexture(
    const resource::TextureResource* resource) {
  const auto it{textures_.find(resource->id)};

  if (it != textures_.end()) {
    return &it->second;
  }

  auto insert_pair{textures_.insert_or_assign(resource->id,
                                              GenerateVulkanTexture(resource))};

  COMET_ASSERT(insert_pair.second, "Could not insert texture with ID ",
               resource->id, "!");

  auto& texture{insert_pair.first->second};

  // Create texture image.
  const auto image_size{texture.width * texture.height * 4};
  AllocatedBuffer staging_buffer{};

  CreateBuffer(staging_buffer, image_size, allocator_,
               VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_AUTO,
               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                   VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
               VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);

  staging_buffer.Map();
  staging_buffer.CopyTo(resource->data.data(), image_size);
  staging_buffer.Unmap();

  CreateImage(texture.allocation, device_, allocator_, texture.width,
              texture.height, texture.mip_levels, VK_SAMPLE_COUNT_1_BIT,
              texture.format, VK_IMAGE_TILING_OPTIMAL,
              VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                  VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
              VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

  CommandBuffer transfer_command_buffer{device_, GetTransferCommandPool()};
  transfer_command_buffer.Allocate();
  transfer_command_buffer.Record();

  TransitionImageLayout(transfer_command_buffer, texture.allocation.image,
                        VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED,
                        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                        texture.mip_levels);

  auto is_specific_transfer_family{
      device_.GetQueueFamilyIndices().IsSpecificTransferFamily()};

  VkCommandBuffer command_buffer{is_specific_transfer_family
                                     ? VK_NULL_HANDLE
                                     : transfer_command_buffer.GetHandle()};

  CommandBuffer graphics_command_buffer{device_, upload_context_.command_pool,
                                        command_buffer};

  if (is_specific_transfer_family) {
    graphics_command_buffer.Allocate();
    graphics_command_buffer.Record();
    transfer_command_buffer.Submit(device_.GetTransferQueue());
    transfer_command_buffer.Free();
  }

  CopyBufferToImage(graphics_command_buffer, staging_buffer.buffer,
                    texture.allocation.image, texture.width, texture.height);

  u32 src_queue_family_index{VK_QUEUE_FAMILY_IGNORED};
  u32 dst_queue_family_index{VK_QUEUE_FAMILY_IGNORED};

  graphics_command_buffer.Submit(device_.GetGraphicsQueue());
  graphics_command_buffer.Free();

  staging_buffer.Destroy();
  GenerateMipmaps(texture);

  // Create texture image view.
  texture.view =
      CreateImageView(device_, texture.allocation.image, texture.format,
                      VK_IMAGE_ASPECT_COLOR_BIT, texture.mip_levels);

  texture.sampler = texture_sampler_;
  return &texture;
}

void VulkanDriver::GenerateMipmaps(const VulkanTexture& texture) {
  // Check if image format supports linear blitting.
  VkFormatProperties format_properties;
  vkGetPhysicalDeviceFormatProperties(device_.GetPhysicalDevice(),
                                      texture.format, &format_properties);

  COMET_ASSERT(
      static_cast<bool>(format_properties.optimalTilingFeatures &
                        VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT),
      "Texture image format does not support linear blitting");

  // vkCmdBlitImage is only in a queue with graphics capacility.
  CommandBuffer command_buffer{device_,
                               frame_data_[current_frame_].command_pool};
  command_buffer.Allocate();
  command_buffer.Record();

  // Will be reused several times.
  VkImageMemoryBarrier barrier{};
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  barrier.image = texture.allocation.image;
  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  barrier.subresourceRange.baseArrayLayer = 0;
  barrier.subresourceRange.layerCount = 1;
  barrier.subresourceRange.levelCount = 1;

  auto mip_width{texture.width};
  auto mip_height{texture.height};

  for (u32 mip_level{1}; mip_level < texture.mip_levels; ++mip_level) {
    barrier.subresourceRange.baseMipLevel = mip_level - 1;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

    vkCmdPipelineBarrier(command_buffer.GetHandle(),
                         VK_PIPELINE_STAGE_TRANSFER_BIT,
                         VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, VK_NULL_HANDLE,
                         0, VK_NULL_HANDLE, 1, &barrier);

    VkImageBlit blit{};
    blit.srcOffsets[0] = {0, 0, 0};
    blit.srcOffsets[1] = {static_cast<s32>(mip_width),
                          static_cast<s32>(mip_height), 1};
    blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blit.srcSubresource.mipLevel = mip_level - 1;
    blit.srcSubresource.baseArrayLayer = 0;
    blit.srcSubresource.layerCount = 1;
    blit.dstOffsets[0] = {0, 0, 0};
    blit.dstOffsets[1] = {mip_width > 1 ? static_cast<s32>(mip_width / 2) : 1,
                          mip_height > 1 ? static_cast<s32>(mip_height / 2) : 1,
                          1};
    blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blit.dstSubresource.mipLevel = mip_level;
    blit.dstSubresource.baseArrayLayer = 0;
    blit.dstSubresource.layerCount = 1;

    vkCmdBlitImage(
        command_buffer.GetHandle(), texture.allocation.image,
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, texture.allocation.image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_LINEAR);

    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(command_buffer.GetHandle(),
                         VK_PIPELINE_STAGE_TRANSFER_BIT,
                         VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0,
                         VK_NULL_HANDLE, 0, VK_NULL_HANDLE, 1, &barrier);

    if (mip_width > 1) mip_width /= 2;
    if (mip_height > 1) mip_height /= 2;
  }

  barrier.subresourceRange.baseMipLevel = texture.mip_levels - 1;
  barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
  barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
  barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

  vkCmdPipelineBarrier(command_buffer.GetHandle(),
                       VK_PIPELINE_STAGE_TRANSFER_BIT,
                       VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0,
                       VK_NULL_HANDLE, 0, VK_NULL_HANDLE, 1, &barrier);

  command_buffer.Submit(device_.GetGraphicsQueue());
  command_buffer.Free();
}

VulkanRenderProxy* VulkanDriver::TryGetVulkanRenderProxy(
    const resource::MeshResource* resource) {
  for (uindex i{0}; i < proxies_.size(); ++i) {
    if (proxies_[i].mesh->id == GenerateMeshId(resource)) {
      return &proxies_[i];
    }
  }

  return nullptr;
}

std::vector<const char*> VulkanDriver::GetRequiredExtensions() {
  u32 glfw_extension_count{0};
  const char** glfw_extensions{
      glfwGetRequiredInstanceExtensions(&glfw_extension_count)};

  std::vector<const char*> extensions(glfw_extensions,
                                      glfw_extensions + glfw_extension_count);

#ifdef COMET_VULKAN_DEBUG_MODE
  extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif  // COMET_VULKAN_DEBUG_MODE

  return extensions;
}

void VulkanDriver::UploadVulkanMesh(VulkanMesh& mesh) {
  const auto vertex_buffer_size{sizeof(VulkanVertex) * mesh.vertices.size()};
  const auto index_buffer_size{sizeof(VulkanIndex) * mesh.indices.size()};
  const auto buffer_size{vertex_buffer_size + index_buffer_size};

  AllocatedBuffer staging_buffer{};
  VkDeviceMemory staging_buffer_memory{VK_NULL_HANDLE};

  CreateBuffer(staging_buffer, buffer_size, allocator_,
               VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_AUTO,
               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                   VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
               VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);

  staging_buffer.Map();
  staging_buffer.CopyTo(mesh.vertices.data(), vertex_buffer_size);
  staging_buffer.CopyTo(mesh.indices.data(), index_buffer_size,
                        vertex_buffer_size);
  staging_buffer.Unmap();

  CreateBuffer(mesh.vertex_buffer, buffer_size, allocator_,
               VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                   VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
                   VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
               VMA_MEMORY_USAGE_AUTO, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

  CopyBuffer(staging_buffer.buffer, mesh.vertex_buffer.buffer, buffer_size);
  staging_buffer.Destroy();
}

void VulkanDriver::CopyBuffer(VkBuffer src_buffer, VkBuffer dst_buffer,
                              VkDeviceSize size) {
  CommandBuffer command_buffer{device_,
                               frame_data_[current_frame_].command_pool};

  command_buffer.Allocate();
  command_buffer.Record();

  VkBufferCopy copy_region{};
  copy_region.srcOffset = 0;
  copy_region.dstOffset = 0;
  copy_region.size = size;

  vkCmdCopyBuffer(command_buffer.GetHandle(), src_buffer, dst_buffer, 1,
                  &copy_region);
  command_buffer.Submit(device_.GetGraphicsQueue());
  command_buffer.Free();
}

void VulkanDriver::CopyBufferToImage(CommandBuffer& command_buffer,
                                     VkBuffer buffer, VkImage image, u32 width,
                                     u32 height) {
  VkBufferImageCopy region{};
  region.bufferOffset = 0;
  region.bufferRowLength = 0;
  region.bufferImageHeight = 0;
  region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  region.imageSubresource.mipLevel = 0;
  region.imageSubresource.baseArrayLayer = 0;
  region.imageSubresource.layerCount = 1;
  region.imageOffset = {0, 0, 0};
  region.imageExtent = {width, height, 1};

  vkCmdCopyBufferToImage(command_buffer.GetHandle(), buffer, image,
                         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
}

void VulkanDriver::TransitionImageLayout(
    const CommandBuffer& command_buffer, VkImage image, VkFormat format,
    VkImageLayout old_layout, VkImageLayout new_layout, u32 mip_levels,
    u32 src_queue_family_index, u32 dst_queue_family_index) {
  VkImageMemoryBarrier barrier{};
  VkPipelineStageFlags source_stage;
  VkPipelineStageFlags destination_stage;
  VkCommandPool command_pool{VK_NULL_HANDLE};
  VkQueue queue{VK_NULL_HANDLE};

  if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED &&
      new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    destination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    command_pool = GetTransferCommandPool();
    queue = device_.GetTransferQueue();
  } else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
             new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    command_pool = frame_data_[current_frame_].command_pool;
    queue = device_.GetGraphicsQueue();
  } else if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED &&
             new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    destination_stage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    command_pool = frame_data_[current_frame_].command_pool;
    queue = device_.GetGraphicsQueue();
  } else {
    COMET_ASSERT(false, "Unsupported layout transition!");
  }

  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  barrier.oldLayout = old_layout;
  barrier.newLayout = new_layout;
  barrier.srcQueueFamilyIndex = src_queue_family_index;
  barrier.dstQueueFamilyIndex = dst_queue_family_index;
  barrier.image = image;

  if (new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

    if (HasStencilComponent(format)) {
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
                       destination_stage, 0, 0, VK_NULL_HANDLE, 0,
                       VK_NULL_HANDLE, 1, &barrier);
}

VulkanMaterialDescr VulkanDriver::GenerateVulkanMaterial(
    const resource::MaterialResource& resource) {
  std::vector<VulkanTextureTuple> texture_tuples{};
  texture_tuples.reserve(resource.texture_tuples.size());

  for (const auto& tuple : resource.texture_tuples) {
    texture_tuples.emplace_back(VulkanTextureTuple{
        UploadVulkanTexture(Engine::Get()
                                .GetResourceManager()
                                .LoadFromResourceId<resource::TextureResource>(
                                    tuple.texture_id)),
        tuple.type});
  }

  return VulkanMaterialDescr{resource.id, nullptr, std::move(texture_tuples)};
}

bool VulkanDriver::IsVulkanMesh(const resource::MeshResource* resource) {
  return meshes_.find(resource->resource_id) != meshes_.cend();
}

VkCommandPool VulkanDriver::GetTransferCommandPool() {
  if (transfer_command_pool_ == VK_NULL_HANDLE) {
    return frame_data_[current_frame_].command_pool;
  }

  return transfer_command_pool_;
}

#ifdef COMET_VULKAN_DEBUG_MODE
void VulkanDriver::InitializeDebugMessenger() {
  auto create_info{init::GetDebugUtilsMessengerCreateInfo(
      VulkanDriver::LogVulkanValidationMessage)};

  COMET_CHECK_VK(
      CreateDebugUtilsMessengerEXT(instance_, &create_info, VK_NULL_HANDLE,
                                   &debug_messenger_),
      "Failed to set up debug messenger");
}

void VulkanDriver::DestroyDebugMessenger() {
  if (debug_messenger_ == VK_NULL_HANDLE) {
    return;
  }

  DestroyDebugUtilsMessengerEXT(instance_, debug_messenger_, VK_NULL_HANDLE);
  debug_messenger_ = VK_NULL_HANDLE;
}

bool VulkanDriver::AreValidationLayersSupported() {
  u32 layer_count;
  vkEnumerateInstanceLayerProperties(&layer_count, VK_NULL_HANDLE);
  std::vector<VkLayerProperties> available_layers(layer_count);
  vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());

  for (const char* layer_name : kValidationLayers_) {
    auto is_layer_found{false};

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

#ifdef COMET_VULKAN_ABORT_ON_ERROR
      COMET_ASSERT(false, callback_data->pMessage);
#endif  // COMET_VULKAN_ABORT_ON_ERROR
      break;
  }

  return VK_FALSE;
}
#endif  // COMET_VULKAN_DEBUG_MODE
}  // namespace vk
}  // namespace rendering
}  // namespace comet
