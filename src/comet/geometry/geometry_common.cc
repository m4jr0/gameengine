// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "geometry_common.h"

namespace comet {
namespace geometry {
const schar* GetMeshTypeLabel(MeshType mesh_type) {
  switch (mesh_type) {
    case MeshType::Unknown:
      return "unknown";

    case MeshType::Static:
      return "static";

    case MeshType::Skinned:
      return "skinned";
  }

  return "???";
}
}  // namespace geometry
}  // namespace comet
