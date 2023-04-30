// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "entity_factory_manager.h"

namespace comet {
namespace entity {
EntityFactoryManager::EntityFactoryManager(
    const EntityFactoryManagerDescr& descr)
    : Manager{descr},
      entity_manager_{descr.entity_manager},
      resource_manager_{descr.resource_manager} {
  COMET_ASSERT(entity_manager_ != nullptr, "Entity manager is null!");
  COMET_ASSERT(resource_manager_ != nullptr, "Resource manager is null!");
}

void EntityFactoryManager::Initialize() {
  Manager::Initialize();

  ModelHandlerDescr model_handler_descr{};
  model_handler_descr.entity_manager = entity_manager_;
  model_handler_descr.resource_manager = resource_manager_;
  model_handler_ = std::make_unique<ModelHandler>(model_handler_descr);
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
