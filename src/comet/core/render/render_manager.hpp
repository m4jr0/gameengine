// Copyright 2021 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_CORE_RENDER_RENDER_MANAGER_HPP_
#define COMET_CORE_RENDER_RENDER_MANAGER_HPP_

constexpr auto kLoggerCometCoreRenderRenderManager = "comet_core_render";

#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "core/game_object/game_object_manager.hpp"
#include "core/manager.hpp"

namespace comet {
class RenderManager : public Manager {
 public:
  static constexpr char kDefaultRenderingWindowName_[] = "Comet Game Engine";
  static constexpr unsigned short int kOpenGLMajorVersion_ = 3;
  static constexpr unsigned short int kOpenGLMinorVersion_ = 3;
  static constexpr unsigned int kDefaultWidth_ = 1280;
  static constexpr unsigned int kDefaultHeight_ = 720;

  RenderManager() = default;
  RenderManager(const RenderManager &) = delete;
  RenderManager(RenderManager &&) = delete;
  RenderManager &operator=(const RenderManager &) = delete;
  RenderManager &operator=(RenderManager &&) = delete;
  virtual ~RenderManager() = default;

  void Initialize() override;
  void Destroy() override;
  void Update(double, GameObjectManager *);

  const int width() const noexcept;
  const int height() const noexcept;
  const GLFWwindow *window() const noexcept;
  void width(int) noexcept;
  void height(int) noexcept;

 private:
  std::string GetTitle();

  int counter_ = 0;
  double current_time_ = 0;
  GLFWwindow *window_ = nullptr;
  int width_ = 0;
  int height_ = 0;
};
}  // namespace comet

#endif  // COMET_CORE_RENDER_RENDER_MANAGER_HPP_
