// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RESOURCE_HANDLER_TEXTURE_RESOURCE_HANDLER_H_
#define COMET_COMET_RESOURCE_HANDLER_TEXTURE_RESOURCE_HANDLER_H_

#include "comet/core/essentials.h"
#include "comet/core/memory/allocator/platform_allocator.h"
#include "comet/core/memory/memory.h"
#include "comet/resource/handler/resource_handler.h"
#include "comet/resource/resource.h"
#include "comet/resource/texture_resource.h"

namespace comet {
namespace resource {
class TextureResourceHandler : public ResourceHandler<TextureResource> {
 public:
  TextureResourceHandler(const ResourceHandlerDescr& descr);
  TextureResourceHandler(const TextureResourceHandler&) = delete;
  TextureResourceHandler(TextureResourceHandler&&) = delete;
  TextureResourceHandler& operator=(const TextureResourceHandler&) = delete;
  TextureResourceHandler& operator=(TextureResourceHandler&&) = delete;
  virtual ~TextureResourceHandler() = default;

  void InitializeDefaults() override;
  void DestroyDefaults() override;

  ResourceFile Pack(const TextureResource& resource,
                    CompressionMode compression_mode) override;
  void Unpack(const ResourceFile& file, ResourceLifeSpan life_span,
              TextureResource* resource) override;

  TextureResource* GetDefaultTextureResource();
  TextureResource* GetDefaultDiffuseTextureResource();
  TextureResource* GetDefaultSpecularTextureResource();
  TextureResource* GetDefaultNormalTextureResource();

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

#endif  // COMET_COMET_RESOURCE_HANDLER_TEXTURE_RESOURCE_HANDLER_H_
