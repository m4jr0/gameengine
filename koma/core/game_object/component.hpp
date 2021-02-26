// Copyright 2021 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef KOMA_CORE_GAME_OBJECT_COMPONENT_HPP_
#define KOMA_CORE_GAME_OBJECT_COMPONENT_HPP_

#define LOGGER_KOMA_CORE_GAME_OBJECT_COMPONENT "koma_core_render"

#include <memory>

#include "boost/uuid/uuid.hpp"
#include "boost/uuid/uuid_generators.hpp"
#include "boost/uuid/uuid_io.hpp"

namespace koma {
class GameObject;

class Component {
 public:
  virtual ~Component(){};
  virtual void Initialize();
  virtual void Destroy();
  virtual void Update();
  virtual void FixedUpdate();

  const boost::uuids::uuid kId() const noexcept;
  std::shared_ptr<GameObject> game_object() noexcept;

 protected:
  std::shared_ptr<GameObject> game_object_ = nullptr;

 private:
  const boost::uuids::uuid kId_ = boost::uuids::random_generator()();
  void game_object(std::shared_ptr<GameObject>);

  friend class GameObject;
};
}  // namespace koma

#endif  // KOMA_CORE_GAME_OBJECT_COMPONENT_HPP_
