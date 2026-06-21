// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RUNTIME_ENTITY_FACTORY_HANDLER_ENTITY_PRIMITIVE_HANDLER_H_
#define COMET_RUNTIME_ENTITY_FACTORY_HANDLER_ENTITY_PRIMITIVE_HANDLER_H_

#include "comet/core/essentials.h"
#include "comet/runtime/entity/factory/handler/entity_handler.h"
#include "comet/runtime/entity/entity_id.h"
#include "comet/data/resource/common.h"

namespace comet {
namespace entity {
class PrimitiveHandler : public Handler {
 public:
  PrimitiveHandler() = default;
  PrimitiveHandler(const PrimitiveHandler&) = delete;
  PrimitiveHandler(PrimitiveHandler&&) = delete;
  PrimitiveHandler& operator=(const PrimitiveHandler&) = delete;
  PrimitiveHandler& operator=(PrimitiveHandler&&) = delete;
  ~PrimitiveHandler() override = default;

  EntityId GenerateCube(f32 size, resource::ResourceLifeSpan life_span) const;
};
}  // namespace entity
}  // namespace comet

#endif  // COMET_RUNTIME_ENTITY_FACTORY_HANDLER_ENTITY_PRIMITIVE_HANDLER_H_