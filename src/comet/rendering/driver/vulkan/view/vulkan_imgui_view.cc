// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "vulkan_imgui_view.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"

#include "comet/core/engine.h"
#include "comet/physics/physics_manager.h"
#include "comet/rendering/driver/vulkan/utils/vulkan_command_buffer_utils.h"
#include "comet/rendering/driver/vulkan/vulkan_debug.h"
#include "comet/rendering/rendering_manager.h"

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

  // TODO(m4jr0): Make clear values more configurable.
  std::memcpy(&render_pass_descr.clear_values[0].color, clear_color_,
              sizeof(render_pass_descr.clear_values[0].color.float32[0]) * 4);
  render_pass_descr.clear_values[1].depthStencil.depth = 1.0f;

  const auto is_msaa{context_->GetDevice().IsMsaa()};
  render_pass_descr.clear_flags = RenderPassClearFlag::None;
  render_pass_descr.attachment_descrs.reserve(is_msaa ? 2 : 1);

  AttachmentDescr color_attachment_descr{};
  color_attachment_descr.type = AttachmentType::Color;
  color_attachment_descr.load_op =
      is_first_ ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
  color_attachment_descr.store_op = VK_ATTACHMENT_STORE_OP_STORE;
  color_attachment_descr.is_final_layout = !is_msaa && is_last_;
  render_pass_descr.attachment_descrs.push_back(color_attachment_descr);

  if (is_msaa) {
    AttachmentDescr resolve_attachment_descr{};
    resolve_attachment_descr.type = AttachmentType::Resolve;
    resolve_attachment_descr.load_op = VK_ATTACHMENT_LOAD_OP_CLEAR;
    resolve_attachment_descr.store_op = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    resolve_attachment_descr.is_final_layout = is_last_;
    render_pass_descr.attachment_descrs.push_back(resolve_attachment_descr);
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
  pool_info.poolSizeCount = std::size(pool_sizes);
  pool_info.pPoolSizes = pool_sizes;

  COMET_CHECK_VK(
      vkCreateDescriptorPool(context_->GetDevice(), &pool_info, VK_NULL_HANDLE,
                             &descriptor_pool_handle_),
      "Failed to create descriptor pool for ImGui!");

  ImGui::CreateContext();
  ImGui_ImplGlfw_InitForVulkan(window_->GetHandle(), false);
  const auto& device{context_->GetDevice()};

  ImGui_ImplVulkan_InitInfo imgui_info{};
  imgui_info.Instance = context_->GetInstanceHandle();
  imgui_info.PhysicalDevice = device.GetPhysicalDeviceHandle();
  imgui_info.Device = device;
  imgui_info.QueueFamily = 0;
  imgui_info.Queue = device.GetGraphicsQueueHandle();
  imgui_info.PipelineCache = VK_NULL_HANDLE;
  imgui_info.DescriptorPool = descriptor_pool_handle_;
  imgui_info.Subpass = 0;
  imgui_info.MinImageCount = context_->GetImageCount();
  imgui_info.ImageCount = context_->GetImageCount();
  imgui_info.MSAASamples = device.GetMsaaSamples();

  ImGui_ImplVulkan_Init(&imgui_info, render_pass_->handle);

  auto command_pool_handle{context_->GetUploadContext().command_pool_handle};
  auto command_buffer_handle{
      GenerateOneTimeCommand(device, command_pool_handle)};
  ImGui_ImplVulkan_CreateFontsTexture(command_buffer_handle);
  SubmitOneTimeCommand(command_buffer_handle, command_pool_handle, device,
                       device.GetGraphicsQueueHandle());
}

void ImGuiView::Destroy() {
  ImGui_ImplVulkan_DestroyFontUploadObjects();

  if (descriptor_pool_handle_ != VK_NULL_HANDLE) {
    vkDestroyDescriptorPool(context_->GetDevice(), descriptor_pool_handle_,
                            VK_NULL_HANDLE);
    descriptor_pool_handle_ = VK_NULL_HANDLE;
  }

  ImGui_ImplVulkan_Shutdown();
  ImGui::DestroyContext();
  View::Destroy();
}

void ImGuiView::Update(const ViewPacket& packet) {
  ImGui_ImplVulkan_NewFrame();
  ImGui_ImplGlfw_NewFrame();

  ImGui::NewFrame();
  Draw();
  ImGui::Render();

  render_pass_handler_->BeginPass(*render_pass_, packet.command_buffer_handle,
                                  packet.frame_in_flight_index);
  ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(),
                                  packet.command_buffer_handle);
  render_pass_handler_->EndPass(packet.command_buffer_handle);
}

void vk::ImGuiView::Draw() const {
  // TODO(m4jr0): Implement driver agnostic logic somewhere else.
  const auto& rendering_manager{Engine::Get().GetRenderingManager()};
  const auto& physics_manager{Engine::Get().GetPhysicsManager()};

  ImGui::Begin("Mini Profiler");
  ImGui::Text("PHYSICS");
  ImGui::Indent();
  ImGui::Text("Frame Time: %f ms", physics_manager.GetFrameTime());
  ImGui::Text("Framerate: %u Hz", physics_manager.GetFrameRate());
  ImGui::Unindent();
  ImGui::Spacing();
  ImGui::Text("RENDERING");
  ImGui::Indent();
  ImGui::Text("Frame Time: %f ms", rendering_manager.GetFrameTime());
  ImGui::Text("Framerate: %u FPS", rendering_manager.GetFrameRate());
  ImGui::Unindent();
  ImGui::End();
}
}  // namespace vk
}  // namespace rendering
}  // namespace comet