// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_OPENGL_DATA_OPENGL_MATERIAL_H_
#define COMET_COMET_RENDERING_DRIVER_OPENGL_DATA_OPENGL_MATERIAL_H_

#include "comet/core/essentials.h"
#include "comet/math/vector.h"
#include "comet/rendering/driver/opengl/data/opengl_shader.h"
#include "comet/rendering/driver/opengl/data/opengl_shader_data.h"
#include "comet/rendering/driver/opengl/data/opengl_texture_map.h"

namespace comet {
namespace rendering {
namespace gl {
using MaterialId = stringid::StringId;
constexpr auto kInvalidMaterialId{static_cast<MaterialId>(-1)};

struct MaterialDescr {
  MaterialId id{kInvalidMaterialId};
  ShaderId shader_id{kInvalidShaderId};
  math::Vec4 diffuse_color{kColorWhiteRgba};
  TextureMap diffuse_map{};
  TextureMap specular_map{};
  TextureMap normal_map{};
  f32 shininess{.0f};
};

struct Material {
  f32 shininess{.0f};
  FrameCount instance_update_frame{kInvalidFrameCount};
  MaterialId id{kInvalidMaterialId};
  MaterialInstanceId instance_id{kInvalidMaterialInstanceId};
  ShaderId shader_id{kInvalidShaderId};
  math::Vec4 diffuse_color{kColorWhiteRgba};
  TextureMap diffuse_map{};
  TextureMap specular_map{};
  TextureMap normal_map{};
};
}  // namespace gl
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_OPENGL_DATA_OPENGL_MATERIAL_H_
