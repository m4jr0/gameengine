// Copyright 2018 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Allows debugging memory leaks.
#include "../../debug.hpp"

#include "component.hpp"

namespace koma {
void Component::Initialize(){
    // Code has to be implemented in children.
};

void Component::Update(double interpolation) {
  // Code has to be implemented in children.
  // This function is called every frame rendered.
}

void Component::FixedUpdate() {
  // Code has to be implemented in children.
  // This function is called every logic frame computed. In most cases, this
  // is where the game logic will be implemented.
}

const boost::uuids::uuid Component::kId() const noexcept {
  return this->kId_;
}
};  // namespace koma