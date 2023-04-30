// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RESOURCE_MATERIAL_RESOURCE_H_
#define COMET_COMET_RESOURCE_MATERIAL_RESOURCE_H_

#include "comet_precompile.h"

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

struct MaterialResourceDescr {
  f32 shininess{.0f};
  math::Vec4 diffuse_color{};
  TextureMap diffuse_map{};
  TextureMap specular_map{};
  TextureMap normal_map{};
  ResourceId shader_id{kInvalidResourceId};
};

struct MaterialResource : public Resource {
  static const ResourceTypeId kResourceTypeId;

  MaterialResourceDescr descr{};
};

ResourceId GenerateMaterialId(const std::string& material_name);
ResourceId GenerateMaterialId(const schar* material_name);

class MaterialHandler : public ResourceHandler {
 public:
  MaterialHandler() = default;
  MaterialHandler(const MaterialHandler&) = delete;
  MaterialHandler(MaterialHandler&&) = delete;
  MaterialHandler& operator=(const MaterialHandler&) = delete;
  MaterialHandler& operator=(MaterialHandler&&) = delete;
  virtual ~MaterialHandler() = default;

  const Resource* GetDefaultResource() override;

 protected:
  ResourceFile Pack(const Resource& resource,
                    CompressionMode compression_mode) const override;
  std::unique_ptr<Resource> Unpack(const ResourceFile& file) const override;

 private:
  std::unique_ptr<MaterialResource> default_material_{nullptr};
};
}  // namespace resource
}  // namespace comet

#endif  // COMET_COMET_RESOURCE_MATERIAL_RESOURCE_H_
