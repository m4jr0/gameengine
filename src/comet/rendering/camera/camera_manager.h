// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_CAMERA_CAMERA_MANAGER_H_
#define COMET_COMET_RENDERING_CAMERA_CAMERA_MANAGER_H_

#include "comet/core/essentials.h"
#include "comet/core/manager.h"
#include "comet/core/memory/memory.h"
#include "comet/event/event.h"
#include "comet/rendering/camera/camera.h"

namespace comet {
namespace rendering {
class CameraManager : public Manager {
 public:
  static CameraManager& Get();

  CameraManager() = default;
  CameraManager(const CameraManager&) = delete;
  CameraManager(CameraManager&&) = delete;
  CameraManager& operator=(const CameraManager&) = delete;
  CameraManager& operator=(CameraManager&&) = delete;
  virtual ~CameraManager() = default;

  void Initialize() override;
  void Shutdown() override;
  Camera* GetMainCamera();

 private:
  void OnEvent(const event::Event& event);
  void GenerateMainCamera();

  memory::UniquePtr<Camera> main_camera_{nullptr};
};
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_CAMERA_CAMERA_MANAGER_H_
