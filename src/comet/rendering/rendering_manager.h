// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_RENDERING_MANAGER_H_
#define COMET_COMET_RENDERING_RENDERING_MANAGER_H_

#include "comet/core/manager.h"
#include "comet/game_object/game_object_manager.h"
#include "comet/rendering/window/glfw_window.h"
#include "comet_precompile.h"

namespace comet {
namespace rendering {
class RenderingManager : public core::Manager {
 public:
  static constexpr unsigned short int kOpenGLMajorVersion_ = 3;
  static constexpr unsigned short int kOpenGLMinorVersion_ = 3;

  RenderingManager() = default;
  RenderingManager(const RenderingManager&) = delete;
  RenderingManager(RenderingManager&&) = delete;
  RenderingManager& operator=(const RenderingManager&) = delete;
  RenderingManager& operator=(RenderingManager&&) = delete;
  virtual ~RenderingManager() = default;

  void Initialize() override;
  void Destroy() override;
  void Update(double, game_object::GameObjectManager&);

  void IncrementSomething() { something_++; }

  const GlfwWindow* GetWindow() const;

 private:
  int something_ = 0;
  int counter_ = 0;
  double current_time_ = 0;
  std::unique_ptr<GlfwWindow> window_ = nullptr;
};
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_RENDERING_MANAGER_H_
