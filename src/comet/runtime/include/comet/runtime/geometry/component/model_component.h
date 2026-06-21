// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RUNTIME_GEOMETRY_COMPONENT_MODEL_COMPONENT_H_
#define COMET_RUNTIME_GEOMETRY_COMPONENT_MODEL_COMPONENT_H_

#include "comet/core/essentials.h"
#include "comet/runtime/entity/entity_id.h"
#include "comet/data/resource/model/model_resource.h"

namespace comet {
namespace geometry {
struct StaticModelComponent {
  entity::EntityId entity_id{entity::kInvalidEntityId};
  resource::StaticModelResourceHandle resource_handle{};
};

struct SkeletalModelComponent {
  entity::EntityId entity_id{entity::kInvalidEntityId};
  resource::SkeletalModelResourceHandle resource_handle{};
};
}  // namespace geometry
}  // namespace comet

#endif  // COMET_RUNTIME_GEOMETRY_COMPONENT_MODEL_COMPONENT_H_