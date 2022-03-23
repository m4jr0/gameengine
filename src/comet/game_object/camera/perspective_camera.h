// Copyright 2022 m4jr0. All Rights Reserved.
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
  PerspectiveCamera() = default;
  PerspectiveCamera(const PerspectiveCamera&);
  PerspectiveCamera(PerspectiveCamera&&) noexcept;
  PerspectiveCamera& operator=(const PerspectiveCamera&);
  PerspectiveCamera& operator=(PerspectiveCamera&&) noexcept;
  virtual ~PerspectiveCamera() = default;

  virtual std::shared_ptr<Component> Clone() const override;
  virtual void UpdateProjectionMatrix(int, int) override;

  float GetFov() const noexcept;
  void SetFov(float) noexcept;

 protected:
  float fov_ = 45.0f;
};
}  // namespace game_object
}  // namespace comet

#endif  // COMET_COMET_GAME_OBJECT_CAMERA_PERSPECTIVE_CAMERA_H_
