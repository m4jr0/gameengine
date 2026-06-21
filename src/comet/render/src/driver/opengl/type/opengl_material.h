// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RENDER_DRIVER_OPENGL_TYPE_OPENGL_MATERIAL_H_
#define COMET_RENDER_DRIVER_OPENGL_TYPE_OPENGL_MATERIAL_H_

#include "comet/core/essentials.h"
#include "comet/core/math/vector.h"
#include "comet/render/driver/opengl/type/opengl_texture_map.h"
#include "comet/render/render_handle.h"
#include "comet/data/render/texture.h"
#include "comet/data/resource/material/material_resource.h"
#include "comet/data/resource/shader/shader_resource.h"

namespace comet {
namespace render {
namespace gl {
struct MaterialDescr {
  resource::MaterialResourceId id{};
  resource::ShaderResourceId shader_resource_id{};
  math::Vec4 diffuse_color{kColorWhiteRgba};
  TextureMap diffuse_map{};
  TextureMap specular_map{};
  TextureMap normal_map{};
  f32 shininess{.0f};
};

struct Material {
  u16 ref_count{0};
  f32 shininess{.0f};
  resource::MaterialResourceId id{};
  resource::ShaderResourceId shader_resource_id{};
  MaterialHandle handle{};
  math::Vec4 diffuse_color{kColorWhiteRgba};
  TextureMap diffuse_map{};
  TextureMap specular_map{};
  TextureMap normal_map{};
};
}  // namespace gl
}  // namespace render
}  // namespace comet

#endif  // COMET_RENDER_DRIVER_OPENGL_TYPE_OPENGL_MATERIAL_H_