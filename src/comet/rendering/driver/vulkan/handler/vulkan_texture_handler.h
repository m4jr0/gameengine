// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_VULKAN_HANDLER_VULKAN_TEXTURE_HANDLER_H_
#define COMET_COMET_RENDERING_DRIVER_VULKAN_HANDLER_VULKAN_TEXTURE_HANDLER_H_

#include "comet/core/essentials.h"
#include "comet/core/memory/allocator/free_list_allocator.h"
#include "comet/core/type/shared_instance_registry.h"
#include "comet/rendering/driver/vulkan/data/vulkan_texture.h"
#include "comet/rendering/driver/vulkan/handler/vulkan_handler.h"
#include "comet/rendering/rendering_handle.h"
#include "comet/resource/texture_resource.h"

namespace comet {
namespace rendering {
namespace vk {
using TextureHandlerDescr = HandlerDescr;

class TextureHandler : public Handler {
 public:
  TextureHandler() = delete;
  explicit TextureHandler(const TextureHandlerDescr& descr);
  TextureHandler(const TextureHandler&) = delete;
  TextureHandler(TextureHandler&&) = delete;
  TextureHandler& operator=(const TextureHandler&) = delete;
  TextureHandler& operator=(TextureHandler&&) = delete;
  ~TextureHandler() override = default;

  TextureHandle GetOrGenerate(resource::TextureResourceId texture_resource_id);
  TextureHandle GetOrGenerate(resource::TextureResourceId texture_resource_id,
                              TextureType type);
  TextureHandle Generate(const RuntimeTextureDescr& descr);

  void Destroy(TextureHandle handle);

  const Texture* Get(TextureHandle handle) const;

 protected:
  void OnInitialize() override;
  void OnShutdown() override;

 private:
  Texture* Get(TextureHandle handle);

  Texture* GenerateTexture(const resource::TextureResource* resource,
                           TextureType type);
  void DestroyTexture(Texture* texture);
  void GenerateMipmaps(Texture* texture) const;

  RuntimeTextureId next_runtime_texture_id_{0};

  memory::PlatformAllocator cache_allocator_{memory::kEngineMemoryTagRendering};

  memory::FiberFreeListAllocator allocator_{sizeof(Texture), 256,
                                            memory::kEngineMemoryTagRendering};

  SharedInstanceRegistry<TextureKey, TextureTag, Texture> textures_{};
};
}  // namespace vk
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_VULKAN_HANDLER_VULKAN_TEXTURE_HANDLER_H_