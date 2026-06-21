// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RUNTIME_CAMERA_CAMERA_H_
#define COMET_RUNTIME_CAMERA_CAMERA_H_

#include "comet/core/essentials.h"
#include "comet/core/math/geometry.h"
#include "comet/core/math/matrix.h"
#include "comet/core/math/vector.h"
#include "comet/runtime/camera/camera_handle.h"

namespace comet {
namespace camera {
enum class ClipSpaceDepthRange : u8 { MinusOneToOne, ZeroToOne };

struct CameraViewData {
  math::Mat4 projection_matrix{1.0f};
  math::Mat4 view_matrix{1.0f};
  math::Vec3 view_position{.0f};
  f32 near_plane{.05f};
  math::Vec3 front{.0f, .0f, -1.0f};
  f32 far_plane{1000.0f};
  math::Vec3 up{.0f, 1.0f, .0f};
  f32 fov_y_radians{.0f};
  math::Vec3 right{1.0f, .0f, .0f};
  f32 aspect_ratio{1.0f};
};

#ifdef COMET_DEBUG
constexpr usize kCameraDebugLabelCapacity{32};
#endif  // COMET_DEBUG

struct CameraPose {
  math::Vec3 position{.0f};
  math::Quat rotation{1.0f, .0f, .0f, .0f};
};

struct CameraDescr {
  CameraPose pose{};
  CameraPose reset_pose{pose};
  f32 fov{45.0f};
  f32 z_near{.1f};
  f32 z_far{1000.0f};
#ifdef COMET_DEBUG
  const schar* debug_label{nullptr};
#endif  // COMET_DEBUG
};

struct ViewportRect {
  f32 x{.0f};
  f32 y{.0f};
  f32 width{.0f};
  f32 height{.0f};
};

enum class CameraKind : u8 {
  Unknown = 0,
  Game,
  Debug,
};

using CameraFlags = u8;

enum CameraFlagBits : CameraFlags {
  kCameraFlagBitsNone = 0x0,
  kCameraFlagBitsGame = 0x1,
  kCameraFlagBitsDebug = 0x2,
  kCameraFlagBitsMain = 0x4,
  kCameraFlagBitsAll = static_cast<CameraFlags>(-1)
};

using CameraProjectionFlags = u8;

enum ProjectionFlagBits : CameraProjectionFlags {
  kProjectionFlagBitsNone = 0x0,
  kProjectionFlagBitsInvertY = 0x1,
  kProjectionFlagBitsDepthZeroToOne = 0x2,
};

struct CameraProjectionDescr {
  f32 fov_y_radians{math::ConvertToRadians(45.0f)};
  f32 near_plane{0.1f};
  f32 far_plane{1000.0f};
  f32 aspect_ratio{1.0f};
  CameraProjectionFlags flags{kProjectionFlagBitsDepthZeroToOne};
};

struct CameraView {
  CameraHandle camera{};
  CameraKind kind{CameraKind::Unknown};
  CameraFlags flags{kCameraFlagBitsNone};
  bool is_debug_draw_enabled{false};
  ViewportRect viewport{};
  CameraViewData data{};

  inline bool IsMain() const noexcept {
    return (flags & kCameraFlagBitsMain) != 0;
  }
};
}  // namespace camera
}  // namespace comet

#endif  // COMET_RUNTIME_CAMERA_CAMERA_H_