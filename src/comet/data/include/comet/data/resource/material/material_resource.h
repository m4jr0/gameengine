// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_DATA_RESOURCE_MATERIAL_MATERIAL_RESOURCE_H_
#define COMET_DATA_RESOURCE_MATERIAL_MATERIAL_RESOURCE_H_

#include "comet/core/essentials.h"
#include "comet/core/string/tstring.h"
#include "comet/core/math/vector.h"
#include "comet/data/resource/resource_file.h"
#include "comet/data/resource/resource.h"
#include "comet/data/resource/runtime/loaded_resource_handle.h"
#include "comet/data/resource/shader/shader_resource.h"
#include "comet/data/resource/texture/texture_resource.h"
#include "comet/data/resource/common.h"

namespace comet {
namespace resource {
struct MaterialResourceTag {};

using MaterialResourceId = ResourceIdT<MaterialResourceTag>;

MaterialResourceId GenerateMaterialId(const schar* qualified_name);
MaterialResourceId GenerateMaterialId(const wchar* qualified_name);
MaterialResourceId GenerateMaterialId(CTStringView qualified_name);

MaterialResourceId GenerateQualifiedMaterialId(CTStringView file_path,
                                               const schar* material_name,
                                               u32 material_index);

struct TextureMapResource {
  TextureResourceId texture_resource_id{};
  render::TextureType type{render::TextureType::Unknown};
  render::TextureRepeatMode u_repeat_mode{
      render::TextureRepeatMode::Unknown};
  render::TextureRepeatMode v_repeat_mode{
      render::TextureRepeatMode::Unknown};
  render::TextureRepeatMode w_repeat_mode{
      render::TextureRepeatMode::Unknown};
  render::TextureFilterMode min_filter_mode{
      render::TextureFilterMode::Unknown};
  render::TextureFilterMode mag_filter_mode{
      render::TextureFilterMode::Unknown};
};

struct MaterialResourceDescr {
  f32 shininess{.0f};
  ShaderResourceId shader_resource_id{};
  math::Vec4 diffuse_color{};
  TextureMapResource diffuse_map{.type = render::TextureType::Diffuse};
  TextureMapResource specular_map{.type = render::TextureType::Specular};
  TextureMapResource normal_map{.type = render::TextureType::Normal};
};

struct MaterialResource : Resource {
  using Id = MaterialResourceId;
  using TypeId = ResourceTypeId;
  using TypeName = ResourceTypeName;

  static constexpr TypeName kResourceTypeName{"material"};
  static const TypeId kResourceTypeId;

  MaterialResourceDescr descr{};

  Id GetId() const noexcept;
};

inline constexpr MaterialResource::Id GetDefaultMaterialId() noexcept {
  return MaterialResource::Id{kFallbackRawResourceId};
}

usize GetMaterialResourceSize(const MaterialResource& resource);
}  // namespace resource
}  // namespace comet

#endif  // COMET_DATA_RESOURCE_MATERIAL_MATERIAL_RESOURCE_H_