// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RENDER_DRIVER_VULKAN_HANDLER_VULKAN_TEXTURE_HANDLER_H_
#define COMET_RENDER_DRIVER_VULKAN_HANDLER_VULKAN_TEXTURE_HANDLER_H_

#include "comet/core/essentials.h"
#include "comet/runtime/memory/allocator/free_list_allocator.h"
#include "comet/runtime/memory/allocator/platform_allocator.h"
#include "comet/runtime/shared_instance_registry.h"
#include "comet/render/driver/vulkan/handler/vulkan_handler.h"
#include "comet/render/driver/vulkan/type/vulkan_texture.h"
#include "comet/render/render_handle.h"
#include "comet/resource/texture/texture_resource.h"

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

  TextureHandle GenerateRuntimeDeferred(const RuntimeTextureDescr& descr);
  TextureHandle GenerateRuntimeImmediate(const RuntimeTextureDescr& descr);

  void Destroy(TextureHandle handle);

  const Texture* Get(TextureHandle handle) const;

  // Kept for compatibility with existing driver code. Textures no longer keep
  // deferred upload resources tied to the upload queue.
  void ReleasePendingUploadResources(FrameInFlightIndex frame);

 protected:
  void OnInitialize() override;
  void OnShutdown() override;

 private:
  Texture* Get(TextureHandle handle);

  Texture* GenerateTexture(const resource::TextureResource* resource,
                           TextureType type);
  Texture* GenerateRuntimeTexture(const RuntimeTextureDescr& descr);

  TextureHandle RegisterTexture(Texture* texture);

  void DestroyTexture(Texture* texture);

  RuntimeTextureId next_runtime_texture_id_{0};

  memory::PlatformAllocator cache_allocator_{memory::kEngineMemoryTagRender};

  memory::FiberFreeListAllocator allocator_{sizeof(Texture), 256,
                                            memory::kEngineMemoryTagRender};

  SharedInstanceRegistry<TextureKey, TextureTag, Texture> textures_{};
};
}  // namespace vk
}  // namespace rendering
}  // namespace comet

#endif  // COMET_RENDER_DRIVER_VULKAN_HANDLER_VULKAN_TEXTURE_HANDLER_H_