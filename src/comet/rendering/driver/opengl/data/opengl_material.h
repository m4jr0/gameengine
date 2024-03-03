// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_OPENGL_DATA_OPENGL_MATERIAL_H_
#define COMET_COMET_RENDERING_DRIVER_OPENGL_DATA_OPENGL_MATERIAL_H_

#include "comet_precompile.h"

#include "comet/math/vector.h"
#include "comet/rendering/driver/opengl/data/opengl_frame.h"
#include "comet/rendering/driver/opengl/data/opengl_shader.h"
#include "comet/rendering/driver/opengl/data/opengl_texture.h"
#include "comet/rendering/driver/opengl/data/opengl_texture_map.h"
#include "comet/rendering/rendering_common.h"

namespace comet {
namespace rendering {
namespace gl {
using MaterialId = stringid::StringId;
constexpr auto kInvalidMaterialId{static_cast<MaterialId>(-1)};

struct MaterialDescr {
  MaterialId id{kInvalidMaterialId};
  ShaderId shader_id{kInvalidShaderId};
  math::Vec4 diffuse_color{kColorWhite, 1.0f};
  TextureMap diffuse_map{};
  TextureMap specular_map{};
  TextureMap normal_map{};
  f32 shininess{.0f};
};

struct Material {
  f32 shininess{.0f};
  FrameIndex instance_update_frame{kInvalidFrameIndex};
  MaterialId id{kInvalidMaterialId};
  MaterialInstanceId instance_id{kInvalidMaterialInstanceId};
  ShaderId shader_id{kInvalidShaderId};
  math::Vec4 diffuse_color{kColorWhite, 1.0f};
  TextureMap diffuse_map{};
  TextureMap specular_map{};
  TextureMap normal_map{};
};
}  // namespace gl
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_OPENGL_DATA_OPENGL_MATERIAL_H_
