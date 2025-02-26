// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RESOURCE_TEXTURE_RESOURCE_H_
#define COMET_COMET_RESOURCE_TEXTURE_RESOURCE_H_

#include "comet/core/essentials.h"
#include "comet/core/memory/allocator/allocator.h"
#include "comet/core/memory/allocator/platform_allocator.h"
#include "comet/core/memory/memory.h"
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
  Array<u8> data{};
};

class TextureHandler : public ResourceHandler {
 public:
  TextureHandler(memory::Allocator* loading_resources_allocator,
                 memory::Allocator* loading_resource_allocator);
  TextureHandler(const TextureHandler&) = delete;
  TextureHandler(TextureHandler&&) = delete;
  TextureHandler& operator=(const TextureHandler&) = delete;
  TextureHandler& operator=(TextureHandler&&) = delete;
  virtual ~TextureHandler() = default;

  void Initialize() override;
  void Shutdown() override;

  void Destroy(ResourceId resource_id) override;
  Resource* GetDefaultResource() override;
  Resource* GetDefaultDiffuseTexture();
  Resource* GetDefaultSpecularTexture();
  Resource* GetDefaultNormalTexture();

 protected:
  virtual Resource* GetInternal(ResourceId resource_id) override;
  ResourceFile Pack(memory::Allocator& allocator, const Resource& resource,
                    CompressionMode compression_mode) const override;
  Resource* Unpack(memory::Allocator& allocator,
                   const ResourceFile& file) override;

 private:
  // TODO(m4jr0): Use another allocator;
  memory::PlatformAllocator resource_data_allocator_{
      memory::kEngineMemoryTagResource};
  memory::UniquePtr<TextureResource> default_texture_{nullptr};
  memory::UniquePtr<TextureResource> diffuse_texture_{nullptr};
  memory::UniquePtr<TextureResource> normal_texture_{nullptr};
  memory::UniquePtr<TextureResource> specular_texture_{nullptr};
};
}  // namespace resource
}  // namespace comet

#endif  // COMET_COMET_RESOURCE_TEXTURE_RESOURCE_H_
