// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "vulkan_view_handler.h"
////////////////////////////////////////////////////////////////////////////////

// External. ///////////////////////////////////////////////////////////////////
#include <type_traits>
#include <utility>
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/memory/memory_utils.h"
#include "comet/profiler/profiler.h"
#include "comet/rendering/driver/vulkan/view/vulkan_shadow_view.h"
#include "comet/rendering/driver/vulkan/view/vulkan_world_view.h"

#ifdef COMET_IMGUI
#include "comet/rendering/driver/vulkan/view/vulkan_imgui_view.h"
#endif  // COMET_IMGUI
#ifdef COMET_DEBUG
#include "comet/rendering/driver/vulkan/view/vulkan_debug_view.h"
#endif  // COMET_DEBUG

namespace comet {
namespace rendering {
namespace vk {
ViewHandler::ViewHandler(const ViewHandlerDescr& descr)
    : Handler{descr},
      shadow_settings_{descr.shadow_settings},
      shader_handler_{descr.shader_handler},
      material_handler_{descr.material_handler},
      texture_handler_{descr.texture_handler},
      pipeline_handler_{descr.pipeline_handler},
      render_pass_handler_{descr.render_pass_handler},
      render_proxy_handler_{descr.render_proxy_handler},
      mesh_handler_{descr.mesh_handler},
      lighting_handler_{descr.lighting_handler},
      window_{descr.window},
      rendering_view_descrs_{descr.rendering_view_descrs} {
  COMET_ASSERT(shader_handler_ != nullptr,
               "Shader handler cannot be null for view handler!");
  COMET_ASSERT(material_handler_ != nullptr,
               "Material handler cannot be null for view handler!");
  COMET_ASSERT(texture_handler_ != nullptr,
               "Texture handler cannot be null for view handler!");
  COMET_ASSERT(pipeline_handler_ != nullptr,
               "Pipeline handler cannot be null for view handler!");
  COMET_ASSERT(render_pass_handler_ != nullptr,
               "Render pass handler cannot be null for view handler!");
  COMET_ASSERT(render_proxy_handler_ != nullptr,
               "Render proxy handler cannot be null for view handler!");
  COMET_ASSERT(mesh_handler_ != nullptr,
               "Mesh handler cannot be null for view handler!");
  COMET_ASSERT(lighting_handler_ != nullptr,
               "Lighting handler cannot be null for view handler!");
  COMET_ASSERT(window_ != nullptr, "Window cannot be null for view handler!");
  COMET_ASSERT(rendering_view_descrs_ != nullptr,
               "Render view descriptions cannot be null for view handler!");
}

void ViewHandler::Update(frame::FramePacket* packet) {
  COMET_PROFILE("ViewHandler::Update");

  for (const auto& view : views_) {
    view->Update(packet);
  }
}

const View* ViewHandler::Generate(const RenderingViewDescr& descr) {
  memory::UniquePtr<View> view{nullptr};

  switch (descr.type) {
    case RenderingViewType::World: {
      WorldViewDescr view_descr{};
      view_descr.id = descr.id;

      view_descr.pass_descr.flags = kViewPassFlagBitsSwapchainTarget |
                                    kViewPassFlagBitsHasColor |
                                    kViewPassFlagBitsHasDepth;

      view_descr.pass_descr.color_load_op = ViewLoadOp::Clear;
      view_descr.pass_descr.color_store_op = ViewStoreOp::Store;
      view_descr.pass_descr.depth_load_op = ViewLoadOp::Clear;
      view_descr.pass_descr.depth_store_op = ViewStoreOp::DontCare;
      view_descr.pass_descr.final_color_op = ViewFinalColorOp::Keep;

      view_descr.width = descr.width;
      view_descr.height = descr.height;
      memory::CopyMemory(view_descr.clear_color, descr.clear_color,
                         sizeof(descr.clear_color[0]) * 4);
      view_descr.context = context_;
      view_descr.shadow_settings = shadow_settings_;
      view_descr.shader_handler = shader_handler_;
      view_descr.texture_handler = texture_handler_;
      view_descr.material_handler = material_handler_;
      view_descr.pipeline_handler = pipeline_handler_;
      view_descr.render_pass_handler = render_pass_handler_;
      view_descr.render_proxy_handler = render_proxy_handler_;
      view_descr.mesh_handler = mesh_handler_;
      view_descr.lighting_handler = lighting_handler_;
      view = std::make_unique<WorldView>(view_descr);
      break;
    }

    case RenderingViewType::Shadow: {
      ShadowViewDescr view_descr{};
      view_descr.id = descr.id;

      view_descr.pass_descr.flags =
          kViewPassFlagBitsOffscreenTarget | kViewPassFlagBitsHasDepth;

      view_descr.pass_descr.depth_load_op = ViewLoadOp::Clear;
      view_descr.pass_descr.depth_store_op = ViewStoreOp::Store;

      view_descr.width = descr.width;
      view_descr.height = descr.height;
      memory::CopyMemory(view_descr.clear_color, descr.clear_color,
                         sizeof(descr.clear_color[0]) * 4);
      view_descr.context = context_;
      view_descr.shader_handler = shader_handler_;
      view_descr.pipeline_handler = pipeline_handler_;
      view_descr.render_pass_handler = render_pass_handler_;
      view_descr.render_proxy_handler = render_proxy_handler_;
      view_descr.lighting_handler = lighting_handler_;
      view_descr.mesh_handler = mesh_handler_;
      view = std::make_unique<ShadowView>(view_descr);
      break;
    }

#ifdef COMET_DEBUG
    case RenderingViewType::Debug: {
      DebugViewDescr view_descr{};
      view_descr.id = descr.id;

      view_descr.pass_descr.flags = kViewPassFlagBitsSwapchainTarget |
                                    kViewPassFlagBitsHasColor |
                                    kViewPassFlagBitsHasDepth;

      view_descr.pass_descr.color_load_op = ViewLoadOp::Load;
      view_descr.pass_descr.color_store_op = ViewStoreOp::Store;
      view_descr.pass_descr.depth_load_op = ViewLoadOp::Load;
      view_descr.pass_descr.depth_store_op = ViewStoreOp::DontCare;
      view_descr.pass_descr.final_color_op = ViewFinalColorOp::Keep;

      view_descr.width = descr.width;
      view_descr.height = descr.height;
      memory::CopyMemory(view_descr.clear_color, descr.clear_color,
                         sizeof(descr.clear_color[0]) * 4);
      view_descr.context = context_;
      view_descr.shader_handler = shader_handler_;
      view_descr.pipeline_handler = pipeline_handler_;
      view_descr.render_pass_handler = render_pass_handler_;
      view_descr.render_proxy_handler = render_proxy_handler_;
      view = std::make_unique<DebugView>(view_descr);
      break;
    }
#endif  // COMET_DEBUG

#ifdef COMET_IMGUI
    case RenderingViewType::ImGui: {
      ImGuiViewDescr view_descr{};
      view_descr.id = descr.id;

      view_descr.pass_descr.flags =
          kViewPassFlagBitsSwapchainTarget | kViewPassFlagBitsHasColor;

      view_descr.pass_descr.color_load_op = ViewLoadOp::Load;
      view_descr.pass_descr.color_store_op = ViewStoreOp::Store;
      view_descr.pass_descr.final_color_op = ViewFinalColorOp::Present;

      view_descr.width = descr.width;
      view_descr.height = descr.height;
      memory::CopyMemory(view_descr.clear_color, descr.clear_color,
                         sizeof(descr.clear_color[0]) * 4);
      view_descr.context = context_;
      view_descr.render_pass_handler = render_pass_handler_;
      view_descr.window = window_;
      view = std::make_unique<ImGuiView>(view_descr);
      break;
    }
#endif  // COMET_IMGUI

    default: {
      COMET_ASSERT(
          false, "Unknown view type: ",
          static_cast<std::underlying_type_t<RenderingViewType>>(descr.type),
          "!");
      return nullptr;
    }
  }

  COMET_ASSERT(view != nullptr, "Generated view is null! What happened?");

  view->Initialize();
  views_.PushBack(std::move(view));
  return views_.GetLast().get();
}

void ViewHandler::Destroy(usize index) { Destroy(Get(index)); }

void ViewHandler::Destroy(View* view) { Destroy(view, false); }

void ViewHandler::SetSize(WindowSize width, WindowSize height) {
  for (auto& view : views_) {
    if (!view->IsSwapchainTarget()) {
      continue;
    }

    view->SetSize(width, height);
  }
}

const View* ViewHandler::Get(usize index) const {
  auto* view{TryGet(index)};
  COMET_ASSERT(view != nullptr,
               "Requested view with index does not exist: ", index, "!");
  return view;
}

const View* ViewHandler::TryGet(usize index) const {
  COMET_ASSERT(index < views_.GetSize(), "Requested view at index #", index,
               ", but view count is ", views_.GetSize(), "!");
  return views_[index].get();
}

void ViewHandler::OnInitialize() {
  views_ = Array<memory::UniquePtr<View>>{&allocator_};

  for (const auto& view_descr : *rendering_view_descrs_) {
    Generate(view_descr);
  }
}

void ViewHandler::OnShutdown() {
  for (auto& view : views_) {
    Destroy(view.get(), true);
  }

  views_.Destroy();
}

View* ViewHandler::Get(usize index) {
  auto* view{TryGet(index)};
  COMET_ASSERT(view != nullptr,
               "Requested view with index does not exist: ", index, "!");
  return view;
}

View* ViewHandler::TryGet(usize index) {
  COMET_ASSERT(index < views_.GetSize(), "Requested view at index #", index,
               ", but view count is ", views_.GetSize(), "!");
  return views_[index].get();
}

void ViewHandler::Destroy(View* view, bool is_destroying_handler) {
  if (is_destroying_handler) {
    view->Destroy();
    return;
  }

  const auto view_id{view->GetId()};
  auto view_index{kInvalidIndex};

  for (u32 i{0}; i < views_.GetSize(); ++i) {
    auto* other_view{views_[i].get()};

    if (other_view->GetId() == view_id) {
      view_index = i;
      break;
    }
  }

  COMET_ASSERT(view_index != kInvalidIndex,
               "Tried to destroy view, but it was not found in the list!");

  view->Destroy();
  views_.RemoveFromPos(views_.begin() + view_index);
}
}  // namespace vk
}  // namespace rendering
}  // namespace comet