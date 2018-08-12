// Copyright 2018 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef KOMA_CORE_RENDERING_RENDERING_MANAGER_HPP_
#define KOMA_CORE_RENDERING_RENDERING_MANAGER_HPP_

#define LOGGER_KOMA_CORE_RENDERING "koma_core_rendering"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "../game_object/game_object_manager.hpp"

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

 private:
  int counter_ = 0;
  double current_time_ = 0;
  GLFWwindow *window_ = nullptr;
  GLuint current_width_ = 0;
  GLuint current_height_ = 0;

  std::string GetTitle();
};
};  // namespace koma

#endif  // KOMA_CORE_RENDERING_RENDERING_MANAGER_HPP_
