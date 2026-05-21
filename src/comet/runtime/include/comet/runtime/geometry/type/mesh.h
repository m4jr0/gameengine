// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RUNTIME_GEOMETRY_TYPE_MESH_H_
#define COMET_RUNTIME_GEOMETRY_TYPE_MESH_H_

// External. ///////////////////////////////////////////////////////////////////
#include <atomic>
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/essentials.h"
#include "comet/core/type/array.h"
#include "comet/core/type/handle.h"
#include "comet/geometry/type/skeleton.h"
#include "comet/math/matrix.h"
#include "comet/math/vector.h"

namespace comet {
namespace geometry {
struct MeshTag {};
using MeshHandle = Handle<MeshTag>;

struct Mesh {
  static_assert(std::atomic<usize>::is_always_lock_free,
                "std::atomic<usize> must be always lock-free");

  MeshHandle handle{};
  bool is_dirty{false};
  std::atomic<usize> ref_count{0};
  MeshId id{kInvalidMeshId};
  MeshType type{MeshType::Unknown};
  math::Mat4 transform{1.0f};
  math::Vec3 local_center{.0f};
  math::Vec3 local_max_extents{.0f};
  Array<Index> indices{};
  Array<SkinnedVertex> vertices{};
};
}  // namespace geometry
}  // namespace comet

#endif  // COMET_RUNTIME_GEOMETRY_TYPE_MESH_H_