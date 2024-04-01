// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_ENTITY_FACTORY_HANDLER_ENTITY_MODEL_HANDLER_H_
#define COMET_COMET_ENTITY_FACTORY_HANDLER_ENTITY_MODEL_HANDLER_H_

#include "comet/core/essentials.h"
#include "comet/core/type/tstring.h"
#include "comet/entity/entity_id.h"
#include "comet/entity/factory/handler/entity_handler.h"

namespace comet {
namespace entity {
class ModelHandler : public Handler {
 public:
  ModelHandler() = default;
  ModelHandler(const ModelHandler&) = delete;
  ModelHandler(ModelHandler&&) = delete;
  ModelHandler& operator=(const ModelHandler&) = delete;
  ModelHandler& operator=(ModelHandler&&) = delete;
  virtual ~ModelHandler() = default;

  EntityId GenerateStatic(CTStringView model_path) const;
  EntityId GenerateSkeletal(CTStringView model_path) const;
};
}  // namespace entity
}  // namespace comet

#endif  // COMET_COMET_ENTITY_FACTORY_HANDLER_ENTITY_MODEL_HANDLER_H_
