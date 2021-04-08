// Copyright 2021 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_GAME_OBJECT_CAMERA_PERSPECTIVE_CAMERA_H_
#define COMET_COMET_GAME_OBJECT_CAMERA_PERSPECTIVE_CAMERA_H_

#include "comet/game_object/camera/camera.h"
#include "comet_precompile.h"

namespace comet {
namespace game_object {
class PerspectiveCamera : public Camera {
 public:
  virtual void UpdateProjectionMatrix(int, int) override;

  void fov(float) noexcept;

  const float fov() const noexcept;

 protected:
  float fov_ = 45.0f;
};
}  // namespace game_object
}  // namespace comet

#endif  // COMET_COMET_GAME_OBJECT_CAMERA_PERSPECTIVE_CAMERA_H_
