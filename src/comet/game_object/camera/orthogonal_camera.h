// Copyright 2021 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_GAME_OBJECT_CAMERA_ORTHOGONAL_CAMERA_H_
#define COMET_COMET_GAME_OBJECT_CAMERA_ORTHOGONAL_CAMERA_H_

#include "comet/game_object/camera/camera.h"

namespace comet {
class OrthogonalCamera : public Camera {
 public:
  virtual void UpdateProjectionMatrix(int, int) override;
};
}  // namespace comet

#endif  // COMET_COMET_GAME_OBJECT_CAMERA_ORTHOGONAL_CAMERA_H_
