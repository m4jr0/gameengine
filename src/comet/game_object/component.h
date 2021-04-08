// Copyright 2021 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_GAME_OBJECT_COMPONENT_H_
#define COMET_COMET_GAME_OBJECT_COMPONENT_H_

constexpr auto kLoggerCometCoreGameObjectComponent = "comet_core_render";

#include "boost/uuid/uuid.hpp"
#include "boost/uuid/uuid_generators.hpp"
#include "boost/uuid/uuid_io.hpp"
#include "comet_precompile.h"

namespace comet {
class GameObject;

class Component {
 public:
  virtual ~Component() = default;

  virtual void Initialize();
  virtual void Destroy();
  virtual void Update();
  virtual void FixedUpdate();

  const boost::uuids::uuid kId() const noexcept;

  std::shared_ptr<GameObject> game_object() noexcept;

 protected:
  std::shared_ptr<GameObject> game_object_ = nullptr;

 private:
  friend class GameObject;

  const boost::uuids::uuid kId_ = boost::uuids::random_generator()();
  void game_object(std::shared_ptr<GameObject>);
};
}  // namespace comet

#endif  // COMET_COMET_GAME_OBJECT_COMPONENT_H_
