// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "frustum.h"
////////////////////////////////////////////////////////////////////////////////

namespace comet {
namespace rendering {
Frustum::Frustum(const math::Plane& top_face, const math::Plane& bottom_face,
                 const math::Plane& left_face, const math::Plane& right_face,
                 const math::Plane& far_face, const math::Plane& near_face)
    : top_face_{top_face},
      bottom_face_{bottom_face},
      left_face_{left_face},
      right_face_{right_face},
      far_face_{far_face},
      near_face_{near_face} {}

bool Frustum::IsAabbContained(const math::Aabb& aabb) const {
  return math::IsAabbInOrOnPlane(top_face_, aabb) &&
         math::IsAabbInOrOnPlane(bottom_face_, aabb) &&
         math::IsAabbInOrOnPlane(left_face_, aabb) &&
         math::IsAabbInOrOnPlane(right_face_, aabb) &&
         math::IsAabbInOrOnPlane(far_face_, aabb) &&
         math::IsAabbInOrOnPlane(near_face_, aabb);
}

bool Frustum::IsSphereContained(const math::Sphere& sphere) const {
  return math::IsSphereInOrOnPlane(top_face_, sphere) &&
         math::IsSphereInOrOnPlane(bottom_face_, sphere) &&
         math::IsSphereInOrOnPlane(left_face_, sphere) &&
         math::IsSphereInOrOnPlane(right_face_, sphere) &&
         math::IsSphereInOrOnPlane(far_face_, sphere) &&
         math::IsSphereInOrOnPlane(near_face_, sphere);
}

const math::Plane& Frustum::GetTop() const noexcept { return top_face_; }

const math::Plane& Frustum::GetBottom() const noexcept { return bottom_face_; }

const math::Plane& Frustum::GetLeft() const noexcept { return left_face_; }

const math::Plane& Frustum::GetRight() const noexcept { return right_face_; }

const math::Plane& Frustum::GetFar() const noexcept { return far_face_; }

const math::Plane& Frustum::GetNear() const noexcept { return near_face_; }

void Frustum::SetTop(const math::Plane& plane) noexcept { top_face_ = plane; }

void Frustum::SetBottom(const math::Plane& plane) noexcept {
  bottom_face_ = plane;
}

void Frustum::SetLeft(const math::Plane& plane) noexcept { left_face_ = plane; }

void Frustum::SetRight(const math::Plane& plane) noexcept {
  right_face_ = plane;
}

void Frustum::SetFar(const math::Plane& plane) noexcept { far_face_ = plane; }

void Frustum::SetNear(const math::Plane& plane) noexcept { near_face_ = plane; }
}  // namespace rendering
}  // namespace comet
