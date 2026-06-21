// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RUNTIME_RESOURCE_TEXTURE_TEXTURE_RESOURCE_HANDLER_H_
#define COMET_RUNTIME_RESOURCE_TEXTURE_TEXTURE_RESOURCE_HANDLER_H_

#include "comet/core/essentials.h"
#include "comet/runtime/memory/allocator/platform_allocator.h"
#include "comet/core/memory/memory.h"
#include "comet/runtime/memory/memory_tag.h"
#include "comet/runtime/resource/resource_handler.h"
#include "comet/data/resource/resource_file.h"
#include "comet/data/resource/resource.h"
#include "comet/data/resource/texture/texture_resource.h"
#include "comet/data/resource/common.h"

namespace comet {
namespace resource {
class TextureResourceHandler
    : public ResourceHandler<TextureResourceTag, TextureResource> {
 public:
  using Base = ResourceHandler;

  explicit TextureResourceHandler(const ResourceHandlerDescr& descr);
  TextureResourceHandler(const TextureResourceHandler&) = delete;
  TextureResourceHandler(TextureResourceHandler&&) = delete;
  TextureResourceHandler& operator=(const TextureResourceHandler&) = delete;
  TextureResourceHandler& operator=(TextureResourceHandler&&) = delete;
  ~TextureResourceHandler() override = default;

  ResourceFile Pack(const TextureResource& resource,
                    CompressionMode compression_mode) override;
  void Unpack(const ResourceFile& file, ResourceLifeSpan life_span,
              TextureResource* resource) override;

  TextureResource* GetDefaultTextureResource();
  TextureResource* GetDefaultDiffuseTextureResource();
  TextureResource* GetDefaultSpecularTextureResource();
  TextureResource* GetDefaultNormalTextureResource();

 protected:
  void InitializeDefaults() override;
  void DestroyDefaults() override;

 private:
  memory::PlatformAllocator resource_data_allocator_{
      kEngineMemoryTagResourceTexture};
  memory::UniquePtr<TextureResource> default_texture_{nullptr};
  memory::UniquePtr<TextureResource> diffuse_texture_{nullptr};
  memory::UniquePtr<TextureResource> normal_texture_{nullptr};
  memory::UniquePtr<TextureResource> specular_texture_{nullptr};
};
}  // namespace resource
}  // namespace comet

#endif  // COMET_RUNTIME_RESOURCE_TEXTURE_TEXTURE_RESOURCE_HANDLER_H_