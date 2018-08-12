// Copyright 2018 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef KOMA_CORE_GAME_OBJECT_CAMERA_PERSPECTIVE_CAMERA_HPP_
#define KOMA_CORE_GAME_OBJECT_CAMERA_PERSPECTIVE_CAMERA_HPP_

#include "camera.hpp"

namespace koma {
class PerspectiveCamera : public Camera {
 public:
   virtual void UpdateProjectionMatrix(int, int) override;

   void fov(float);

   const float fov() const noexcept;

protected:
  float fov_ = 45.0f;
};
};  // namespace koma

#endif  // KOMA_CORE_GAME_OBJECT_CAMERA_PERSPECTIVE_CAMERA_HPP_
