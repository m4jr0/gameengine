// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "camera_manager.h"

namespace comet {
namespace rendering {
void CameraManager::Initialize() {
  Manager::Initialize();
  GenerateMainCamera();
}

void CameraManager::Shutdown() {
  main_camera_ = nullptr;
  Manager::Shutdown();
}

Camera* CameraManager::GetMainCamera() {
  COMET_ASSERT(main_camera_ != nullptr, "Main camera is null!");
  return main_camera_.get();
}

void CameraManager::GenerateMainCamera() {
  main_camera_ = std::make_unique<Camera>();
}
}  // namespace rendering
}  // namespace comet
