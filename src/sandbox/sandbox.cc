// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "sandbox/sandbox.h"
////////////////////////////////////////////////////////////////////////////////

namespace comet {
namespace sandbox {
void Sandbox::OnInitialize() {
  camera_handler_ = std::make_unique<CameraHandler>();
  COMET_ASSERT(camera_handler_ != nullptr, "Sandbox::OnInitialize",
               "camera handler allocation failed");

  camera_handler_->Initialize();

  LoadSandboxScene();
}

void Sandbox::OnUpdate(f64& lag) {
  COMET_ASSERT(camera_handler_ != nullptr, "Sandbox::OnUpdate",
               "camera handler is null");

  camera_handler_->Update();
  UpdateSandboxScene(lag);
}

void Sandbox::OnShutdown() {
  UnloadSandboxScene();

  if (camera_handler_ != nullptr) {
    camera_handler_->Shutdown();
    camera_handler_ = nullptr;
  }
}

void Sandbox::LoadSandboxScene() {
  // TODO(m4jr0): Move hardcoded sandbox scene loading here.
}

void Sandbox::UpdateSandboxScene([[maybe_unused]] f64& lag) {
  // TODO(m4jr0): Move hardcoded sandbox scene update here.
}

void Sandbox::UnloadSandboxScene() {
  // TODO(m4jr0): Move hardcoded sandbox scene unloading here.
}
}  // namespace sandbox
}  // namespace comet