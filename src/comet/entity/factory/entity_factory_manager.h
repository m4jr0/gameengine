// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_ENTITY_FACTORY_ENTITY_FACTORY_MANAGER_H_
#define COMET_COMET_ENTITY_FACTORY_ENTITY_FACTORY_MANAGER_H_

#include "comet_precompile.h"

#include "comet/core/manager.h"
#include "comet/entity/entity_manager.h"
#include "comet/entity/factory/handler/entity_model_handler.h"
#include "comet/resource/resource_manager.h"

namespace comet {
namespace entity {
struct EntityFactoryManagerDescr : ManagerDescr {
  EntityManager* entity_manager{nullptr};
  resource::ResourceManager* resource_manager{nullptr};
};

class EntityFactoryManager : public Manager {
 public:
  EntityFactoryManager() = delete;
  explicit EntityFactoryManager(const EntityFactoryManagerDescr& descr);
  EntityFactoryManager(const EntityFactoryManager&) = delete;
  EntityFactoryManager(EntityFactoryManager&&) = delete;
  EntityFactoryManager& operator=(const EntityFactoryManager&) = delete;
  EntityFactoryManager& operator=(EntityFactoryManager&&) = delete;
  virtual ~EntityFactoryManager() = default;

  void Initialize() override;
  void Shutdown() override;

  const ModelHandler* GetModel() const;

 private:
  EntityManager* entity_manager_{nullptr};
  resource::ResourceManager* resource_manager_{nullptr};
  std::unique_ptr<ModelHandler> model_handler_{nullptr};
};
}  // namespace entity
}  // namespace comet

#endif  // COMET_COMET_ENTITY_FACTORY_ENTITY_FACTORY_MANAGER_H_
