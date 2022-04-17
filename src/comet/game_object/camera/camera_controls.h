// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_GAME_OBJECT_CAMERA_CAMERA_CONTROLS_H_
#define COMET_COMET_GAME_OBJECT_CAMERA_CAMERA_CONTROLS_H_

#include "comet/game_object/component.h"
#include "comet_precompile.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

namespace comet {
namespace game_object {
class CameraControls : public Component {
 public:
  CameraControls() = default;
  CameraControls(const CameraControls&);
  CameraControls(CameraControls&&) noexcept;
  CameraControls& operator=(const CameraControls&);
  CameraControls& operator=(CameraControls&&) noexcept;
  virtual ~CameraControls() = default;

  virtual std::shared_ptr<Component> Clone() const override;
  virtual void Update() override;

 private:
  glm::vec3 position_ = glm::vec3(0, 0, 5);
  float horizontal_angle_ = 3.14f;
  float vertical_angle_ = 0.0f;
  float speed_ = 0.01f;
  float mouse_speed_ = 0.005f;
};
}  // namespace game_object
}  // namespace comet

#endif  // COMET_COMET_GAME_OBJECT_CAMERA_CAMERA_CONTROLS_H_
