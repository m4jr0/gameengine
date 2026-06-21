// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_engine_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/engine/debug/ui/render/render_debug_ui.h"
////////////////////////////////////////////////////////////////////////////////

#ifdef COMET_HAS_DEBUG_UI

// External. ///////////////////////////////////////////////////////////////////
#include "imgui.h"
////////////////////////////////////////////////////////////////////////////////

namespace comet {
namespace debug {
void RenderDebugUi::Draw(RenderingDebugSettings& settings) {
  ImGui::Begin("Camera Debug");
  ImGui::Indent();

  // 🔘 Debug camera master switch
  bool enabled{settings.IsDebugCameraEnabled()};
  if (ImGui::Checkbox("Enable Debug Camera", &enabled)) {
    settings.SetDebugCameraEnabled(enabled);
  }

  ImGui::Separator();

  auto& camera_manager{camera::CameraManager::Get()};

  const auto main_camera{camera_manager.GetMainCamera()};

#ifdef COMET_DEBUG
  const auto debug_camera{camera_manager.GetDebugCamera()};
  const auto use_debug{settings.IsDebugCameraEnabled()};
#endif

  u32 index{0};

  auto draw_camera = [&](auto handle, auto is_debug_camera) {
    if (!handle || !camera_manager.IsAlive(handle)) {
      return;
    }

    const auto* label{camera_manager.GetDebugLabel(handle)};

    if (label == nullptr) {
      label = "Camera";
    }

    ImGui::PushID(static_cast<s32>(index));

    if (ImGui::TreeNode("CameraNode", "%s #%u", label, index)) {
      DrawCameraInfo(camera_manager, handle);

      ImGui::Separator();

      DrawWorldFlags("Rendering", settings, is_debug_camera);

#ifdef COMET_DEBUG_RENDERING
      DrawDebugDrawFlags("Debug Draw", settings, is_debug_camera);
#endif  // COMET_DEBUG_RENDERING

      ImGui::TreePop();
    }

    ImGui::PopID();
    ++index;
  };

  // Game camera
  draw_camera(main_camera, false);

#ifdef COMET_DEBUG
  if (use_debug) {
    draw_camera(debug_camera, true);
  }
#endif

  ImGui::Unindent();
  ImGui::End();
}

void RenderDebugUi::DrawWorldFlags(const char* label,
                                      RenderingDebugSettings& settings,
                                      bool is_debug_camera) {
  if (!ImGui::TreeNode(label)) {
    return;
  }

  auto drawFlag = [&](const char* name, u32 flag) {
    bool enabled = settings.HasFlag(is_debug_camera, flag);

    if (ImGui::Checkbox(name, &enabled)) {
      settings.SetFlag(is_debug_camera, flag, enabled);
    }
  };

  drawFlag("Disable Textures", kWorldDebugFlagBitsDisableTextures);
  drawFlag("Disable Lighting", kWorldDebugFlagBitsDisableLighting);
  drawFlag("Disable Shadows", kWorldDebugFlagBitsDisableShadows);
  drawFlag("Show Normals", kWorldDebugFlagBitsShowNormals);

  ImGui::TreePop();
}

void RenderDebugUi::DrawDebugDrawFlags(const char* label,
                                          RenderingDebugSettings& settings,
                                          bool is_debug_camera) {
  if (!ImGui::TreeNode(label)) {
    return;
  }

  auto drawFlag = [&](const char* name, DebugDrawFlags flag) {
    bool enabled = settings.HasDebugDrawFlag(is_debug_camera, flag);

    if (ImGui::Checkbox(name, &enabled)) {
      settings.SetDebugDrawFlag(is_debug_camera, flag, enabled);
    }
  };

  drawFlag("Culling Boxes", kDebugDrawFlagBitsCullingBoxes);
  drawFlag("Camera Frustums", kDebugDrawFlagBitsCameraFrustums);
  drawFlag("Cascade Frustums", kDebugDrawFlagBitsCascadeFrustums);
  drawFlag("Light Frustums", kDebugDrawFlagBitsLightFrustums);

  ImGui::TreePop();
}

void RenderDebugUi::DrawCameraInfo(camera::CameraManager& camera_manager,
                                      camera::CameraHandle handle) {
  const auto& position{camera_manager.GetPosition(handle)};
  const auto& front{camera_manager.GetFront(handle)};
  const auto& up{camera_manager.GetUp(handle)};
  const auto& right{camera_manager.GetRight(handle)};
  const auto size{camera_manager.GetSize(handle)};

  ImGui::Text("Position: %.3f, %.3f, %.3f", position.x, position.y, position.z);
  ImGui::Text("Front:    %.3f, %.3f, %.3f", front.x, front.y, front.z);
  ImGui::Text("Up:       %.3f, %.3f, %.3f", up.x, up.y, up.z);
  ImGui::Text("Right:    %.3f, %.3f, %.3f", right.x, right.y, right.z);

  ImGui::Separator();

  ImGui::Text("Viewport: %u x %u", size.width, size.height);
  ImGui::Text("Aspect:   %.3f", camera_manager.GetAspectRatio(handle));
  ImGui::Text("FOV:      %.3f deg", camera_manager.GetFov(handle));
  ImGui::Text("Near/Far: %.3f / %.3f", camera_manager.GetNearPlane(handle),
              camera_manager.GetFarPlane(handle));
}
}  // namespace debug
}  // namespace comet

#endif  // COMET_HAS_DEBUG_UI