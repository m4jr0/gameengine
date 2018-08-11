// Copyright 2018 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef KOMA_CORE_RENDERING_RENDERING_MANAGER_HPP_
#define KOMA_CORE_RENDERING_RENDERING_MANAGER_HPP_

#define LOGGER_KOMA_CORE_RENDERING "koma_core_rendering"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>
#include <stdexcept>

#include "../../utils/logger.hpp"
#include "../game_object/game_object_manager.hpp"
#include "../resource/locator.hpp"

namespace koma {
class RenderingManager {
 public:
  static constexpr char kRenderingWindowName[] = "Koma Game Engine";
  static constexpr unsigned short int kOpenGLMajorVersion = 3;
  static constexpr unsigned short int kOpenGLMinorVersion = 3;
  static constexpr unsigned int kDefaultWidth = 1280;
  static constexpr unsigned int kDefaultHeight = 720;

  void Initialize();
  void Destroy();

  void Update(double, GameObjectManager *);

  const int counter() const noexcept { return this->counter_; };

 private:
  int counter_ = 0;
  double current_time_ = 0;
  GLFWwindow *window = nullptr;
  GLuint current_width = 0;
  GLuint current_height = 0;

  std::string GetTitle();
};
};  // namespace koma

#endif  // KOMA_CORE_RENDERING_RENDERING_MANAGER_HPP_
