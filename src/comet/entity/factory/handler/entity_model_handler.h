// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_ENTITY_FACTORY_HANDLER_ENTITY_MODEL_HANDLER_H_
#define COMET_COMET_ENTITY_FACTORY_HANDLER_ENTITY_MODEL_HANDLER_H_

#include "comet_precompile.h"

#include "comet/entity/entity_id.h"
#include "comet/entity/factory/handler/entity_handler.h"
#include "comet/resource/resource_manager.h"

namespace comet {
namespace entity {
struct ModelHandlerDescr : HandlerDescr {
  resource::ResourceManager* resource_manager{nullptr};
};

class ModelHandler : public Handler {
 public:
  ModelHandler() = delete;
  explicit ModelHandler(const ModelHandlerDescr& descr);
  ModelHandler(const ModelHandler&) = delete;
  ModelHandler(ModelHandler&&) = delete;
  ModelHandler& operator=(const ModelHandler&) = delete;
  ModelHandler& operator=(ModelHandler&&) = delete;
  virtual ~ModelHandler() = default;

  EntityId Generate(const std::string& model_path) const;
  EntityId Generate(const schar* model_path) const;

 private:
  resource::ResourceManager* resource_manager_{nullptr};
};
}  // namespace entity
}  // namespace comet

#endif  // COMET_COMET_ENTITY_FACTORY_HANDLER_ENTITY_MODEL_HANDLER_H_
