// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_ENTITY_FACTORY_MODEL_ENTITY_H_
#define COMET_COMET_ENTITY_FACTORY_MODEL_ENTITY_H_

#include "comet_precompile.h"

#include "comet/entity/entity.h"

namespace comet {
namespace entity {
EntityId GenerateModelEntity(const std::string& model_path);
EntityId GenerateModelEntity(const schar* model_path);
}  // namespace entity
}  // namespace comet

#endif  // COMET_COMET_ENTITY_FACTORY_MODEL_ENTITY_H_
