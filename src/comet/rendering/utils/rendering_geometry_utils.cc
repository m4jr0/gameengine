// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "rendering_geometry_utils.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/math/math_scalar.h"
#include "comet/math/numeric_utils.h"
#include "comet/math/plane.h"
#include "comet/rendering/type/rendering_texture_type.h"

namespace comet {
namespace rendering {
namespace internal {
math::Vec3 GetFallbackTangentAxis(const math::Vec3& normal) {
  return math::Abs(normal.y) < .999f ? math::Vec3{.0f, 1.0f, .0f}
                                     : math::Vec3{1.0f, .0f, .0f};
}

math::Vec3 SanitizeNormal(math::Vec3 normal) {
  if (!math::IsAlmostZero(math::GetSquaredMagnitude(normal))) {
    math::Normalize(normal);
    return normal;
  }

  return math::Vec3{.0f, 1.0f, .0f};
}
}  // namespace internal

math::Vec4 GenerateTangentWithSign(const math::Vec3& raw_normal,
                                   const math::Vec3& raw_tangent,
                                   const math::Vec3* raw_bitangent) {
  const auto normal{internal::SanitizeNormal(raw_normal)};
  auto tangent{raw_tangent};

  if (math::IsAlmostZero(math::GetSquaredMagnitude(tangent))) {
    tangent = math::Cross(internal::GetFallbackTangentAxis(normal), normal);
  }

  tangent = tangent - normal * math::Dot(normal, tangent);

  if (math::IsAlmostZero(math::GetSquaredMagnitude(tangent))) {
    tangent = math::Cross(internal::GetFallbackTangentAxis(normal), normal);
  }

  math::Normalize(tangent);
  auto sign{1.0f};

  if (raw_bitangent != nullptr) {
    auto bitangent{*raw_bitangent};

    if (!math::IsAlmostZero(math::GetSquaredMagnitude(bitangent))) {
      math::Normalize(bitangent);
      sign = math::Dot(math::Cross(normal, tangent), bitangent) < .0f ? -1.0f
                                                                      : 1.0f;
    }
  }

  return math::Vec4{tangent.x, tangent.y, tangent.z, sign};
}

void GenerateGeometry(const math::Aabb& aabb,
                      Array<geometry::SkinnedVertex>& vertices,
                      Array<geometry::Index>& indices, bool is_visible) {
  COMET_ASSERT(
      vertices.GetSize() == 0, "rendering_geometry_utils::GenerateGeometry",
      "aabb vertex array is not empty", "vertex_count", vertices.GetSize());
  COMET_ASSERT(
      indices.GetSize() == 0, "rendering_geometry_utils::GenerateGeometry",
      "aabb index array is not empty", "index_count", indices.GetSize());

  const math::Vec3 extents{aabb.extents[0], aabb.extents[1], aabb.extents[2]};

  vertices.Reserve(8);

  geometry::SkinnedVertex vertex{};
  vertex.color = math::Vec4{is_visible ? kColorGreenRgb : kColorRedRgb, 1.0f};

  constexpr auto kTopRightFarIndex{0};
  vertex.position = aabb.center + math::Vec3(extents.x, extents.y, -extents.z);
  vertices.PushBack(vertex);

  constexpr auto kTopRightNearIndex{1};
  vertex.position = aabb.center + math::Vec3(extents.x, extents.y, extents.z);
  vertices.PushBack(vertex);

  constexpr auto kTopLeftFarIndex{2};
  vertex.position = aabb.center + math::Vec3(-extents.x, extents.y, -extents.z);
  vertices.PushBack(vertex);

  constexpr auto kTopLeftNearIndex{3};
  vertex.position = aabb.center + math::Vec3(-extents.x, extents.y, extents.z);
  vertices.PushBack(vertex);

  constexpr auto kBottomRightFarIndex{4};
  vertex.position = aabb.center + math::Vec3(extents.x, -extents.y, -extents.z);
  vertices.PushBack(vertex);

  constexpr auto kBottomRightNearIndex{5};
  vertex.position = aabb.center + math::Vec3(extents.x, -extents.y, extents.z);
  vertices.PushBack(vertex);

  constexpr auto kBottomLeftFarIndex{6};
  vertex.position =
      aabb.center + math::Vec3(-extents.x, -extents.y, -extents.z);
  vertices.PushBack(vertex);

  constexpr auto kBottomLeftNearIndex{7};
  vertex.position = aabb.center + math::Vec3(-extents.x, -extents.y, extents.z);
  vertices.PushBack(vertex);

  indices.Reserve(36);
  indices.PushBack(kTopRightFarIndex);
  indices.PushBack(kTopRightNearIndex);
  indices.PushBack(kTopLeftNearIndex);

  indices.PushBack(kTopRightFarIndex);
  indices.PushBack(kTopLeftNearIndex);
  indices.PushBack(kTopLeftFarIndex);

  indices.PushBack(kTopRightFarIndex);
  indices.PushBack(kBottomRightNearIndex);
  indices.PushBack(kTopRightNearIndex);

  indices.PushBack(kTopRightFarIndex);
  indices.PushBack(kBottomRightFarIndex);
  indices.PushBack(kBottomRightNearIndex);

  indices.PushBack(kBottomRightFarIndex);
  indices.PushBack(kBottomRightNearIndex);
  indices.PushBack(kBottomLeftFarIndex);

  indices.PushBack(kBottomLeftFarIndex);
  indices.PushBack(kBottomRightNearIndex);
  indices.PushBack(kBottomLeftNearIndex);

  indices.PushBack(kTopLeftNearIndex);
  indices.PushBack(kBottomLeftNearIndex);
  indices.PushBack(kBottomLeftFarIndex);

  indices.PushBack(kTopLeftNearIndex);
  indices.PushBack(kBottomLeftFarIndex);
  indices.PushBack(kTopLeftFarIndex);

  indices.PushBack(kTopRightFarIndex);
  indices.PushBack(kBottomRightFarIndex);
  indices.PushBack(kTopLeftFarIndex);

  indices.PushBack(kTopLeftFarIndex);
  indices.PushBack(kBottomRightFarIndex);
  indices.PushBack(kBottomLeftFarIndex);

  indices.PushBack(kTopLeftNearIndex);
  indices.PushBack(kTopRightNearIndex);
  indices.PushBack(kBottomRightNearIndex);

  indices.PushBack(kTopLeftNearIndex);
  indices.PushBack(kBottomRightNearIndex);
  indices.PushBack(kBottomLeftNearIndex);
}

void GenerateGeometry(const Frustum& frustum,
                      Array<geometry::SkinnedVertex>& vertices,
                      Array<geometry::Index>& indices) {
  COMET_ASSERT(
      vertices.GetSize() == 0, "rendering_geometry_utils::GenerateGeometry",
      "frustum vertex array is not empty", "vertex_count", vertices.GetSize());
  COMET_ASSERT(
      indices.GetSize() == 0, "rendering_geometry_utils::GenerateGeometry",
      "frustum index array is not empty", "index_count", indices.GetSize());

  const auto& top_face{frustum.GetTop()};
  const auto& bottom_face{frustum.GetBottom()};
  const auto& left_face{frustum.GetLeft()};
  const auto& right_face{frustum.GetRight()};
  const auto& near_face{frustum.GetNear()};
  const auto& far_face{frustum.GetFar()};

  vertices.Reserve(8);

  geometry::SkinnedVertex vertex{};
  vertex.color = math::Vec4{kColorBlackRgb, 1.0f};
  [[maybe_unused]] auto is_intersection{false};

  constexpr auto kTopRightFarIndex{0};

  is_intersection =
      math::Intersect(top_face, far_face, right_face, vertex.position);
  COMET_ASSERT(is_intersection, "rendering_geometry_utils::GenerateGeometry",
               "top far right frustum planes do not intersect");

  vertices.PushBack(vertex);

  constexpr auto kTopRightNearIndex{1};

  is_intersection =
      math::Intersect(top_face, near_face, right_face, vertex.position);
  COMET_ASSERT(is_intersection, "rendering_geometry_utils::GenerateGeometry",
               "top near right frustum planes do not intersect");

  vertices.PushBack(vertex);

  constexpr auto kTopLeftFarIndex{2};

  is_intersection =
      math::Intersect(top_face, far_face, left_face, vertex.position);
  COMET_ASSERT(is_intersection, "rendering_geometry_utils::GenerateGeometry",
               "top far left frustum planes do not intersect");

  vertices.PushBack(vertex);

  constexpr auto kTopLeftNearIndex{3};

  is_intersection =
      math::Intersect(top_face, near_face, left_face, vertex.position);
  COMET_ASSERT(is_intersection, "rendering_geometry_utils::GenerateGeometry",
               "top near left frustum planes do not intersect");

  vertices.PushBack(vertex);

  constexpr auto kBottomRightFarIndex{4};

  is_intersection =
      math::Intersect(bottom_face, far_face, right_face, vertex.position);
  COMET_ASSERT(is_intersection, "rendering_geometry_utils::GenerateGeometry",
               "bottom far right frustum planes do not intersect");

  vertices.PushBack(vertex);

  constexpr auto kBottomRightNearIndex{5};

  is_intersection =
      math::Intersect(bottom_face, near_face, right_face, vertex.position);
  COMET_ASSERT(is_intersection, "rendering_geometry_utils::GenerateGeometry",
               "bottom near right frustum planes do not intersect");

  vertices.PushBack(vertex);

  constexpr auto kBottomLeftFarIndex{6};

  is_intersection =
      math::Intersect(bottom_face, far_face, left_face, vertex.position);
  COMET_ASSERT(is_intersection, "rendering_geometry_utils::GenerateGeometry",
               "bottom far left frustum planes do not intersect");

  vertices.PushBack(vertex);

  constexpr auto kBottomLeftNearIndex{7};

  is_intersection =
      math::Intersect(bottom_face, near_face, left_face, vertex.position);
  COMET_ASSERT(is_intersection, "rendering_geometry_utils::GenerateGeometry",
               "bottom near left frustum planes do not intersect");
  vertices.PushBack(vertex);

  indices.Reserve(36);
  indices.PushBack(kTopRightFarIndex);
  indices.PushBack(kTopRightNearIndex);
  indices.PushBack(kTopLeftNearIndex);

  indices.PushBack(kTopRightFarIndex);
  indices.PushBack(kTopLeftNearIndex);
  indices.PushBack(kTopLeftFarIndex);

  indices.PushBack(kTopRightFarIndex);
  indices.PushBack(kBottomRightNearIndex);
  indices.PushBack(kTopRightNearIndex);

  indices.PushBack(kTopRightFarIndex);
  indices.PushBack(kBottomRightFarIndex);
  indices.PushBack(kBottomRightNearIndex);

  indices.PushBack(kBottomRightFarIndex);
  indices.PushBack(kBottomRightNearIndex);
  indices.PushBack(kBottomLeftFarIndex);

  indices.PushBack(kBottomLeftFarIndex);
  indices.PushBack(kBottomRightNearIndex);
  indices.PushBack(kBottomLeftNearIndex);

  indices.PushBack(kTopLeftNearIndex);
  indices.PushBack(kBottomLeftNearIndex);
  indices.PushBack(kBottomLeftFarIndex);

  indices.PushBack(kTopLeftNearIndex);
  indices.PushBack(kBottomLeftFarIndex);
  indices.PushBack(kTopLeftFarIndex);

  indices.PushBack(kTopRightFarIndex);
  indices.PushBack(kBottomRightFarIndex);
  indices.PushBack(kTopLeftFarIndex);

  indices.PushBack(kTopLeftFarIndex);
  indices.PushBack(kBottomRightFarIndex);
  indices.PushBack(kBottomLeftFarIndex);

  indices.PushBack(kTopLeftNearIndex);
  indices.PushBack(kTopRightNearIndex);
  indices.PushBack(kBottomRightNearIndex);

  indices.PushBack(kTopLeftNearIndex);
  indices.PushBack(kBottomRightNearIndex);
  indices.PushBack(kBottomLeftNearIndex);
}
}  // namespace rendering
}  // namespace comet