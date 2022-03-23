// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_GAME_OBJECT_CAMERA_ORTHOGONAL_CAMERA_H_
#define COMET_COMET_GAME_OBJECT_CAMERA_ORTHOGONAL_CAMERA_H_

#include "comet/game_object/camera/camera.h"

namespace comet {
namespace game_object {
class OrthogonalCamera : public Camera {
 public:
  OrthogonalCamera() = default;
  OrthogonalCamera(const OrthogonalCamera&);
  OrthogonalCamera(OrthogonalCamera&&) noexcept;
  OrthogonalCamera& operator=(const OrthogonalCamera&);
  OrthogonalCamera& operator=(OrthogonalCamera&&) noexcept;
  virtual ~OrthogonalCamera() = default;

  virtual std::shared_ptr<Component> Clone() const override;
  virtual void UpdateProjectionMatrix(int, int) override;
};
}  // namespace game_object
}  // namespace comet

#endif  // COMET_COMET_GAME_OBJECT_CAMERA_ORTHOGONAL_CAMERA_H_
