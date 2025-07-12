// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RESOURCE_MATERIAL_RESOURCE_H_
#define COMET_COMET_RESOURCE_MATERIAL_RESOURCE_H_

#include "comet/core/essentials.h"
#include "comet/math/vector.h"
#include "comet/rendering/rendering_common.h"
#include "comet/resource/resource.h"

namespace comet {
namespace resource {
struct TextureMap {
  ResourceId texture_id{kInvalidResourceId};
  rendering::TextureType type{rendering::TextureType::Unknown};
  rendering::TextureRepeatMode u_repeat_mode{
      rendering::TextureRepeatMode::Unknown};
  rendering::TextureRepeatMode v_repeat_mode{
      rendering::TextureRepeatMode::Unknown};
  rendering::TextureRepeatMode w_repeat_mode{
      rendering::TextureRepeatMode::Unknown};
  rendering::TextureFilterMode min_filter_mode{
      rendering::TextureFilterMode::Unknown};
  rendering::TextureFilterMode mag_filter_mode{
      rendering::TextureFilterMode::Unknown};
};

constexpr auto kMaxShaderNameLen{128};

struct MaterialResourceDescr {
  f32 shininess{.0f};
  math::Vec4 diffuse_color{};
  TextureMap diffuse_map{};
  TextureMap specular_map{};
  TextureMap normal_map{};
  schar shader_name[kMaxShaderNameLen]{0};
};

struct MaterialResource : Resource {
  static const ResourceTypeId kResourceTypeId;

  MaterialResourceDescr descr{};
};

ResourceId GenerateMaterialId(const schar* material_name);
}  // namespace resource
}  // namespace comet

#endif  // COMET_COMET_RESOURCE_MATERIAL_RESOURCE_H_
