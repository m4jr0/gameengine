// Copyright 2021 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_CORE_GAME_OBJECT_CAMERA_PERSPECTIVE_CAMERA_HPP_
#define COMET_CORE_GAME_OBJECT_CAMERA_PERSPECTIVE_CAMERA_HPP_

#include "comet/game_object/camera/camera.hpp"
#include "comet_precompile.hpp"

namespace comet {
class PerspectiveCamera : public Camera {
 public:
  virtual void UpdateProjectionMatrix(int, int) override;

  void fov(float) noexcept;

  const float fov() const noexcept;

 protected:
  float fov_ = 45.0f;
};
}  // namespace comet

#endif  // COMET_CORE_GAME_OBJECT_CAMERA_PERSPECTIVE_CAMERA_HPP_
