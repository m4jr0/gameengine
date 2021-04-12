// Copyright 2021 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "imgui_opengl_renderer.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#ifdef _WIN32
#include "debug_windows.h"
#endif  // _WIN32

namespace comet {
namespace editor {
namespace ui {
void ImguiOpenGlRenderer::Initialize() {
  rendering::RenderingManager::Initialize();

  if (!IMGUI_CHECKVERSION()) {
    throw std::runtime_error(
        "Something wrong happened while initializing imgui.");
  }

  ImGui::CreateContext();

  ImGuiIO& io = ImGui::GetIO();
  // Setup Platform/Renderer bindings

  ImGui_ImplGlfw_InitForOpenGL(GetWindow()->GetGlfwWindow(), true);
  ImGui_ImplOpenGL3_Init("#version 410");
  // Setup Dear ImGui style
  ImGui::StyleColorsDark();
}

void ImguiOpenGlRenderer::Update(
    double interpolation, game_object::GameObjectManager& game_object_manager) {
  if (glfwWindowShouldClose(window_->GetGlfwWindow()) != 0) {
    core::Engine::GetEngine().Quit();

    return;
  }

  rendering::RenderingManager::Update(interpolation, game_object_manager);

  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  bool show_demo_window = true;
  bool show_another_window = false;

  if (show_demo_window) ImGui::ShowDemoWindow(&show_demo_window);

  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

  window_->Update();
}

void ImguiOpenGlRenderer::Destroy() {
  rendering::RenderingManager::Destroy();
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
}
}  // namespace ui
}  // namespace editor
}  // namespace comet