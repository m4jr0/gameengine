// Copyright 2018 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef KOMA_CORE_RENDERING_MANAGER_HPP_
#define KOMA_CORE_RENDERING_MANAGER_HPP_

#include "game_object_manager.hpp"

namespace koma {
class RenderingManager {
 public:
  void Initialize();
  void Destroy();

  void Update(double, GameObjectManager *);
  void ResetCounter() noexcept { this->counter_ = 0; };

  const int counter() const noexcept { return this->counter_; };
 private:
  int counter_ = 0;
};
};  // namespace koma

#endif  // KOMA_CORE_RENDERING_MANAGER_HPP_
