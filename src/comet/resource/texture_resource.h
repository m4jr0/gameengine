// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RESOURCE_TEXTURE_RESOURCE_H_
#define COMET_COMET_RESOURCE_TEXTURE_RESOURCE_H_

#include "comet_precompile.h"

#include "comet/rendering/rendering_common.h"
#include "comet/resource/resource.h"

namespace comet {
namespace resource {
constexpr ResourceId kDefaultDiffuseTextureResourceId{1};
constexpr ResourceId kDefaultSpecularTextureResourceId{2};
constexpr ResourceId kDefaultNormalTextureResourceId{3};

ResourceId GetDefaultTextureFromType(rendering::TextureType texture_type);

struct TextureResourceDescr {
  u64 size{0};
  rendering::TextureFormat format{rendering::TextureFormat::Unknown};
  u32 resolution[3]{0, 0, 0};
  u8 channel_count{0};
};

struct TextureResource : public Resource {
  static const ResourceTypeId kResourceTypeId;

  TextureResourceDescr descr{};
  std::vector<u8> data{};
};

class TextureHandler : public ResourceHandler {
 public:
  TextureHandler() = default;
  TextureHandler(const TextureHandler&) = delete;
  TextureHandler(TextureHandler&&) = delete;
  TextureHandler& operator=(const TextureHandler&) = delete;
  TextureHandler& operator=(TextureHandler&&) = delete;
  virtual ~TextureHandler() = default;

  const Resource* Get(ResourceId resource_id) override;
  const Resource* GetDefaultResource() override;
  const Resource* GetDefaultDiffuseTexture();
  const Resource* GetDefaultSpecularTexture();
  const Resource* GetDefaultNormalTexture();

 protected:
  ResourceFile Pack(const Resource& resource,
                    CompressionMode compression_mode) const override;
  std::unique_ptr<Resource> Unpack(const ResourceFile& file) const override;

 private:
  std::unique_ptr<TextureResource> default_texture_{nullptr};
  std::unique_ptr<TextureResource> diffuse_texture_{nullptr};
  std::unique_ptr<TextureResource> normal_texture_{nullptr};
  std::unique_ptr<TextureResource> specular_texture_{nullptr};
};
}  // namespace resource
}  // namespace comet

#endif  // COMET_COMET_RESOURCE_TEXTURE_RESOURCE_H_
