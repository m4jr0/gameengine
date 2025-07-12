// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_GEOMETRY_COMPONENT_MODEL_COMPONENT_H_
#define COMET_COMET_GEOMETRY_COMPONENT_MODEL_COMPONENT_H_

#include "comet/core/essentials.h"
#include "comet/entity/entity_id.h"
#include "comet/resource/model_resource.h"

namespace comet {
namespace geometry {
struct StaticModelComponent {
  entity::EntityId entity_id{entity::kInvalidEntityId};
  const resource::StaticModelResource* resource{nullptr};
};

struct SkeletalModelComponent {
  entity::EntityId entity_id{entity::kInvalidEntityId};
  const resource::SkeletalModelResource* resource{nullptr};
};
}  // namespace geometry
}  // namespace comet

#endif  // COMET_COMET_GEOMETRY_COMPONENT_MODEL_COMPONENT_H_
