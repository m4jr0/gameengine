// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "opengl_view_handler.h"
////////////////////////////////////////////////////////////////////////////////

// External. ///////////////////////////////////////////////////////////////////
#include <utility>
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/memory/memory_utils.h"
#include "comet/core/type_trait.h"
#include "comet/profiler/profiler.h"
#include "comet/rendering/driver/opengl/view/opengl_shadow_view.h"
#include "comet/rendering/driver/opengl/view/opengl_world_view.h"
#include "comet/rendering/label/view_label.h"

#ifdef COMET_IMGUI
#include "comet/rendering/driver/opengl/view/opengl_imgui_view.h"
#endif  // COMET_IMGUI

#ifdef COMET_DEBUG
#include "comet/rendering/driver/opengl/view/opengl_debug_view.h"
#endif  // COMET_DEBUG

namespace comet {
namespace rendering {
namespace gl {
ViewHandler::ViewHandler(const ViewHandlerDescr& descr)
    : Handler{descr},
      shadow_settings_{descr.shadow_settings},
      shader_handler_{descr.shader_handler},
      material_handler_{descr.material_handler},
      render_proxy_handler_{descr.render_proxy_handler},
      mesh_handler_{descr.mesh_handler},
      lighting_handler_{descr.lighting_handler},
      window_{descr.window},
      rendering_view_descrs_{descr.rendering_view_descrs} {
  COMET_ASSERT(shader_handler_ != nullptr, "ViewHandler::ViewHandler",
               "shader handler is null");
  COMET_ASSERT(material_handler_ != nullptr, "ViewHandler::ViewHandler",
               "material handler is null");
  COMET_ASSERT(render_proxy_handler_ != nullptr, "ViewHandler::ViewHandler",
               "render proxy handler is null");
  COMET_ASSERT(mesh_handler_ != nullptr, "ViewHandler::ViewHandler",
               "mesh handler is null");
  COMET_ASSERT(lighting_handler_ != nullptr, "ViewHandler::ViewHandler",
               "lighting handler is null");
  COMET_ASSERT(window_ != nullptr, "ViewHandler::ViewHandler",
               "window is null");
  COMET_ASSERT(rendering_view_descrs_ != nullptr, "ViewHandler::ViewHandler",
               "rendering view descriptions are null");
}

void ViewHandler::Update(frame::FramePacket* packet) {
  COMET_PROFILE("ViewHandler::Update");
  COMET_ASSERT(packet != nullptr, "ViewHandler::Update",
               "frame packet is null");

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

      view_descr.frame_state = frame_state_;
      view_descr.shadow_settings = shadow_settings_;
      view_descr.shader_handler = shader_handler_;
      view_descr.material_handler = material_handler_;
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

      view_descr.frame_state = frame_state_;
      view_descr.shader_handler = shader_handler_;
      view_descr.render_proxy_handler = render_proxy_handler_;
      view_descr.mesh_handler = mesh_handler_;
      view_descr.lighting_handler = lighting_handler_;

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

      view_descr.frame_state = frame_state_;
      view_descr.shader_handler = shader_handler_;
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

      view_descr.frame_state = frame_state_;
      view_descr.window = window_;

      view = std::make_unique<ImGuiView>(view_descr);
      break;
    }
#endif  // COMET_IMGUI

    default: {
      COMET_ASSERT(false, "ViewHandler::Generate", "view type is unsupported",
                   "view_type", GetRenderingViewTypeLabel(descr.type),
                   "view_type_value", ToUnderlying(descr.type));
      return nullptr;
    }
  }

  COMET_ASSERT(view != nullptr, "ViewHandler::Generate",
               "generated view is null", "view_type",
               GetRenderingViewTypeLabel(descr.type), "view_type_value",
               ToUnderlying(descr.type), "view_id", descr.id);

  view->Initialize();
  views_.PushLast(std::move(view));
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
  COMET_ASSERT(view != nullptr, "ViewHandler::Get", "view not found", "index",
               index);
  return view;
}

const View* ViewHandler::TryGet(usize index) const {
  if (index >= views_.GetSize()) {
    return nullptr;
  }

  return views_[index].get();
}

void ViewHandler::OnInitialize() {
  views_ = Array<memory::UniquePtr<View>>::WithCapacity(
      &allocator_, rendering_view_descrs_->GetSize());

  for (const auto& view_descr : *rendering_view_descrs_) {
    Generate(view_descr);
  }
}

void ViewHandler::OnShutdown() {
  for (auto& view : views_) {
    Destroy(view.get(), true);
  }

  frame_state_ = nullptr;
  views_.Release();
}

View* ViewHandler::Get(usize index) {
  auto* view{TryGet(index)};
  COMET_ASSERT(view != nullptr, "ViewHandler::Get", "view not found", "index",
               index);
  return view;
}

View* ViewHandler::TryGet(usize index) {
  if (index >= views_.GetSize()) {
    return nullptr;
  }

  return views_[index].get();
}

void ViewHandler::Destroy(View* view, bool is_destroying_handler) {
  COMET_ASSERT(view != nullptr, "ViewHandler::Destroy", "view is null");

  if (is_destroying_handler) {
    view->Destroy();
    return;
  }

  const auto view_id{view->GetId()};
  auto view_index{kInvalidIndex};

  for (usize i{0}; i < views_.GetSize(); ++i) {
    auto* other_view{views_[i].get()};

    if (other_view->GetId() == view_id) {
      view_index = i;
      break;
    }
  }

  COMET_ASSERT(view_index != kInvalidIndex, "ViewHandler::Destroy",
               "view was not found in handler", "view_id", view_id);

  view->Destroy();
  views_.RemoveFromPos(views_.begin() + view_index);
}
}  // namespace gl
}  // namespace rendering
}  // namespace comet