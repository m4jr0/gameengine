// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_GEOMETRY_COMPONENT_MESH_COMPONENT_H_
#define COMET_COMET_GEOMETRY_COMPONENT_MESH_COMPONENT_H_

#include "comet/core/essentials.h"
#include "comet/entity/type/entity_id.h"
#include "comet/geometry/type/mesh.h"
#include "comet/resource/material/material_resource.h"

namespace comet {
namespace geometry {
struct MeshComponent {
  entity::EntityId entity_id{entity::kInvalidEntityId};
  entity::EntityId model_entity_id{entity::kInvalidEntityId};
  MeshHandle mesh_handle{};
  resource::MaterialResourceId material_resource_id{};
};
}  // namespace geometry
}  // namespace comet

#endif  // COMET_COMET_GEOMETRY_COMPONENT_MESH_COMPONENT_H_