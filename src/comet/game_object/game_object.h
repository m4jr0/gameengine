// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_GAME_OBJECT_GAME_OBJECT_H_
#define COMET_COMET_GAME_OBJECT_GAME_OBJECT_H_

#include "boost/uuid/uuid.hpp"
#include "boost/uuid/uuid_generators.hpp"
#include "boost/uuid/uuid_io.hpp"
#include "comet/game_object/component.h"
#include "comet_precompile.h"

namespace comet {
namespace game_object {
class GameObject final : public std::enable_shared_from_this<GameObject> {
 public:
  GameObject() = default;
  GameObject(const std::shared_ptr<GameObject>) = delete;
  GameObject(GameObject&&) = delete;
  GameObject& operator=(const GameObject&) = delete;
  GameObject& operator=(GameObject&&) = delete;
  ~GameObject() = default;

  std::shared_ptr<GameObject> Clone() const;
  static std::shared_ptr<GameObject> Create();
  void Destroy();
  void Update();
  void FixedUpdate();
  void AddComponent(std::shared_ptr<Component> component);
  void RemoveComponent(std::shared_ptr<Component> component);
  std::shared_ptr<Component> GetComponent(
      const boost::uuids::uuid& component_id);

  template <typename ComponentType>
  auto GetComponent() {
    for (const auto& it : components_) {
      if (const auto to_return =
              std::dynamic_pointer_cast<ComponentType>(it.second)) {
        return to_return;
      }
    }

    return std::shared_ptr<ComponentType>{};
  };

  const boost::uuids::uuid GetId() const noexcept;

 private:
  boost::uuids::uuid id_ = boost::uuids::random_generator()();
  std::unordered_map<std::string, std::shared_ptr<Component>> components_;
};
}  // namespace game_object
}  // namespace comet

#endif  // COMET_COMET_GAME_OBJECT_GAME_OBJECT_H_
