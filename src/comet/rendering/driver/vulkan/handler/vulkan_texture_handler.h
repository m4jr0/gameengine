// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_VULKAN_HANDLER_VULKAN_TEXTURE_HANDLER_H_
#define COMET_COMET_RENDERING_DRIVER_VULKAN_HANDLER_VULKAN_TEXTURE_HANDLER_H_

#include "comet/core/essentials.h"
#include "comet/core/memory/allocator/free_list_allocator.h"
#include "comet/core/memory/memory.h"
#include "comet/core/type/map.h"
#include "comet/rendering/driver/vulkan/data/vulkan_texture.h"
#include "comet/rendering/driver/vulkan/handler/vulkan_handler.h"
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
  virtual ~TextureHandler() = default;

  void Initialize() override;
  void Shutdown() override;

  const Texture* Generate(const resource::TextureResource* resource);
  const Texture* Generate(const resource::TextureResource* resource,
                          TextureType type);

  const Texture* Get(TextureId texture_id) const;
  const Texture* Get(TextureId texture_id, TextureType type) const;

  const Texture* TryGet(TextureId texture_id) const;
  const Texture* TryGet(TextureId texture_id, TextureType type) const;

  const Texture* GetOrGenerate(const resource::TextureResource* resource);
  const Texture* GetOrGenerate(const resource::TextureResource* resource,
                               TextureType type);

  void Destroy(TextureId texture_id);
  void Destroy(TextureId texture_id, TextureType type);
  void Destroy(Texture* texture);

 private:
  struct TextureKey {
    TextureId id{kInvalidTextureId};
    TextureType type{TextureType::Unknown};
  };

  struct TextureKeyHashLogic : public MapHashLogic<TextureKey, Texture*> {
    using EntryPair = typename MapHashLogic<TextureKey, Texture*>::Value;
    using EntryKey = typename MapHashLogic<TextureKey, Texture*>::Hashable;

    static const EntryKey& GetHashable(const EntryPair& pair) {
      return pair.key;
    }

    static HashValue Hash(const EntryKey& key) {
      HashValue hash{0};
      hash = HashCombine(hash, static_cast<HashValue>(key.id));
      hash = HashCombine(hash, static_cast<u32>(key.type));
      return hash;
    }

    static bool AreEqual(const EntryKey& a, const EntryKey& b) {
      return a.id == b.id && a.type == b.type;
    }
  };

  Texture* Get(TextureId texture_id);
  Texture* Get(TextureId texture_id, TextureType type);

  Texture* TryGet(TextureId texture_id);
  Texture* TryGet(TextureId texture_id, TextureType type);

  void Destroy(Texture* texture, bool is_destroying_handler);

  static u32 GetMipLevels(const resource::TextureResource* resource);
  static bool IsSrgbTextureType(TextureType type);
  static VkFormat GetVkFormat(const resource::TextureResource* resource,
                              TextureType type);
  static u8 GetResolvedChannelCount(const resource::TextureResource* resource);

  void GenerateMipmaps(const Texture* texture) const;
  Texture* GenerateInstance(const resource::TextureResource* resource,
                            TextureType type);

  memory::FiberFreeListAllocator allocator_{sizeof(Texture), 256,
                                            memory::kEngineMemoryTagRendering};
  Map<TextureKey, Texture*, TextureKeyHashLogic> textures_{};
};
}  // namespace vk
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_VULKAN_HANDLER_VULKAN_TEXTURE_HANDLER_H_