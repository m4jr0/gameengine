// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "vulkan_imgui_view.h"
////////////////////////////////////////////////////////////////////////////////

#ifdef COMET_IMGUI
// External. ///////////////////////////////////////////////////////////////////
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/c_array.h"
#include "comet/core/frame/frame_container.h"
#include "comet/core/memory/memory_utils.h"
#include "comet/profiler/profiler.h"
#include "comet/rendering/debug_ui_registry.h"
#include "comet/rendering/driver/vulkan/utils/vulkan_view_utils.h"
#include "comet/rendering/driver/vulkan/vulkan_debug.h"

namespace comet {
namespace rendering {
namespace vk {
ImGuiView::ImGuiView(const ImGuiViewDescr& descr)
    : View{descr}, window_{descr.window} {
  COMET_ASSERT(window_ != nullptr, "ImGuiView::ImGuiView", "window is null");
}

void ImGuiView::Prepare(const ViewUpdate&) {
  COMET_PROFILE("ImGuiView::Prepare");

  ImGui_ImplVulkan_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  DrawDebugUi();

  ImGui::Render();
}

void ImGuiView::Begin(const ViewUpdate&) {
  COMET_PROFILE("ImGuiView::Begin");

  VkClearValue clear_values[2]{};
  memory::CopyMemory(&clear_values[0].color, clear_color_,
                     sizeof(clear_values[0].color.float32[0]) * 4);
  clear_values[1].depthStencil.depth = 1.0f;
  clear_values[1].depthStencil.stencil = 0;

  const auto current_frame{context_->GetFrameInFlightIndex()};
  auto& frame_data{context_->GetFrameData(current_frame)};
  const auto command_buffer_handle{frame_data.command_buffer_handle};

  render_pass_handler_->BeginPass(render_pass_handle_, command_buffer_handle,
                                  context_->GetImageIndex(), clear_values, 2);

  is_pass_open_ = true;
}

void ImGuiView::Draw(const ViewUpdate&) {
  COMET_PROFILE("ImGuiView::Draw");

  if (!is_pass_open_) {
    return;
  }

  const auto current_frame{context_->GetFrameInFlightIndex()};
  auto& frame_data{context_->GetFrameData(current_frame)};

  ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(),
                                  frame_data.command_buffer_handle);
}

void ImGuiView::End(const ViewUpdate&) {
  COMET_PROFILE("ImGuiView::End");

  if (!is_pass_open_) {
    return;
  }

  const auto current_frame{context_->GetFrameInFlightIndex()};
  auto& frame_data{context_->GetFrameData(current_frame)};

  render_pass_handler_->EndPass(frame_data.command_buffer_handle);
  is_pass_open_ = false;
}

void ImGuiView::OnInitialize() {
  RenderPassDescr render_pass_descr{};

  VkExtent2D extent{};
  extent.width = width_;
  extent.height = height_;
  render_pass_descr.extent = extent;
  render_pass_descr.offset = {0, 0};

  render_pass_descr.dependencies = frame::FrameArray<VkSubpassDependency>{};
  render_pass_descr.dependencies.Reserve(1);

  auto& dependency{render_pass_descr.dependencies.EmplaceLast()};
  dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
  dependency.dstSubpass = 0;
  dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
                             VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

  auto& device{context_->GetDevice()};
  const auto is_msaa{device.IsMsaa()};

  render_pass_descr.clear_flags = GenerateClearFlags(pass_descr_);
  render_pass_descr.attachment_descrs = frame::FrameArray<AttachmentDescr>{};
  render_pass_descr.attachment_descrs.Reserve(is_msaa ? 2 : 1);

  GenerateAttachmentDescrs(
      pass_descr_, is_msaa ? device.GetMsaaSamples() : VK_SAMPLE_COUNT_1_BIT,
      render_pass_descr.attachment_descrs);

  render_pass_descr.options = kRenderPassOptionFlagBitsSwapchainTarget;

  if (is_msaa) {
    render_pass_descr.options |= kRenderPassOptionFlagBitsMultisampled;
  }

  render_pass_handle_ = render_pass_handler_->GetOrGenerate(render_pass_descr);

  VkDescriptorPoolSize pool_sizes[] = {
      {VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
      {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
      {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
      {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
      {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
      {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
      {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
      {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
      {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
      {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
      {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000}};

  VkDescriptorPoolCreateInfo pool_info{};
  pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
  pool_info.maxSets = 1000;
  pool_info.poolSizeCount = static_cast<u32>(GetLength(pool_sizes));
  pool_info.pPoolSizes = pool_sizes;

  COMET_CHECK_VK(vkCreateDescriptorPool(device, &pool_info, VK_NULL_HANDLE,
                                        &descriptor_pool_handle_),
                 "ImGuiView::OnInitialize",
                 "imgui descriptor pool creation failed");

#ifdef COMET_DEBUG
  IMGUI_CHECKVERSION();
#endif  // COMET_DEBUG

  ImGui::CreateContext();
  ImGui_ImplGlfw_InitForVulkan(window_->GetHandle(), false);

  const auto image_count{context_->GetImageCount()};

  ImGui_ImplVulkan_InitInfo imgui_info{};
  imgui_info.Instance = context_->GetInstanceHandle();
  imgui_info.PhysicalDevice = device.GetPhysicalDeviceHandle();
  imgui_info.Device = device;

  auto& indices{device.GetQueueFamilyIndices()};
  imgui_info.QueueFamily = indices.graphics_family.value_or(0);
  imgui_info.Queue = device.GetGraphicsQueueContext().handle;
  imgui_info.PipelineCache = VK_NULL_HANDLE;
  imgui_info.DescriptorPool = descriptor_pool_handle_;
  imgui_info.PipelineInfoMain.RenderPass =
      render_pass_handler_->GetVkHandle(render_pass_handle_);
  imgui_info.PipelineInfoMain.Subpass = 0;
  imgui_info.MinImageCount = image_count;
  imgui_info.ImageCount = image_count;
  imgui_info.PipelineInfoMain.MSAASamples = device.GetMsaaSamples();

  ImGui_ImplVulkan_Init(&imgui_info);
  ImGui::StyleColorsDark();
}

void ImGuiView::OnDestroy() {
  ImGui_ImplGlfw_Shutdown();
  ImGui_ImplVulkan_Shutdown();

  if (descriptor_pool_handle_ != VK_NULL_HANDLE) {
    vkDestroyDescriptorPool(context_->GetDevice(), descriptor_pool_handle_,
                            VK_NULL_HANDLE);
    descriptor_pool_handle_ = VK_NULL_HANDLE;
  }

  ImGui::DestroyContext();
  window_ = nullptr;
  is_pass_open_ = false;
}

void ImGuiView::DrawDebugUi() const {
#ifdef COMET_HAS_DEBUG_UI
  DebugUiRegistry::Get().Draw();
#endif  // COMET_HAS_DEBUG_UI
}
}  // namespace vk
}  // namespace rendering
}  // namespace comet
#endif  // COMET_IMGUI