// Copyright 2018 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "component.hpp"

namespace koma {
  void Component::Update(double interpolation) {
    // TODO(m4jr0): Implement it!
  }

  void Component::FixedUpdate() {
    // TODO(m4jr0): Implement it!
  }

  const boost::uuids::uuid Component::id() const {
    return this->id_;
  }
};  // namespace koma
