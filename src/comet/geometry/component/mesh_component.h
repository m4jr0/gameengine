// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_GEOMETRY_COMPONENT_MESH_COMPONENT_H_
#define COMET_COMET_GEOMETRY_COMPONENT_MESH_COMPONENT_H_

#include "comet/core/essentials.h"
#include "comet/entity/entity_id.h"
#include "comet/geometry/geometry_common.h"
#include "comet/resource/material_resource.h"
#include "comet/resource/model_resource.h"

namespace comet {
namespace geometry {
struct MeshComponent {
  entity::EntityId entity_id{entity::kInvalidEntityId};
  entity::EntityId model_entity_id{entity::kInvalidEntityId};
  geometry::Mesh* mesh{nullptr};
  const resource::MaterialResource* material{nullptr};
};

struct SkeletonComponent {
  const resource::SkeletonResource* resource{nullptr};
};
}  // namespace geometry
}  // namespace comet

#endif  // COMET_COMET_GEOMETRY_COMPONENT_MESH_COMPONENT_H_
