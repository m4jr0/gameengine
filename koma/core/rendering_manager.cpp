// Copyright 2018 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "rendering_manager.hpp"

namespace koma {
void RenderingManager::Initialize() {}
void RenderingManager::Destroy() {}

void RenderingManager::Update(double interpolation,
                              GameObjectManager *game_object_manager) {
  game_object_manager->Update(interpolation);
  ++this->counter_;
}
};  // namespace koma
