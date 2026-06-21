// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_runtime_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/runtime/entity/factory/entity_factory_manager.h"
////////////////////////////////////////////////////////////////////////////////

namespace comet {
namespace entity {
EntityFactoryManager& EntityFactoryManager::Get() {
  static EntityFactoryManager singleton{};
  return singleton;
}

const ModelHandler* EntityFactoryManager::GetModel() const {
  return model_handler_.get();
}

const PrimitiveHandler* EntityFactoryManager::GetPrimitive() const {
  return primitive_handler_.get();
}

void EntityFactoryManager::OnInitialize() {
  primitive_handler_ = std::make_unique<PrimitiveHandler>();
  model_handler_ = std::make_unique<ModelHandler>();
  primitive_handler_->Initialize();
  model_handler_->Initialize();
}

void EntityFactoryManager::OnShutdown() {
  model_handler_->Shutdown();
  primitive_handler_->Shutdown();
  model_handler_ = nullptr;
}
}  // namespace entity
}  // namespace comet
