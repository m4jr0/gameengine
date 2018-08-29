// Copyright 2018 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef KOMA_CORE_GAME_OBJECT_PHYSICS_TRANFORM_HPP_
#define KOMA_CORE_GAME_OBJECT_PHYSICS_TRANFORM_HPP_

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <core/game_object/component.hpp>

namespace koma {
class Transform : public Component {
 public:
  const glm::mat4 GetTransformMatrix() const;

  const glm::vec3 position() const noexcept;
  void position(float, float, float);
  void position(glm::vec3);

 private:
  glm::vec3 position_ = glm::vec3(0, 0, 0);
};
};  // namespace koma

#endif  // KOMA_CORE_GAME_OBJECT_PHYSICS_TRANFORM_HPP_
