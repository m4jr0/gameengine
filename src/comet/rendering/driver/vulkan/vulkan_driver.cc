// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"

#include "vulkan_driver.h"

#define VMA_IMPLEMENTATION
#include "comet/core/c_string.h"
#include "comet/core/frame/frame_utils.h"
#include "comet/core/logger.h"
#include "comet/core/type/array.h"
#include "comet/event/event_manager.h"
#include "comet/profiler/profiler.h"
#include "comet/rendering/driver/vulkan/data/vulkan_frame.h"
#include "comet/rendering/driver/vulkan/data/vulkan_image.h"
#include "comet/rendering/driver/vulkan/utils/vulkan_command_buffer_utils.h"
#include "comet/rendering/driver/vulkan/utils/vulkan_initializer_utils.h"
#include "comet/rendering/driver/vulkan/vulkan_alloc.h"
#include "comet/rendering/driver/vulkan/vulkan_debug.h"
#include "comet/rendering/rendering_common.h"
#include "comet/rendering/window/window_event.h"

namespace comet {
namespace rendering {
namespace vk {
VulkanDriver::VulkanDriver(const VulkanDriverDescr& descr)
    : Driver(descr),
      vulkan_major_version_{descr.vulkan_major_version},
      vulkan_minor_version_{descr.vulkan_minor_version},
      vulkan_patch_version_{descr.vulkan_patch_version},
      vulkan_variant_version_{descr.vulkan_variant_version},
      max_frames_in_flight_{static_cast<u8>(descr.max_frames_in_flight)} {
  WindowDescr window_descr{};
  window_descr.width = descr.window_width;
  window_descr.height = descr.window_height;
  SetName(window_descr, app_name_, app_name_len_);
  window_ = std::make_unique<VulkanGlfwWindow>(window_descr);
}

void VulkanDriver::Initialize() {
  Driver::Initialize();
  COMET_LOG_RENDERING_DEBUG("Initializing Vulkan driver.");
  window_->Initialize();
  COMET_ASSERT(window_->IsInitialized(), " GLFW window is not initialized!");
  InitializeVulkanInstance();

#ifdef COMET_DEBUG_RENDERING
  InitializeDebugMessenger();
  InitializeDebugReportCallback();
#endif  // COMET_DEBUG_RENDERING

  window_->AttachSurface(instance_handle_);

  DeviceDescr device_descr{};
  device_descr.instance_handle = instance_handle_;
  device_descr.surface_handle = window_->GetSurfaceHandle();
  device_descr.is_sampler_anisotropy = is_sampler_anisotropy_;
  device_descr.is_sample_rate_shading = is_sample_rate_shading_;
  device_descr.anti_aliasing_type = anti_aliasing_type_;
  device_ = std::make_unique<Device>(device_descr);
  device_->Initialize();

  COMET_VK_INITIALIZE_DEBUG_LABELS(instance_handle_, device_->GetHandle());
  COMET_VK_SET_DEBUG_LABEL(device_->GetGraphicsQueueHandle(), "graphics_queue");
  COMET_VK_SET_DEBUG_LABEL(device_->GetPresentQueueHandle(), "present_queue");
  COMET_VK_SET_DEBUG_LABEL(device_->GetTransferQueueHandle(), "transfer_queue");

  ContextDescr context_descr{};
  context_descr.vulkan_major_version = vulkan_major_version_;
  context_descr.vulkan_minor_version = vulkan_minor_version_;
  context_descr.vulkan_patch_version = vulkan_patch_version_;
  context_descr.vulkan_variant_version = vulkan_variant_version_;
  context_descr.max_frames_in_flight = max_frames_in_flight_;
  context_descr.is_sampler_anisotropy = is_sampler_anisotropy_;
  context_descr.is_sample_rate_shading = is_sample_rate_shading_;
  context_descr.max_object_count = kDefaultMaxObjectCount_;
  context_descr.instance_handle = instance_handle_;
  context_descr.device = device_.get();
  context_ = std::make_unique<Context>(context_descr);
  context_->Initialize();

  SwapchainDescr swapchain_descr{};
  swapchain_descr.is_vsync = is_vsync_;
  swapchain_descr.is_triple_buffering = is_triple_buffering_;
  swapchain_descr.window = window_.get();
  swapchain_descr.context = context_.get();
  swapchain_ = std::make_unique<Swapchain>(swapchain_descr);
  swapchain_->Initialize();

  InitializeHandlers();
}

void VulkanDriver::Shutdown() {
  device_->WaitIdle();
  DestroyHandlers();
  swapchain_->Destroy();
  context_->Destroy();
  device_->Destroy();
#ifdef COMET_DEBUG_RENDERING
  DestroyDebugReportCallback();
  DestroyDebugMessenger();
#endif  // COMET_DEBUG_RENDERING

  if (window_->IsInitialized()) {
    window_->DetachSurface(instance_handle_);
  }

  DestroyInstance();

  if (window_->IsInitialized()) {
    window_->Destroy();
  }

  vulkan_major_version_ = 0;
  vulkan_minor_version_ = 0;
  vulkan_patch_version_ = 0;
  vulkan_variant_version_ = 0;
  max_frames_in_flight_ = 2;
  instance_handle_ = VK_NULL_HANDLE;
  Driver::Shutdown();
}

void VulkanDriver::Update(frame::FramePacket* packet) {
  packet->projection_matrix[1][1] *= -1;  // Axis is inverted in Vulkan.
  window_->Update();

  if (swapchain_->IsReloadNeeded()) {
    ApplyWindowResize();
    packet->is_rendering_skipped = true;
  } else if (!swapchain_->IsPresentationAvailable()) {
    packet->is_rendering_skipped = true;
  }

  PreDraw(packet);
  Draw(packet);
  PostDraw(packet);

  context_->GoToNextFrame();
}

DriverType VulkanDriver::GetType() const noexcept { return DriverType::Vulkan; }

void VulkanDriver::SetSize(WindowSize width, WindowSize height) {
  window_->SetSize(width, height);
}

Window* VulkanDriver::GetWindow() { return window_.get(); }

u32 VulkanDriver::GetDrawCount() const {
  return render_proxy_handler_->GetVisibleCount();
}

void VulkanDriver::InitializeVulkanInstance() {
  COMET_LOG_RENDERING_DEBUG("Initializing  instance.");
  u32 extension_count{0};
  vkEnumerateInstanceExtensionProperties(VK_NULL_HANDLE, &extension_count,
                                         VK_NULL_HANDLE);
  frame::FrameArray<VkExtensionProperties> extensions{};
  extensions.Resize(extension_count);
  vkEnumerateInstanceExtensionProperties(VK_NULL_HANDLE, &extension_count,
                                         extensions.GetData());

  COMET_LOG_RENDERING_DEBUG("Available  extensions:");

#ifdef COMET_DEBUG
  for (const auto& extension : extensions) {
    COMET_LOG_RENDERING_DEBUG("\t", extension.extensionName);
  }
#endif  // COMET_DEBUG

  VkApplicationInfo app_info{};
  app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  app_info.pApplicationName = app_name_;
  app_info.applicationVersion = VK_MAKE_API_VERSION(
      0, app_major_version_, app_minor_version_, app_patch_version_);

  app_info.pEngineName = version::kCometName.data();
  app_info.engineVersion = VK_MAKE_API_VERSION(0, version::kCometVersionMajor,
                                               version::kCometVersionMinor,
                                               version::kCometVersionPatch);
  app_info.apiVersion =
      VK_MAKE_API_VERSION(vulkan_variant_version_, vulkan_major_version_,
                          vulkan_minor_version_, vulkan_patch_version_);

  VkInstanceCreateInfo create_info{};
  create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  create_info.pApplicationInfo = &app_info;

#ifdef COMET_DEBUG_RENDERING
  frame::FrameArray<VkValidationFeatureEnableEXT> enabled_validation_features{
#ifdef COMET_VALIDATION_GPU_ASSISTED_EXT
      VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT,
#endif  // COMET_VALIDATION_GPU_ASSISTED_EXT

#ifdef COMET_VALIDATION_GPU_ASSISTED_RESERVE_BINDING_SLOT_EXT
      VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_RESERVE_BINDING_SLOT_EXT,
#endif  // COMET_VALIDATION_GPU_ASSISTED_RESERVE_BINDING_SLOT_EXT

#ifdef COMET_VALIDATION_BEST_PRACTICES_EXT
      VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT,
#endif  // COMET_VALIDATION_BEST_PRACTICES_EXT

#ifdef COMET_VALIDATION_DEBUG_PRINTF_EXT
      VK_VALIDATION_FEATURE_ENABLE_DEBUG_PRINTF_EXT,
#endif  // COMET_VALIDATION_DEBUG_PRINTF_EXT

#ifdef COMET_VALIDATION_SYNCHRONIZATION_VALIDATION_EXT
      VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION_EXT,
#endif  // COMET_VALIDATION_SYNCHRONIZATION_VALIDATION_EXT
  };

  VkValidationFeaturesEXT validation_features{};
  validation_features.sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT;
  validation_features.enabledValidationFeatureCount =
      static_cast<u32>(enabled_validation_features.GetSize());
  validation_features.pEnabledValidationFeatures =
      enabled_validation_features.GetData();
  validation_features.disabledValidationFeatureCount = 0;
  validation_features.pDisabledValidationFeatures = VK_NULL_HANDLE;
  validation_features.pNext = VK_NULL_HANDLE;
#endif  // COMET_DEBUG_RENDERING

  const auto required_extensions{GetRequiredExtensions()};
  const u32 required_extension_count{
      static_cast<u32>(required_extensions.GetSize())};

  COMET_LOG_RENDERING_DEBUG("Required extensions:");

  for (usize i{0}; i < required_extension_count; ++i) {
    const auto* required_extension{required_extensions[i]};
    COMET_LOG_RENDERING_DEBUG("\t", required_extension);
    auto is_found{false};

    for (const auto& extension : extensions) {
      if (AreStringsEqual(required_extension, extension.extensionName)) {
        is_found = true;
        break;
      }
    }

    if (!is_found) {
      COMET_LOG_RENDERING_ERROR("Required extension is not available: ",
                                required_extension);
    }
  }

  create_info.enabledExtensionCount = required_extension_count;
  create_info.ppEnabledExtensionNames = required_extensions.GetData();

#ifndef COMET_DEBUG_RENDERING
  create_info.enabledLayerCount = 0;
  create_info.pNext = VK_NULL_HANDLE;
#else
  COMET_ASSERT(AreValidationLayersSupported(),
               "At least one validation layer is not available!");

  create_info.enabledLayerCount =
      static_cast<u32>(kValidationLayers_.GetSize());
  create_info.ppEnabledLayerNames = kValidationLayers_.GetData();
  auto debug_create_info{init::GenerateDebugUtilsMessengerCreateInfo(
      VulkanDriver::LogVulkanValidationMessage)};
  debug_create_info.pNext = &validation_features;
  create_info.pNext =
      static_cast<VkDebugUtilsMessengerCreateInfoEXT*>(&debug_create_info);
#endif  // !COMET_DEBUG_RENDERING

  COMET_CHECK_VK(
      vkCreateInstance(&create_info,
                       MemoryCallbacks::Get().GetAllocCallbacksHandle(),
                       &instance_handle_),
      "Failed to create instance!");
}

void VulkanDriver::InitializeHandlers() {
  DescriptorHandlerDescr descriptor_handler_descr{};
  descriptor_handler_descr.context = context_.get();
  descriptor_handler_ =
      std::make_unique<DescriptorHandler>(descriptor_handler_descr);

  TextureHandlerDescr texture_handler_descr{};
  texture_handler_descr.context = context_.get();
  texture_handler_ = std::make_unique<TextureHandler>(texture_handler_descr);

  PipelineHandlerDescr pipeline_handler_descr{};
  pipeline_handler_descr.context = context_.get();
  pipeline_handler_ = std::make_unique<PipelineHandler>(pipeline_handler_descr);

  ShaderModuleHandlerDescr shader_module_handler_descr{};
  shader_module_handler_descr.context = context_.get();
  shader_module_handler_ =
      std::make_unique<ShaderModuleHandler>(shader_module_handler_descr);

  MaterialHandlerDescr material_handler_descr{};
  material_handler_descr.context = context_.get();
  material_handler_descr.texture_handler = texture_handler_.get();
  material_handler_ = std::make_unique<MaterialHandler>(material_handler_descr);

  ShaderHandlerDescr shader_handler_descr{};
  shader_handler_descr.context = context_.get();
  shader_handler_descr.shader_module_handler = shader_module_handler_.get();
  shader_handler_descr.pipeline_handler = pipeline_handler_.get();
  shader_handler_descr.material_handler = material_handler_.get();
  shader_handler_descr.texture_handler = texture_handler_.get();
  shader_handler_descr.descriptor_handler = descriptor_handler_.get();
  shader_handler_ = std::make_unique<ShaderHandler>(shader_handler_descr);

  MeshHandlerDescr mesh_handler_descr{};
  mesh_handler_descr.context = context_.get();
  mesh_handler_ = std::make_unique<MeshHandler>(mesh_handler_descr);

  RenderPassHandlerDescr render_pass_handler_descr{};
  render_pass_handler_descr.context = context_.get();
  render_pass_handler_descr.swapchain = swapchain_.get();
  render_pass_handler_ =
      std::make_unique<RenderPassHandler>(render_pass_handler_descr);

  RenderProxyHandlerDescr proxy_handler_descr{};
  proxy_handler_descr.context = context_.get();
  proxy_handler_descr.material_handler = material_handler_.get();
  proxy_handler_descr.mesh_handler = mesh_handler_.get();
  proxy_handler_descr.shader_handler = shader_handler_.get();
  render_proxy_handler_ =
      std::make_unique<RenderProxyHandler>(proxy_handler_descr);

  ViewHandlerDescr view_handler_descr{};
  view_handler_descr.context = context_.get();
  view_handler_descr.shader_handler = shader_handler_.get();
  view_handler_descr.pipeline_handler = pipeline_handler_.get();
  view_handler_descr.render_pass_handler = render_pass_handler_.get();
  view_handler_descr.render_proxy_handler = render_proxy_handler_.get();
  view_handler_descr.rendering_view_descrs = &rendering_view_descrs_;
  view_handler_descr.window = window_.get();
  view_handler_ = std::make_unique<ViewHandler>(view_handler_descr);

  descriptor_handler_->Initialize();
  texture_handler_->Initialize();
  pipeline_handler_->Initialize();
  shader_module_handler_->Initialize();
  shader_handler_->Initialize();
  material_handler_->Initialize();
  mesh_handler_->Initialize();
  render_pass_handler_->Initialize();
  render_proxy_handler_->Initialize();
  view_handler_->Initialize();
}

void VulkanDriver::DestroyHandlers() {
  // Order is important to improve performance.
  shader_module_handler_->Shutdown();
  render_proxy_handler_->Shutdown();
  shader_handler_->Shutdown();
  texture_handler_->Shutdown();
  material_handler_->Shutdown();
  mesh_handler_->Shutdown();
  pipeline_handler_->Shutdown();
  render_pass_handler_->Shutdown();
  view_handler_->Shutdown();
  descriptor_handler_->Shutdown();

  shader_module_handler_ = nullptr;
  shader_handler_ = nullptr;
  texture_handler_ = nullptr;
  material_handler_ = nullptr;
  mesh_handler_ = nullptr;
  pipeline_handler_ = nullptr;
  render_pass_handler_ = nullptr;
  render_proxy_handler_ = nullptr;
  view_handler_ = nullptr;
}

void VulkanDriver::DestroyInstance() {
  if (instance_handle_ == VK_NULL_HANDLE) {
    return;
  }

  vkDestroyInstance(instance_handle_,
                    MemoryCallbacks::Get().GetAllocCallbacksHandle());
  instance_handle_ = VK_NULL_HANDLE;
}

void VulkanDriver::ApplyWindowResize() {
  device_->WaitIdle();
  swapchain_->HandlePreSwapchainReload();
  context_->HandlePreSwapchainReload();

  if (!swapchain_->Reload()) {
    return;
  }

  context_->HandlePostSwapchainReload();
  swapchain_->HandlePostSwapchainReload();
  const auto& extent{swapchain_->GetExtent()};
  view_handler_->SetSize(static_cast<WindowSize>(extent.width),
                         static_cast<WindowSize>(extent.height));
}

void VulkanDriver::PreDraw(const frame::FramePacket* packet) {
  COMET_PROFILE("VulkanDriver::PreDraw");
  auto& frame_data{context_->GetFrameData()};

  COMET_CHECK_VK(
      vkWaitForFences(device_->GetHandle(), 1, &frame_data.render_fence_handle,
                      VK_TRUE, static_cast<u64>(-1)),
      "Something wrong happened while waiting for render fence!");

  descriptor_handler_->ResetDynamic();

  if (packet->is_rendering_skipped) {
    return;
  }

  auto result{
      swapchain_->AcquireNextImage(frame_data.present_semaphore_handle)};

  if (result == VK_ERROR_OUT_OF_DATE_KHR) {
    ApplyWindowResize();
    return;
  }

  COMET_ASSERT(result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR,
               "Failed to acquire swapchain image!");

  return;
}

void VulkanDriver::PostDraw(const frame::FramePacket* packet) {
  COMET_PROFILE("VulkanDriver::PostDraw");

  if (packet->is_rendering_skipped) {
    return;
  }

  const auto result{swapchain_->QueuePresent()};

  if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
    ApplyWindowResize();
    return;
  }

  COMET_ASSERT(result == VK_SUCCESS, "Failed to present swap chain image!");
}

// TODO(m4jr0): Refactor this part to improve clarity and maintainability. The
// current intermixing of `is_rendering_skipped` checks has made the logic
// tangled. Separate internal (non-rendering) updates cleanly from
// rendering-related updates. This will make the code easier to understand,
// extend, and use correctly.
void VulkanDriver::Draw(frame::FramePacket* packet) {
  COMET_PROFILE("VulkanDriver::Draw");
  auto& frame_data{context_->GetFrameData()};

  vkWaitForFences(device_->GetHandle(), 1, &frame_data.render_fence_handle,
                  VK_TRUE, static_cast<u64>(-1));

  // Reset fence if work is submitted.
  COMET_CHECK_VK(
      vkResetFences(device_->GetHandle(), 1, &frame_data.render_fence_handle),
      "Unable to reset render fence!");

  auto command_data{
      GenerateCommandData(*device_, frame_data.command_buffer_handle)};

  RecordCommand(command_data);

  mesh_handler_->Wait();
  mesh_handler_->Update(packet);
  render_proxy_handler_->Update(packet);

  if (!packet->is_rendering_skipped) {
    PrepareRenderBarriers(command_data);
    PrepareViewportAndScissor(command_data);
  }

  view_handler_->Update(packet);

  VkPipelineStageFlags2 wait_stage{
      VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT};

  frame::FrameArray<VkSemaphore> signal_semaphores{};
  signal_semaphores.Reserve(2);
  signal_semaphores.PushBack(context_->GetRenderSemaphoreHandle());
  signal_semaphores.PushBack(*context_->GetTransferSemaphoreHandle());

  const auto wait_value{context_->GetTransferTimelineValue()};
  context_->UpdateTransferTimelineValue();
  const auto signal_value{context_->GetTransferTimelineValue()};

  frame::FrameArray<VkSemaphoreSubmitInfo> wait_infos{};
  wait_infos.Reserve(2);

  if (!packet->is_rendering_skipped) {
    auto& wait_present{wait_infos.EmplaceBack()};
    wait_present.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
    wait_present.semaphore = frame_data.present_semaphore_handle;
    wait_present.value = 0;
    wait_present.stageMask = wait_stage;
    wait_present.deviceIndex = 0;
  }

  auto& wait_transfer{wait_infos.EmplaceBack()};
  wait_transfer.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
  wait_transfer.semaphore = *context_->GetTransferSemaphoreHandle();
  wait_transfer.value = wait_value;
  wait_transfer.stageMask = VK_PIPELINE_STAGE_2_COPY_BIT;
  wait_transfer.deviceIndex = 0;

  frame::FrameArray<VkSemaphoreSubmitInfo> signal_infos{};
  signal_infos.Reserve(2);

  if (!packet->is_rendering_skipped) {
    auto& signal_render{signal_infos.EmplaceBack()};
    signal_render.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
    signal_render.semaphore = context_->GetRenderSemaphoreHandle();
    signal_render.value = 0;
    signal_render.stageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT;
    signal_render.deviceIndex = 0;
  }

  auto& signal_transfer{signal_infos.EmplaceBack()};
  signal_transfer.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
  signal_transfer.semaphore = *context_->GetTransferSemaphoreHandle();
  signal_transfer.value = signal_value;
  signal_transfer.stageMask = VK_PIPELINE_STAGE_2_COPY_BIT;
  signal_transfer.deviceIndex = 0;

  SubmitCommand2(command_data, device_->GetGraphicsQueueHandle(),
                 frame_data.render_fence_handle, wait_infos.GetData(),
                 static_cast<u32>(wait_infos.GetSize()), signal_infos.GetData(),
                 static_cast<u32>(signal_infos.GetSize()), VK_NULL_HANDLE);
}

void VulkanDriver::PrepareRenderBarriers(const CommandData& command_data) {
  VkImageMemoryBarrier barrier{};
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

  const auto& swapchain_images{swapchain_->GetImages()};
  barrier.image = swapchain_images[context_->GetImageIndex()].handle;

  barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  barrier.subresourceRange.baseMipLevel = 0;
  barrier.subresourceRange.levelCount = 1;
  barrier.subresourceRange.baseArrayLayer = 0;
  barrier.subresourceRange.layerCount = 1;
  barrier.srcAccessMask = 0;
  barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

  vkCmdPipelineBarrier(command_data.command_buffer_handle,
                       VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                       VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0,
                       VK_NULL_HANDLE, 0, VK_NULL_HANDLE, 1, &barrier);
}

void VulkanDriver::PrepareViewportAndScissor(const CommandData& command_data) {
  const auto& extent{swapchain_->GetExtent()};

  VkViewport viewport{};
  viewport.x = 0.0f;
  viewport.y = 0.0f;
  viewport.width = static_cast<f32>(extent.width);
  viewport.height = static_cast<f32>(extent.height);
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;

  VkRect2D scissor{};
  scissor.offset = {0, 0};
  scissor.extent = extent;

  vkCmdSetViewport(command_data.command_buffer_handle, 0, 1, &viewport);
  vkCmdSetScissor(command_data.command_buffer_handle, 0, 1, &scissor);
}

frame::FrameArray<const schar*> VulkanDriver::GetRequiredExtensions() {
  u32 glfw_extension_count{0};
  const auto** glfw_extensions{
      glfwGetRequiredInstanceExtensions(&glfw_extension_count)};

  frame::FrameArray<const schar*> extensions{};
  extensions.Reserve(glfw_extension_count);

  for (usize i{0}; i < glfw_extension_count; ++i) {
    extensions.PushBack(glfw_extensions[i]);
  }

#ifdef COMET_DEBUG_RENDERING
  extensions.Reserve(extensions.GetSize() + 2);
  extensions.PushBack(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  extensions.PushBack(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
#endif  // COMET_DEBUG_RENDERING

  return extensions;
}

#ifdef COMET_DEBUG_RENDERING
void VulkanDriver::InitializeDebugMessenger() {
  auto create_info{init::GenerateDebugUtilsMessengerCreateInfo(
      VulkanDriver::LogVulkanValidationMessage)};

  COMET_CHECK_VK(debug::CreateDebugUtilsMessengerEXT(
                     instance_handle_, &create_info, VK_NULL_HANDLE,
                     &debug_messenger_handle_),
                 "Failed to set up debug messenger");
}

void VulkanDriver::DestroyDebugMessenger() {
  if (debug_messenger_handle_ == VK_NULL_HANDLE) {
    return;
  }

  debug::DestroyDebugUtilsMessengerEXT(instance_handle_,
                                       debug_messenger_handle_, VK_NULL_HANDLE);
  debug_messenger_handle_ = VK_NULL_HANDLE;
}

void VulkanDriver::InitializeDebugReportCallback() {
  COMET_CHECK_VK(
      debug::CreateDebugReportCallback(
          instance_handle_,
          VK_DEBUG_REPORT_DEBUG_BIT_EXT |
              VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT |
              VK_DEBUG_REPORT_INFORMATION_BIT_EXT |
              VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_ERROR_BIT_EXT,
          VulkanDriver::LogVulkanDebugReportMessage,
          debug_report_callback_handle_),
      "Failed to set up debug report callback");
}

void VulkanDriver::DestroyDebugReportCallback() {
  if (debug_report_callback_handle_ == VK_NULL_HANDLE) {
    return;
  }

  debug::DestroyDebugReportCallback(instance_handle_,
                                    debug_report_callback_handle_);
  debug_report_callback_handle_ = VK_NULL_HANDLE;
}

bool VulkanDriver::AreValidationLayersSupported() {
  u32 layer_count;
  vkEnumerateInstanceLayerProperties(&layer_count, VK_NULL_HANDLE);
  frame::FrameArray<VkLayerProperties> available_layers{};
  available_layers.Resize(layer_count);
  vkEnumerateInstanceLayerProperties(&layer_count, available_layers.GetData());

  for (const schar* layer_name : kValidationLayers_) {
    auto is_layer_found{false};

    for (const auto& layer_properties : available_layers) {
      if (AreStringsEqual(layer_name, layer_properties.layerName)) {
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
    const VkDebugUtilsMessengerCallbackDataEXT* callback_data, void*) {
  constexpr auto kMessageTypeStrMaxLen{48};
  schar message_type_str[kMessageTypeStrMaxLen]{'\0'};

  switch (message_type) {
    case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT:
      Copy(message_type_str, "General", 7);
      break;
    case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT:
      Copy(message_type_str, "Validation", 10);
      break;
    case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT:
      Copy(message_type_str, "Performance", 11);
      break;
    default:
      Copy(message_type_str, "???", 3);
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

VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDriver::LogVulkanDebugReportMessage(
    VkFlags message_flags, VkDebugReportObjectTypeEXT, u64, usize,
    int32_t message_code, const schar* layer_prefix, const schar* message,
    void*) {
  if (message_flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT ||
      message_flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT) {
    COMET_LOG_RENDERING_DEBUG("[Debug | ", layer_prefix, "] ", message_code,
                              ": ", message);
  } else if (message_flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT) {
    COMET_LOG_RENDERING_INFO("[Debug | ", layer_prefix, "] ", message_code,
                             ": ", message);
  } else if (message_flags & VK_DEBUG_REPORT_WARNING_BIT_EXT) {
    COMET_LOG_RENDERING_WARNING("[Debug | ", layer_prefix, "] ", message_code,
                                ": ", message);
  } else if (message_flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) {
    COMET_LOG_RENDERING_ERROR("[Debug | ", layer_prefix, "] ", message_code,
                              ": ", message);

#ifdef COMET_VULKAN_ABORT_ON_ERROR
    COMET_ASSERT(false, "[", layer_prefix, "] ", message_code, ": ", message);
#endif  // COMET_VULKAN_ABORT_ON_ERROR
  }

  return VK_FALSE;
}
#endif  // COMET_DEBUG_RENDERING
}  // namespace vk
}  // namespace rendering
}  // namespace comet
