// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RENDERING_DRIVER_VULKAN_DATA_VULKAN_TEXTURE_H_
#define COMET_RENDERING_DRIVER_VULKAN_DATA_VULKAN_TEXTURE_H_

#include "comet/core/essentials.h"
#include "comet/core/type/map.h"
#include "comet/rendering/driver/vulkan/data/vulkan_image.h"
#include "comet/rendering/rendering_common.h"
#include "comet/resource/resource.h"

namespace comet {
namespace rendering {
namespace vk {
struct TextureKey {
  resource::ResourceId resource_id{resource::kInvalidResourceId};
  rendering::TextureType type{rendering::TextureType::Unknown};
};

using TextureId = resource::ResourceId;
constexpr auto kInvalidTextureId{resource::kInvalidResourceId};

HashValue GenerateHash(const TextureKey& key);

struct TextureKeyHashLogic : public MapHashLogic<TextureKey, struct Texture*> {
  using EntryPair = typename MapHashLogic<TextureKey, Texture*>::Value;
  using EntryKey = typename MapHashLogic<TextureKey, Texture*>::Hashable;

  static const EntryKey& GetHashable(const EntryPair& pair) { return pair.key; }

  static HashValue Hash(const EntryKey& key) { return GenerateHash(key); }

  static bool AreEqual(const EntryKey& a, const EntryKey& b) {
    return a.resource_id == b.resource_id && a.type == b.type;
  }
};

struct Texture {
  resource::ResourceId id{resource::kInvalidResourceId};
  rendering::TextureType type{rendering::TextureType::Unknown};
  u32 ref_count{1};
  u32 width{0};
  u32 height{0};
  u32 depth{0};
  u32 mip_levels{1};
  u8 channel_count{0};
  VkFormat format{VK_FORMAT_UNDEFINED};
  Image image{};
};
}  // namespace vk
}  // namespace rendering
}  // namespace comet

#endif  // COMET_RENDERING_DRIVER_VULKAN_DATA_VULKAN_TEXTURE_H_