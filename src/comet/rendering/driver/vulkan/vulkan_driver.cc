// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "vulkan_driver.h"
////////////////////////////////////////////////////////////////////////////////

#define VMA_IMPLEMENTATION
#include "comet/core/c_string.h"
#include "comet/core/debug_label.h"
#include "comet/core/frame/frame_utils.h"
#include "comet/core/logger/logging.h"
#include "comet/core/type/array.h"
#include "comet/profiler/profiler.h"
#include "comet/rendering/driver/vulkan/type/vulkan_frame_type.h"
#include "comet/rendering/driver/vulkan/utils/vulkan_command_buffer_utils.h"
#include "comet/rendering/driver/vulkan/utils/vulkan_initializer_utils.h"
#include "comet/rendering/driver/vulkan/vulkan_alloc.h"
#include "comet/rendering/driver/vulkan/vulkan_debug.h"
#include "comet/rendering/type/rendering_camera_type.h"
#include "comet/rendering/type/rendering_common_type.h"

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

void VulkanDriver::Update(frame::FramePacket* packet) {
  COMET_ASSERT(packet != nullptr, "VulkanDriver::Update",
               "frame packet is null");
  COMET_ASSERT(window_ != nullptr, "VulkanDriver::Update", "window is null");
  COMET_ASSERT(context_ != nullptr, "VulkanDriver::Update", "context is null");

  // Axis is inverted in Vulkan.
  packet->camera_data.projection_matrix[1][1] *= -1;
  window_->Update();

  HandleSwapchainState(packet);
  PreDraw(packet);
  Draw(packet);

  if (packet->can_present) {
    PostDraw();
  }

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

void VulkanDriver::OnInitialize() {
  COMET_LOG_DEBUG(LoggerType::Rendering, "VulkanDriver::OnInitialize",
                  "initializing vulkan driver");

  COMET_ASSERT(window_ != nullptr, "VulkanDriver::OnInitialize",
               "window is null");

  window_->Initialize();

  COMET_ASSERT(window_->IsInitialized(), "VulkanDriver::OnInitialize",
               "window not initialized");

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

void VulkanDriver::OnShutdown() {
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
}

void VulkanDriver::InitializeVulkanInstance() {
  COMET_LOG_DEBUG(LoggerType::Rendering,
                  "VulkanDriver::InitializeVulkanInstance",
                  "initializing vulkan instance");

  u32 extension_count{0};
  vkEnumerateInstanceExtensionProperties(VK_NULL_HANDLE, &extension_count,
                                         VK_NULL_HANDLE);
  frame::FrameArray<VkExtensionProperties> extensions{};
  extensions.Resize(extension_count);
  vkEnumerateInstanceExtensionProperties(VK_NULL_HANDLE, &extension_count,
                                         extensions.GetData());

  COMET_LOG_DEBUG(LoggerType::Rendering,
                  "VulkanDriver::InitializeVulkanInstance",
                  "enumerated instance extensions", "count", extension_count);

#ifdef COMET_DEBUG
  for (const auto& extension : extensions) {
    COMET_LOG_DEBUG(
        LoggerType::Rendering, "VulkanDriver::InitializeVulkanInstance",
        "available instance extension", "name", extension.extensionName);
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

  COMET_ASSERT(glfwVulkanSupported(), "VulkanDriver::InitializeVulkanInstance",
               "glfw reports vulkan not supported");

  const auto required_extensions{GetRequiredExtensions()};
  const auto required_extension_count{
      static_cast<u32>(required_extensions.GetSize())};

  for (usize i{0}; i < required_extension_count; ++i) {
    const auto* required_extension{required_extensions[i]};
    auto is_found{false};

    for (const auto& extension : extensions) {
      if (AreStringsEqual(required_extension, extension.extensionName)) {
        is_found = true;
        break;
      }
    }

    if (!is_found) {
      COMET_LOG_ERROR(
          LoggerType::Rendering, "VulkanDriver::InitializeVulkanInstance",
          "required extension not available", "extension", required_extension);
    } else {
      COMET_LOG_DEBUG(
          LoggerType::Rendering, "VulkanDriver::InitializeVulkanInstance",
          "required extension available", "extension", required_extension);
    }
  }

  create_info.enabledExtensionCount = required_extension_count;
  create_info.ppEnabledExtensionNames = required_extensions.GetData();

#ifndef COMET_DEBUG_RENDERING
  create_info.enabledLayerCount = 0;
  create_info.pNext = VK_NULL_HANDLE;
#else
  COMET_ASSERT(AreValidationLayersSupported(),
               "VulkanDriver::InitializeVulkanInstance",
               "validation layers not supported");

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
      "VulkanDriver::InitializeVulkanInstance", "failed to create instance");
}

void VulkanDriver::InitializeHandlers() {
  DescriptorHandlerDescr descriptor_handler_descr{};
  descriptor_handler_descr.context = context_.get();
  descriptor_handler_ =
      std::make_unique<DescriptorHandler>(descriptor_handler_descr);

  SamplerHandlerDescr sampler_handler_descr{};
  sampler_handler_descr.context = context_.get();
  sampler_handler_ = std::make_unique<SamplerHandler>(sampler_handler_descr);

  TextureHandlerDescr texture_handler_descr{};
  texture_handler_descr.context = context_.get();
  texture_handler_ = std::make_unique<TextureHandler>(texture_handler_descr);

  RenderPassHandlerDescr render_pass_handler_descr{};
  render_pass_handler_descr.context = context_.get();
  render_pass_handler_descr.swapchain = swapchain_.get();
  render_pass_handler_ =
      std::make_unique<RenderPassHandler>(render_pass_handler_descr);

  PipelineHandlerDescr pipeline_handler_descr{};
  pipeline_handler_descr.context = context_.get();
  pipeline_handler_descr.render_pass_handler = render_pass_handler_.get();
  pipeline_handler_ = std::make_unique<PipelineHandler>(pipeline_handler_descr);

  ShaderModuleHandlerDescr shader_module_handler_descr{};
  shader_module_handler_descr.context = context_.get();
  shader_module_handler_ =
      std::make_unique<ShaderModuleHandler>(shader_module_handler_descr);

  MaterialHandlerDescr material_handler_descr{};
  material_handler_descr.context = context_.get();
  material_handler_descr.texture_handler = texture_handler_.get();
  material_handler_descr.sampler_handler = sampler_handler_.get();
  material_handler_ = std::make_unique<MaterialHandler>(material_handler_descr);

  ShaderHandlerDescr shader_handler_descr{};
  shader_handler_descr.context = context_.get();
  shader_handler_descr.shader_module_handler = shader_module_handler_.get();
  shader_handler_descr.pipeline_handler = pipeline_handler_.get();
  shader_handler_descr.material_handler = material_handler_.get();
  shader_handler_descr.texture_handler = texture_handler_.get();
  shader_handler_descr.sampler_handler = sampler_handler_.get();
  shader_handler_descr.descriptor_handler = descriptor_handler_.get();
  shader_handler_descr.render_pass_handler = render_pass_handler_.get();
  shader_handler_ = std::make_unique<ShaderHandler>(shader_handler_descr);

  MeshHandlerDescr mesh_handler_descr{};
  mesh_handler_descr.context = context_.get();
  mesh_handler_ = std::make_unique<MeshHandler>(mesh_handler_descr);

  RenderProxyHandlerDescr proxy_handler_descr{};
  proxy_handler_descr.context = context_.get();
  proxy_handler_descr.material_handler = material_handler_.get();
  proxy_handler_descr.mesh_handler = mesh_handler_.get();
  proxy_handler_descr.shader_handler = shader_handler_.get();
  render_proxy_handler_ =
      std::make_unique<RenderProxyHandler>(proxy_handler_descr);

  LightingHandlerDescr lighting_handler_descr{};
  lighting_handler_descr.context = context_.get();
  lighting_handler_descr.shadow_settings = shadow_settings_;
  lighting_handler_descr.texture_handler = texture_handler_.get();
  lighting_handler_descr.sampler_handler = sampler_handler_.get();
  lighting_handler_descr.render_pass_handler = render_pass_handler_.get();
  lighting_handler_ = std::make_unique<LightingHandler>(lighting_handler_descr);

  ViewHandlerDescr view_handler_descr{};
  view_handler_descr.context = context_.get();
  view_handler_descr.shadow_settings = shadow_settings_;
  view_handler_descr.shader_handler = shader_handler_.get();
  view_handler_descr.material_handler = material_handler_.get();
  view_handler_descr.texture_handler = texture_handler_.get();
  view_handler_descr.pipeline_handler = pipeline_handler_.get();
  view_handler_descr.render_pass_handler = render_pass_handler_.get();
  view_handler_descr.render_proxy_handler = render_proxy_handler_.get();
  view_handler_descr.mesh_handler = mesh_handler_.get();
  view_handler_descr.lighting_handler = lighting_handler_.get();
  view_handler_descr.rendering_view_descrs = &rendering_view_descrs_;
  view_handler_descr.window = window_.get();
  view_handler_ = std::make_unique<ViewHandler>(view_handler_descr);

  descriptor_handler_->Initialize();
  texture_handler_->Initialize();
  pipeline_handler_->Initialize();
  shader_module_handler_->Initialize();
  material_handler_->Initialize();
  mesh_handler_->Initialize();
  render_pass_handler_->Initialize();
  sampler_handler_->Initialize();
  shader_handler_->Initialize();
  lighting_handler_->Initialize();
  render_proxy_handler_->Initialize();
  view_handler_->Initialize();
}

void VulkanDriver::DestroyHandlers() {
  if (view_handler_ != nullptr) {
    view_handler_->Shutdown();
    view_handler_ = nullptr;
  }

  if (lighting_handler_ != nullptr) {
    lighting_handler_->Shutdown();
    lighting_handler_ = nullptr;
  }

  if (render_proxy_handler_ != nullptr) {
    render_proxy_handler_->Shutdown();
    render_proxy_handler_ = nullptr;
  }

  if (material_handler_ != nullptr) {
    material_handler_->Shutdown();
    material_handler_ = nullptr;
  }

  if (mesh_handler_ != nullptr) {
    mesh_handler_->Shutdown();
    mesh_handler_ = nullptr;
  }

  if (sampler_handler_ != nullptr) {
    sampler_handler_->Shutdown();
    sampler_handler_ = nullptr;
  }

  if (shader_handler_ != nullptr) {
    shader_handler_->Shutdown();
    shader_handler_ = nullptr;
  }

  if (texture_handler_ != nullptr) {
    texture_handler_->Shutdown();
    texture_handler_ = nullptr;
  }

  if (shader_module_handler_ != nullptr) {
    shader_module_handler_->Shutdown();
    shader_module_handler_ = nullptr;
  }

  if (pipeline_handler_ != nullptr) {
    pipeline_handler_->Shutdown();
    pipeline_handler_ = nullptr;
  }

  if (render_pass_handler_ != nullptr) {
    render_pass_handler_->Shutdown();
    render_pass_handler_ = nullptr;
  }

  if (descriptor_handler_ != nullptr) {
    descriptor_handler_->Shutdown();
    descriptor_handler_ = nullptr;
  }
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

void VulkanDriver::PreDraw(frame::FramePacket* packet) {
  COMET_PROFILE("VulkanDriver::PreDraw");
  COMET_ASSERT(packet != nullptr, "VulkanDriver::PreDraw",
               "frame packet is null");

  WaitForFences();
  descriptor_handler_->ResetDynamic();

  if (!packet->can_present) {
    return;
  }

  auto& frame_data{context_->GetFrameData()};

  const auto result{
      swapchain_->AcquireNextImage(frame_data.present_semaphore_handle)};

  if (result == VK_ERROR_OUT_OF_DATE_KHR) {
    packet->can_present = false;
    ApplyWindowResize();
    return;
  }

  COMET_ASSERT(result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR,
               "VulkanDriver::PreDraw", "failed to acquire swapchain image",
               "result", static_cast<s32>(result));
}

void VulkanDriver::PostDraw() {
  COMET_PROFILE("VulkanDriver::PostDraw");
  const auto result{swapchain_->QueuePresent()};

  if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
    ApplyWindowResize();
    return;
  }

  COMET_ASSERT(result == VK_SUCCESS, "VulkanDriver::PostDraw",
               "failed to present swapchain image", "result",
               static_cast<s32>(result));
}

void VulkanDriver::Draw(frame::FramePacket* packet) {
  COMET_PROFILE("VulkanDriver::Draw");
  COMET_ASSERT(packet != nullptr, "VulkanDriver::Draw", "frame packet is null");

  auto& frame_data{context_->GetFrameData()};

  ResetRenderFence(frame_data);

  const auto command_data{
      GenerateCommandData(*device_, frame_data.command_buffer_handle)};

  BeginFrameCommandRecording(command_data);
  UpdateGpuSceneState(packet);

  if (packet->can_present) {
    RecordFrame(packet);
  }

  SubmitFrame(packet, command_data, frame_data);
}

void VulkanDriver::WaitForFences() {
  COMET_PROFILE("VulkanDriver::WaitForFences");
  auto& frame_data{context_->GetFrameData()};

  COMET_CHECK_VK(
      vkWaitForFences(device_->GetHandle(), 1, &frame_data.render_fence_handle,
                      VK_TRUE, static_cast<u64>(-1)),
      "VulkanDriver::WaitForFences", "failed to wait for render fence");
}

void VulkanDriver::HandleSwapchainState(frame::FramePacket* packet) {
  COMET_ASSERT(packet != nullptr, "VulkanDriver::HandleSwapchainState",
               "frame packet is null");

  if (swapchain_->IsReloadNeeded()) {
    ApplyWindowResize();
    packet->can_present = false;
  } else if (!swapchain_->IsPresentationAvailable()) {
    packet->can_present = false;
  }
}

void VulkanDriver::ResetRenderFence(FrameData& frame_data) {
  COMET_PROFILE("VulkanDriver::ResetRenderFence");

  // Reset fence if work is submitted.
  COMET_CHECK_VK(
      vkResetFences(device_->GetHandle(), 1, &frame_data.render_fence_handle),
      "VulkanDriver::ResetRenderFence", "failed to reset render fence");
}

void VulkanDriver::UpdateGpuSceneState(frame::FramePacket* packet) {
  COMET_PROFILE("VulkanDriver::UpdateGpuSceneState");
  COMET_ASSERT(packet != nullptr, "VulkanDriver::UpdateGpuSceneState",
               "frame packet is null");

  mesh_handler_->AcquireFromTransferQueueIfNeeded();
  mesh_handler_->Update(packet);
  lighting_handler_->Update(packet);
  render_proxy_handler_->Update(packet);
}

void VulkanDriver::RecordFrame(frame::FramePacket* packet) {
  COMET_PROFILE("VulkanDriver::RecordFrame");
  COMET_ASSERT(packet != nullptr, "VulkanDriver::SubmitFrame",
               "frame packet is null");

  view_handler_->Update(packet);
}

void VulkanDriver::SubmitFrame(const frame::FramePacket* packet,
                               const CommandData& command_data,
                               FrameData& frame_data) {
  COMET_PROFILE("VulkanDriver::SubmitFrame");
  VkPipelineStageFlags2 wait_stage{
      VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT};

  frame::FrameArray<VkSemaphoreSubmitInfo> wait_infos{};
  wait_infos.Reserve(2);

  if (packet->can_present) {
    auto& wait_present{wait_infos.EmplaceBack()};
    wait_present.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
    wait_present.semaphore = frame_data.present_semaphore_handle;
    wait_present.value = 0;
    wait_present.stageMask = wait_stage;
    wait_present.deviceIndex = 0;
  }

  frame::FrameArray<VkSemaphoreSubmitInfo> signal_infos{};
  signal_infos.Reserve(0 + static_cast<ssize>(packet->can_present));

  if (packet->can_present) {
    auto& signal_render{signal_infos.EmplaceBack()};
    signal_render.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
    signal_render.semaphore = context_->GetRenderSemaphoreHandle();
    signal_render.value = 0;
    signal_render.stageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT;
    signal_render.deviceIndex = 0;
  }

  SubmitCommand2(command_data, device_->GetGraphicsQueueHandle(),
                 frame_data.render_fence_handle, wait_infos.GetData(),
                 static_cast<u32>(wait_infos.GetSize()), signal_infos.GetData(),
                 static_cast<u32>(signal_infos.GetSize()), VK_NULL_HANDLE);
}

frame::FrameArray<const schar*> VulkanDriver::GetRequiredExtensions() {
  u32 glfw_extension_count{0};
  const auto** glfw_extensions{
      glfwGetRequiredInstanceExtensions(&glfw_extension_count)};
  frame::FrameArray<const schar*> extensions{};

  if (glfw_extensions == nullptr || glfw_extension_count == 0) {
    const char* desc;
    const auto code{glfwGetError(&desc)};

    COMET_LOG_ERROR(LoggerType::Rendering,
                    "VulkanDriver::GetRequiredExtensions",
                    "failed to get required instance extensions", "glfw_error",
                    code, "description", desc != nullptr ? desc : "(null)");

    COMET_ASSERT(false, "VulkanDriver::GetRequiredExtensions",
                 "required instance extensions unavailable");
  }

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
  const auto create_info{init::GenerateDebugUtilsMessengerCreateInfo(
      VulkanDriver::LogVulkanValidationMessage)};

  COMET_CHECK_VK(debug::CreateDebugUtilsMessengerEXT(
                     instance_handle_, &create_info, VK_NULL_HANDLE,
                     &debug_messenger_handle_),
                 "VulkanDriver::InitializeDebugMessenger",
                 "failed to set up debug messenger");
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
      "VulkanDriver::InitializeDebugReportCallback",
      "failed to set up debug report callback");
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
      COMET_LOG_ERROR(LoggerType::Rendering,
                      "VulkanDriver::AreValidationLayersSupported",
                      "validation layer unavailable", "layer", layer_name);
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
      Copy(message_type_str, "general", 7);
      break;
    case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT:
      Copy(message_type_str, "validation", 10);
      break;
    case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT:
      Copy(message_type_str, "performance", 11);
      break;
    default:
      Copy(message_type_str, kUnknownLabel, kUnknownLabelLen);
      break;
  }

  switch (message_severity) {
    default:
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
      COMET_LOG_DEBUG(LoggerType::Rendering,
                      "VulkanDriver::LogVulkanValidationMessage",
                      "vulkan validation message", "type", message_type_str,
                      "message", callback_data->pMessage);
      break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
      COMET_LOG_INFO(LoggerType::Rendering,
                     "VulkanDriver::LogVulkanValidationMessage",
                     "vulkan validation message", "type", message_type_str,
                     "message", callback_data->pMessage);
      break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
      COMET_LOG_WARNING(LoggerType::Rendering,
                        "VulkanDriver::LogVulkanValidationMessage",
                        "vulkan validation message", "type", message_type_str,
                        "message", callback_data->pMessage);
      break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
      COMET_LOG_ERROR(LoggerType::Rendering,
                      "VulkanDriver::LogVulkanValidationMessage",
                      "vulkan validation message", "type", message_type_str,
                      "message", callback_data->pMessage);

#ifdef COMET_VULKAN_ABORT_ON_ERROR
      COMET_ASSERT(false, "VulkanDriver::LogVulkanValidationMessage",
                   "validation error", "message", callback_data->pMessage);
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
    COMET_LOG_DEBUG(LoggerType::Rendering,
                    "VulkanDriver::LogVulkanDebugReportMessage",
                    "vulkan debug report message", "layer", layer_prefix,
                    "code", message_code, "message", message);
  } else if (message_flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT) {
    COMET_LOG_INFO(LoggerType::Rendering,
                   "VulkanDriver::LogVulkanDebugReportMessage",
                   "vulkan debug report message", "layer", layer_prefix, "code",
                   message_code, "message", message);
  } else if (message_flags & VK_DEBUG_REPORT_WARNING_BIT_EXT) {
    COMET_LOG_WARNING(LoggerType::Rendering,
                      "VulkanDriver::LogVulkanDebugReportMessage",
                      "vulkan debug report message", "layer", layer_prefix,
                      "code", message_code, "message", message);
  } else if (message_flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) {
    COMET_LOG_ERROR(LoggerType::Rendering,
                    "VulkanDriver::LogVulkanDebugReportMessage",
                    "vulkan debug report message", "layer", layer_prefix,
                    "code", message_code, "message", message);

#ifdef COMET_VULKAN_ABORT_ON_ERROR
    COMET_ASSERT(false, "VulkanDriver::LogVulkanDebugReportMessage",
                 "debug report error", "layer", layer_prefix, "code",
                 message_code, "message", message);
#endif  // COMET_VULKAN_ABORT_ON_ERROR
  }

  return VK_FALSE;
}
#endif  // COMET_DEBUG_RENDERING
}  // namespace vk
}  // namespace rendering
}  // namespace comet
