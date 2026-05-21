// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_SANDBOX_SANDBOX_H_
#define COMET_SANDBOX_SANDBOX_H_

#include "comet/core/essentials.h"
#include "comet/core/memory/memory.h"
#include "comet/engine/engine_client.h"
#include "sandbox/camera_handler.h"

namespace comet {
namespace sandbox {
class Sandbox : public EngineClient {
 public:
  Sandbox() = default;
  Sandbox(const Sandbox&) = delete;
  Sandbox(Sandbox&&) = delete;
  Sandbox& operator=(const Sandbox&) = delete;
  Sandbox& operator=(Sandbox&&) = delete;
  ~Sandbox() override = default;

  void OnInitialize() override;
  void OnUpdate(f64& lag) override;
  void OnShutdown() override;

 private:
  void LoadSandboxScene();
  void UpdateSandboxScene(f64& lag);
  void UnloadSandboxScene();

  memory::UniquePtr<CameraHandler> camera_handler_{nullptr};
};
}  // namespace sandbox
}  // namespace comet

#endif  // COMET_SANDBOX_SANDBOX_H_