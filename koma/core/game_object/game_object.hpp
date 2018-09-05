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
  // We want to be sure that GameObject instances are created in a shared
  // pointer. Because of this, we can try to make our constructor private, and
  // create a static koma::GameObject::Create() method that returns a new
  // koma::GameObject instance pointed by a shared_ptr<GameObject> pointer.
  //
  // For performance reasons, we also want to use the
  // std::make_shared<GameObject> function, which then cannot access private
  // members of our koma::GameObject class, including our
  // koma::GameObject::Create() method.
  //
  // To solve this, a public constructor was implemented, needing a
  // constructor_tag_ struct to create an instance of a koma::GameObject
  // class.
  //
  // The constructor_tag_ struct cannot be accessed outside our
  // koma::GameObject class, making it impossible to call the public
  // constructor outside of the latter.
  //
  // Therefore, it is impossible to create a koma::GameObject instance without
  // having it pointed by a std::shared_ptr<GameObject> pointer, making the
  // use of koma::GameObject's shared_from_this safe.
  struct constructor_tag_ { explicit constructor_tag_() = default; };

 public:
  ~GameObject();
  GameObject(constructor_tag_);  // Please, see comment above.
  static std::shared_ptr<GameObject> Create();  // Same here.
  void Destroy();

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
