// Copyright 2018 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef KOMA_CORE_GAME_OBJECT_GAME_OBJECT_HPP_
#define KOMA_CORE_GAME_OBJECT_GAME_OBJECT_HPP_

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <memory>
#include <unordered_map>

#include "component.hpp"

namespace koma {
class GameObject final : public std::enable_shared_from_this<GameObject> {
 protected:
  struct constructor_tag_ { explicit constructor_tag_() = default; };

 public:
  ~GameObject();
  GameObject(constructor_tag_);
  static std::shared_ptr<GameObject> Create();

  void Update();
  void FixedUpdate();
  void AddComponent(std::shared_ptr<Component>);
  void RemoveComponent(std::shared_ptr<Component>);
  std::shared_ptr<Component> GetComponent(const boost::uuids::uuid);

  template<typename ComponentType>
  auto GetComponent() {
    for (auto it : this->components_) {
      if (
        auto to_return = std::dynamic_pointer_cast<ComponentType>(it.second)
        ) {
        return to_return;
      }
    }

    return std::shared_ptr<ComponentType>{};
  };

  const boost::uuids::uuid kId() const noexcept;

 private:
  const boost::uuids::uuid kId_ = boost::uuids::random_generator()();
  std::unordered_map<std::string, std::shared_ptr<Component>> components_;
};
};  // namespace koma

#endif  // KOMA_CORE_GAME_OBJECT_GAME_OBJECT_HPP_
