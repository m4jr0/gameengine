// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_GAME_OBJECT_PHYSICS_TRANFORM_H_
#define COMET_COMET_GAME_OBJECT_PHYSICS_TRANFORM_H_

#include "comet/game_object/component.h"
#include "comet_precompile.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

namespace comet {
namespace game_object {
class Transform : public Component,
                  public std::enable_shared_from_this<Transform> {
 public:
  Transform() = default;
  Transform(const Transform&);
  Transform(Transform&&) noexcept;
  Transform& operator=(const Transform&);
  Transform& operator=(Transform&&) noexcept;
  virtual ~Transform() = default;

  virtual std::shared_ptr<Component> Clone() const override;
  void Destroy() override;

  const glm::mat4 GetTransformMatrix() const;
  const glm::vec3& GetPosition() const noexcept;
  void SetPosition(float, float, float);
  void SetPosition(glm::vec3);
  std::shared_ptr<Transform> GetParent() noexcept;
  std::shared_ptr<Transform> GetRootParent() noexcept;

 protected:
  glm::vec3 position_ = glm::vec3(0, 0, 0);
  std::shared_ptr<Transform> parent_ = nullptr;
  std::shared_ptr<Transform> root_parent_ = nullptr;
  std::unordered_map<std::string, std::shared_ptr<Transform>> children_;
};
}  // namespace game_object
}  // namespace comet

#endif  // COMET_COMET_GAME_OBJECT_PHYSICS_TRANFORM_H_
