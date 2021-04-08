// Copyright 2021 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_RENDERING_MANAGER_H_
#define COMET_COMET_RENDERING_RENDERING_MANAGER_H_

constexpr auto kLoggerCometCoreRenderRenderManager = "comet_core_render";

#include "comet/core/manager.h"
#include "comet/game_object/game_object_manager.h"
#include "comet/rendering/window/glfw_window.h"
#include "comet_precompile.h"

namespace comet {
class RenderManager : public Manager {
 public:
  static constexpr unsigned short int kOpenGLMajorVersion_ = 3;
  static constexpr unsigned short int kOpenGLMinorVersion_ = 3;

  RenderManager() = default;
  RenderManager(const RenderManager &) = delete;
  RenderManager(RenderManager &&) = delete;
  RenderManager &operator=(const RenderManager &) = delete;
  RenderManager &operator=(RenderManager &&) = delete;
  virtual ~RenderManager() = default;

  void Initialize() override;
  void Destroy() override;
  void Update(double, GameObjectManager *);

  const GlfwWindow *window() const;

 private:
  int counter_ = 0;
  double current_time_ = 0;
  std::unique_ptr<GlfwWindow> window_ = nullptr;
};
}  // namespace comet

#endif  // COMET_COMET_RENDERING_RENDERING_MANAGER_H_
