// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RENDER_UTILS_GEOMETRY_UTILS_H_
#define COMET_RENDER_UTILS_GEOMETRY_UTILS_H_

#include "comet/core/essentials.h"
#include "comet/core/container/array.h"
#include "comet/data/geometry/mesh.h"
#include "comet/core/math/bounding_volume.h"
#include "comet/core/math/vector.h"
#include "comet/render/frustum.h"

namespace comet {
namespace render {
math::Vec4 GenerateTangentWithSign(const math::Vec3& raw_normal,
                                   const math::Vec3& raw_tangent,
                                   const math::Vec3* raw_bitangent = nullptr);

void GenerateGeometry(const math::Aabb& aabb,
                      Array<geometry::SkinnedVertex>& vertices,
                      Array<geometry::Index>& indices, bool is_visible);

void GenerateGeometry(const Frustum& frustum,
                      Array<geometry::SkinnedVertex>& vertices,
                      Array<geometry::Index>& indices);
}  // namespace render
}  // namespace comet

#endif  // COMET_RENDER_UTILS_GEOMETRY_UTILS_H_