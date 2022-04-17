// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_GAME_OBJECT_COMPONENT_H_
#define COMET_COMET_GAME_OBJECT_COMPONENT_H_

#include "boost/uuid/uuid.hpp"
#include "boost/uuid/uuid_generators.hpp"
#include "boost/uuid/uuid_io.hpp"
#include "comet_precompile.h"

namespace comet {
namespace game_object {
class GameObject;

class Component {
 public:
  Component() = default;
  Component(const Component&);
  Component(Component&&) noexcept;
  Component& operator=(const Component&);
  Component& operator=(Component&&) noexcept;
  virtual ~Component() = default;

  virtual std::shared_ptr<Component> Clone() const = 0;
  virtual void Initialize();
  virtual void Destroy();
  virtual void Update();
  virtual void FixedUpdate();

  const boost::uuids::uuid& GetId() const noexcept;
  std::shared_ptr<GameObject> GetGameObject() const noexcept;

 protected:
  std::shared_ptr<GameObject> game_object_ = nullptr;

 private:
  friend class GameObject;

  boost::uuids::uuid id_ = boost::uuids::random_generator()();

  void SetGameObject(std::shared_ptr<GameObject>);
};
}  // namespace game_object
}  // namespace comet

#endif  // COMET_COMET_GAME_OBJECT_COMPONENT_H_
