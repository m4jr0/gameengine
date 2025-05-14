// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"

#include "vulkan_imgui_view.h"

#ifdef COMET_IMGUI
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"

#include "comet/core/c_array.h"
#include "comet/core/frame/frame_packet.h"
#include "comet/core/frame/frame_utils.h"
#include "comet/core/memory/memory_utils.h"
#include "comet/core/type/array.h"
#include "comet/profiler/profiler.h"
#include "comet/rendering/driver/vulkan/vulkan_debug.h"

#ifdef COMET_DEBUG
#include "comet/rendering/debugger/debugger_displayer_manager.h"
#endif  // COMET_DEBUG

namespace comet {
namespace rendering {
namespace vk {
ImGuiView::ImGuiView(const ImGuiViewDescr& descr)
    : View{descr}, window_{descr.window} {
  COMET_ASSERT(window_ != nullptr, "Window is null!");
}

void ImGuiView::Initialize() {
  View::Initialize();
  RenderPassDescr render_pass_descr{};
  render_pass_descr.id = id_;

  VkExtent2D extent{};
  extent.width = width_;
  extent.height = height_;
  render_pass_descr.extent = extent;
  render_pass_descr.offset.x = 0;
  render_pass_descr.offset.y = 0;

  render_pass_descr.dependencies = frame::FrameArray<VkSubpassDependency>{};
  render_pass_descr.dependencies.Reserve(1);

  auto& dependency{render_pass_descr.dependencies.EmplaceBack()};
  dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
  dependency.dstSubpass = 0;
  dependency.srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
  dependency.srcAccessMask = 0;
  dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                            VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
  dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
                             VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
  dependency.dependencyFlags = 0;

  // TODO(m4jr0): Make clear values more configurable.
  memory::CopyMemory(
      &render_pass_descr.clear_values[0].color, clear_color_,
      sizeof(render_pass_descr.clear_values[0].color.float32[0]) * 4);
  render_pass_descr.clear_values[1].depthStencil.depth = 1.0f;

  auto& device{context_->GetDevice()};
  const auto is_msaa{device.IsMsaa()};
  render_pass_descr.clear_flags = RenderPassClearFlag::None;
  render_pass_descr.attachment_descrs = frame::FrameArray<AttachmentDescr>{};
  render_pass_descr.attachment_descrs.Reserve(is_msaa ? 2 : 1);

  AttachmentDescr color_attachment_descr{};
  color_attachment_descr.type = AttachmentType::Color;
  color_attachment_descr.load_op =
      is_first_ ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
  color_attachment_descr.store_op = VK_ATTACHMENT_STORE_OP_STORE;
  color_attachment_descr.is_final_layout = !is_msaa && is_last_;
  render_pass_descr.attachment_descrs.PushBack(color_attachment_descr);

  if (is_msaa) {
    AttachmentDescr resolve_attachment_descr{};
    resolve_attachment_descr.type = AttachmentType::Resolve;
    resolve_attachment_descr.load_op = VK_ATTACHMENT_LOAD_OP_CLEAR;
    resolve_attachment_descr.store_op = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    resolve_attachment_descr.is_final_layout = is_last_;
    render_pass_descr.attachment_descrs.PushBack(resolve_attachment_descr);
  }

  render_pass_ = render_pass_handler_->Generate(render_pass_descr);

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

  VkDescriptorPoolCreateInfo pool_info = {};
  pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
  pool_info.maxSets = 1000;
  pool_info.poolSizeCount = static_cast<u32>(GetLength(pool_sizes));
  pool_info.pPoolSizes = pool_sizes;

  COMET_CHECK_VK(vkCreateDescriptorPool(device, &pool_info, VK_NULL_HANDLE,
                                        &descriptor_pool_handle_),
                 "Failed to create descriptor pool for ImGui!");

#ifdef COMET_DEBUG
  IMGUI_CHECKVERSION();
#endif  // COMET_DEBUG
  ImGui::CreateContext();
  ImGui_ImplGlfw_InitForVulkan(window_->GetHandle(), false);
  auto image_count{context_->GetImageCount()};

  ImGui_ImplVulkan_InitInfo imgui_info{};
  imgui_info.Instance = context_->GetInstanceHandle();
  imgui_info.PhysicalDevice = device.GetPhysicalDeviceHandle();
  imgui_info.Device = device;
  imgui_info.QueueFamily = 0;
  imgui_info.Queue = device.GetGraphicsQueueHandle();
  imgui_info.PipelineCache = VK_NULL_HANDLE;
  imgui_info.DescriptorPool = descriptor_pool_handle_;
  imgui_info.RenderPass = render_pass_->handle;
  imgui_info.Subpass = 0;
  imgui_info.MinImageCount = image_count;
  imgui_info.ImageCount = image_count;
  imgui_info.MSAASamples = device.GetMsaaSamples();

  ImGui_ImplVulkan_Init(&imgui_info);
  ImGui::StyleColorsDark();
  ImGui_ImplVulkan_CreateFontsTexture();
}

void ImGuiView::Destroy() {
  ImGui_ImplGlfw_Shutdown();
  ImGui_ImplVulkan_DestroyFontsTexture();

  if (descriptor_pool_handle_ != VK_NULL_HANDLE) {
    vkDestroyDescriptorPool(context_->GetDevice(), descriptor_pool_handle_,
                            VK_NULL_HANDLE);
    descriptor_pool_handle_ = VK_NULL_HANDLE;
  }

  ImGui_ImplVulkan_Shutdown();
  ImGui::DestroyContext();
  View::Destroy();
}

void ImGuiView::Update(frame::FramePacket*) {
  COMET_PROFILE("ImGuiView::Update");
  ImGui_ImplVulkan_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
  Draw();
  ImGui::Render();

  auto command_buffer_handle{context_->GetFrameData().command_buffer_handle};
  render_pass_handler_->BeginPass(render_pass_, command_buffer_handle,
                                  context_->GetImageIndex());
  ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), command_buffer_handle);
  render_pass_handler_->EndPass(command_buffer_handle);
}

void vk::ImGuiView::Draw() const {
#ifdef COMET_PROFILING
  DebuggerDisplayerManager::Get().Draw();
#endif  // COMET_PROFILING
}
}  // namespace vk
}  // namespace rendering
}  // namespace comet
#endif  // COMET_IMGUI