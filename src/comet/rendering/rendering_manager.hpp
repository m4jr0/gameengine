// Copyright 2021 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_CORE_RENDER_RENDER_MANAGER_HPP_
#define COMET_CORE_RENDER_RENDER_MANAGER_HPP_

constexpr auto kLoggerCometCoreRenderRenderManager = "comet_core_render";

#include "comet/core/manager.hpp"
#include "comet/game_object/game_object_manager.hpp"
#include "comet/rendering/window/glfw_window.hpp"
#include "comet_precompile.hpp"

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

#endif  // COMET_CORE_RENDER_RENDER_MANAGER_HPP_
