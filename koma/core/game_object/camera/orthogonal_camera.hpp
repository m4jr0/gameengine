// Copyright 2021 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef KOMA_CORE_GAME_OBJECT_CAMERA_ORTHOGONAL_CAMERA_HPP_
#define KOMA_CORE_GAME_OBJECT_CAMERA_ORTHOGONAL_CAMERA_HPP_

#include "camera.hpp"

namespace koma {
class OrthogonalCamera : public Camera {
public:
  virtual void UpdateProjectionMatrix(int, int) override;
};
}  // namespace koma

#endif  // KOMA_CORE_GAME_OBJECT_CAMERA_ORTHOGONAL_CAMERA_HPP_
