// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "geometry_mesh_label.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/debug_label.h"

namespace comet {
namespace geometry {
const schar* GetMeshTypeLabel(MeshType type) {
  switch (type) {
    case MeshType::Unknown:
      return "unknown";
    case MeshType::Static:
      return "static";
    case MeshType::Skinned:
      return "skinned";
    default:
      return kUnknownLabel;
  }
}
}  // namespace geometry
}  // namespace comet