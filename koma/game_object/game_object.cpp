// Copyright 2018 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "game_object.hpp"

namespace koma {
void GameObject::Update(double interpolation) {
  // TODO(m4jr0): Implement it!
}

void GameObject::FixedUpdate() {
  // TODO(m4jr0): Implement it!
}

const boost::uuids::uuid GameObject::id() const {
  return this->id_;
}
};  // namespace koma
