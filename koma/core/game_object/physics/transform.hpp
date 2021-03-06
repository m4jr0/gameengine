// Copyright 2021 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef KOMA_CORE_GAME_OBJECT_PHYSICS_TRANFORM_HPP_
#define KOMA_CORE_GAME_OBJECT_PHYSICS_TRANFORM_HPP_

#include <unordered_map>

#include "core/game_object/component.hpp"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

namespace koma {
class Transform : public Component,
                  public std::enable_shared_from_this<Transform> {
 public:
  void Destroy() override;

  const glm::mat4 GetTransformMatrix() const;

  const glm::vec3 position() const noexcept;
  void position(float, float, float);
  void position(glm::vec3);
  const std::shared_ptr<Transform> parent() const noexcept;
  const std::shared_ptr<Transform> root_parent() const noexcept;

 protected:
  glm::vec3 position_ = glm::vec3(0, 0, 0);
  std::shared_ptr<Transform> parent_ = nullptr;
  std::shared_ptr<Transform> root_parent_ = nullptr;
  std::unordered_map<std::string, std::shared_ptr<Transform>> children_;
};
}  // namespace koma

#endif  // KOMA_CORE_GAME_OBJECT_PHYSICS_TRANFORM_HPP_
