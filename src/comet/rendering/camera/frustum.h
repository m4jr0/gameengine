// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_CAMERA_FRUSTUM_H_
#define COMET_COMET_RENDERING_CAMERA_FRUSTUM_H_

#include "comet/core/essentials.h"
#include "comet/math/bounding_volume.h"
#include "comet/math/plane.h"

namespace comet {
namespace rendering {
class Frustum {
 public:
  Frustum() = default;
  Frustum(const math::Plane& top_face, const math::Plane& bottom_face,
          const math::Plane& left_face, const math::Plane& right_face,
          const math::Plane& far_face, const math::Plane& near_face);
  Frustum(const Frustum&) = default;
  Frustum(Frustum&&) = default;
  Frustum& operator=(const Frustum&) = default;
  Frustum& operator=(Frustum&&) = default;
  ~Frustum() = default;

  bool IsAabbContained(const math::Aabb& aabb) const;
  bool IsSphereContained(const math::Sphere& sphere) const;
  const math::Plane& GetTop() const noexcept;
  const math::Plane& GetBottom() const noexcept;
  const math::Plane& GetLeft() const noexcept;
  const math::Plane& GetRight() const noexcept;
  const math::Plane& GetFar() const noexcept;
  const math::Plane& GetNear() const noexcept;
  void SetTop(const math::Plane& plane) noexcept;
  void SetBottom(const math::Plane& plane) noexcept;
  void SetLeft(const math::Plane& plane) noexcept;
  void SetRight(const math::Plane& plane) noexcept;
  void SetFar(const math::Plane& plane) noexcept;
  void SetNear(const math::Plane& plane) noexcept;

 private:
  math::Plane top_face_{};
  math::Plane bottom_face_{};
  math::Plane left_face_{};
  math::Plane right_face_{};
  math::Plane far_face_{};
  math::Plane near_face_{};
};
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_CAMERA_FRUSTUM_H_
