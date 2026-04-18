// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RESOURCE_MATERIAL_RESOURCE_H_
#define COMET_COMET_RESOURCE_MATERIAL_RESOURCE_H_

#include "comet/core/essentials.h"
#include "comet/core/type/tstring.h"
#include "comet/math/vector.h"
#include "comet/rendering/rendering_type.h"
#include "comet/resource/resource.h"
#include "comet/resource/resource_type.h"
#include "comet/resource/runtime/loaded_resource_handle.h"
#include "comet/resource/shader_resource.h"
#include "comet/resource/texture_resource.h"

namespace comet {
namespace resource {
struct MaterialResourceTag {};

using MaterialResourceId = ResourceIdT<MaterialResourceTag>;
using MaterialResourceHandle = LoadedResourceHandle<MaterialResourceTag>;

MaterialResourceId GenerateMaterialId(const schar* qualified_name);
MaterialResourceId GenerateMaterialId(const wchar* qualified_name);
MaterialResourceId GenerateMaterialId(CTStringView qualified_name);

MaterialResourceId GenerateQualifiedMaterialId(CTStringView file_path,
                                               const schar* material_name,
                                               u32 material_index);

struct TextureMapResource {
  TextureResourceId texture_resource_id{};
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

struct MaterialResourceDescr {
  f32 shininess{.0f};
  ShaderResourceId shader_resource_id{};
  math::Vec4 diffuse_color{};
  TextureMapResource diffuse_map{.type = rendering::TextureType::Diffuse};
  TextureMapResource specular_map{.type = rendering::TextureType::Specular};
  TextureMapResource normal_map{.type = rendering::TextureType::Normal};
};

struct MaterialResource : Resource {
  using Id = MaterialResourceId;
  using Handle = MaterialResourceHandle;
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

#endif  // COMET_COMET_RESOURCE_MATERIAL_RESOURCE_H_