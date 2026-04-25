// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_TYPE_LIGHT_H_
#define COMET_COMET_RENDERING_TYPE_LIGHT_H_

#include "comet/core/essentials.h"
#include "comet/entity/type/entity_id.h"
#include "comet/math/vector.h"
#include "comet/rendering/rendering_handle.h"

namespace comet {
namespace rendering {
enum class LightType : u8 { Unknown = 0, Directional, Spot, Point };

enum class ShadowType : u8 {
  None = 0,
  DirectionalOrtho,
  SpotPerspective,
  PointCubemap
};

struct LightShadow {
  bool is_enabled{false};
  f32 max_distance{30.0f};
  f32 bias_constant{.0005f};
  f32 bias_slope{.0025f};

  // For directional CSM only.
  u32 cascade_count{4};
  f32 cascade_lambda{.5f};
};

struct LightProperties {
  LightType type{LightType::Directional};
  math::Vec3 position{.0f};
  math::Vec3 direction{.0f, -1.0f, .0f};
  math::Vec3 color{1.0f};
  f32 intensity{1.0f};
  f32 range{10.0f};
  f32 inner_angle{.0f};
  f32 outer_angle{.0f};
};

struct LightDescr {
  LightProperties props{};
  LightShadow shadow{};
  entity::EntityId entity_id{entity::kInvalidEntityId};
};

struct Light {
  bool is_dirty{true};
  entity::EntityId entity_id{entity::kInvalidEntityId};
  LightHandle handle{};
  LightProperties props{};
  LightShadow shadow{};
};

struct ShadowSettings {
  u32 resolution{2048};
  f32 max_distance{30.0f};
  u32 cascade_count{4};
  f32 cascade_lambda{.5f};
  f32 bias_constant{.0005f};
  f32 bias_slope{.005f};

  f32 caster_extrusion_factor{4.0f};
  f32 receiver_pad_xy{2.0f};
  f32 receiver_pad_z{5.0f};

  f32 cascade_blend_ratio{.10f};

  f32 pcf_radius{1.0f};
  u32 pcf_samples{4};

  bool is_debug_cascades{false};
  s32 debug_single_cascade{-1};
  bool is_blending_disabled{false};
};
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_TYPE_LIGHT_H_
