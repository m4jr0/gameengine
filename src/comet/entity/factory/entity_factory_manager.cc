// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "entity_factory_manager.h"

namespace comet {
namespace entity {
EntityFactoryManager& EntityFactoryManager::Get() {
  static EntityFactoryManager singleton{};
  return singleton;
}

void EntityFactoryManager::Initialize() {
  Manager::Initialize();

  model_handler_ = std::make_unique<ModelHandler>();
  model_handler_->Initialize();
}

void EntityFactoryManager::Shutdown() {
  model_handler_->Shutdown();
  model_handler_ = nullptr;
  Manager::Shutdown();
}

const ModelHandler* EntityFactoryManager::GetModel() const {
  return model_handler_.get();
}
}  // namespace entity
}  // namespace comet
