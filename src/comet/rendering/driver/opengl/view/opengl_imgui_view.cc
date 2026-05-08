// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "opengl_imgui_view.h"
////////////////////////////////////////////////////////////////////////////////

#ifdef COMET_IMGUI
// External. ///////////////////////////////////////////////////////////////////
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/profiler/profiler.h"
#include "comet/rendering/debug_ui_registry.h"

namespace comet {
namespace rendering {
namespace gl {
ImGuiView::ImGuiView(const ImGuiViewDescr& descr)
    : View{descr}, window_{descr.window} {
  COMET_ASSERT(window_ != nullptr, "ImGuiView::ImGuiView", "window is null");
}

void ImGuiView::Prepare(const ViewUpdate&) {
  COMET_PROFILE("ImGuiView::Prepare");

  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  DrawDebugUi();

  ImGui::Render();
}

void ImGuiView::Begin(const ViewUpdate&) { COMET_PROFILE("ImGuiView::Begin"); }

void ImGuiView::Draw(const ViewUpdate&) {
  COMET_PROFILE("ImGuiView::Draw");

  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void ImGuiView::End(const ViewUpdate&) { COMET_PROFILE("ImGuiView::End"); }

void ImGuiView::OnInitialize() {
#ifdef COMET_DEBUG
  IMGUI_CHECKVERSION();
#endif  // COMET_DEBUG

  ImGui::CreateContext();
  ImGui_ImplGlfw_InitForOpenGL(window_->GetHandle(), false);
  ImGui_ImplOpenGL3_Init();
  ImGui::StyleColorsDark();
}

void ImGuiView::OnDestroy() {
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  window_ = nullptr;
}

void ImGuiView::DrawDebugUi() const {
#ifdef COMET_HAS_DEBUG_UI
  DebugUiRegistry::Get().Draw();
#endif  // COMET_HAS_DEBUG_UI
}
}  // namespace gl
}  // namespace rendering
}  // namespace comet
#endif  // COMET_IMGUI