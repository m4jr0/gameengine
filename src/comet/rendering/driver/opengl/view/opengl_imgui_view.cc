// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"

#include "opengl_imgui_view.h"

#ifdef COMET_IMGUI
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "comet/profiler/profiler.h"

#ifdef COMET_DEBUG
#include "comet/rendering/debugger/debugger_displayer_manager.h"
#endif  // COMET_DEBUG

namespace comet {
namespace rendering {
namespace gl {
ImGuiView::ImGuiView(const ImGuiViewDescr& descr)
    : View{descr}, window_{descr.window} {
  COMET_ASSERT(window_ != nullptr, "Window is null!");
}

void ImGuiView::Initialize() {
  View::Initialize();

#ifdef COMET_DEBUG
  IMGUI_CHECKVERSION();
#endif  // COMET_DEBUG
  ImGui::CreateContext();
  ImGui_ImplGlfw_InitForOpenGL(window_->GetHandle(), false);
  ImGui_ImplOpenGL3_Init();
  ImGui::StyleColorsDark();
}

void ImGuiView::Destroy() {
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
  View::Destroy();
}

void ImGuiView::Update(frame::FramePacket* packet) {
  COMET_PROFILE("ImGuiView::Update");

  if (packet->is_rendering_skipped) {
    return;
  }

  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  Draw();

  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void gl::ImGuiView::Draw() const {
#ifdef COMET_PROFILING
  DebuggerDisplayerManager::Get().Draw();
#endif  // COMET_PROFILING
}
}  // namespace gl
}  // namespace rendering
}  // namespace comet
#endif  // COMET_IMGUI