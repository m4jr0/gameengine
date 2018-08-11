// Copyright 2018 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef KOMA_CORE_GAME_OBJECT_GAME_OBJECT_HPP_
#define KOMA_CORE_GAME_OBJECT_GAME_OBJECT_HPP_

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <unordered_map>

#include "component.hpp"

namespace koma {
class GameObject final {
 public:
  ~GameObject();

  void Update(double);
  void FixedUpdate();
  void AddComponent(Component*);
  void RemoveComponent(Component*);

  const boost::uuids::uuid kId() const noexcept;

 private:
    const boost::uuids::uuid kId_ = boost::uuids::random_generator()();
    std::unordered_map<std::string, Component*> components_;
};
};  // namespace koma

#endif  // KOMA_CORE_GAME_OBJECT_GAME_OBJECT_HPP_
