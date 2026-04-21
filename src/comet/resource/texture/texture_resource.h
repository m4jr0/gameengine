// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RESOURCE_TEXTURE_TEXTURE_RESOURCE_H_
#define COMET_COMET_RESOURCE_TEXTURE_TEXTURE_RESOURCE_H_

#include "comet/core/essentials.h"
#include "comet/core/type/array.h"
#include "comet/rendering/type/rendering_texture_type.h"
#include "comet/resource/resource.h"
#include "comet/resource/resource_id.h"
#include "comet/resource/runtime/loaded_resource_handle.h"

namespace comet {
namespace resource {
struct TextureResourceTag {};

using TextureResourceId = ResourceIdT<TextureResourceTag>;
using TextureResourceHandle = LoadedResourceHandle<TextureResourceTag>;

inline constexpr TextureResourceId kDefaultDiffuseTextureId{1};
inline constexpr TextureResourceId kDefaultSpecularTextureId{2};
inline constexpr TextureResourceId kDefaultNormalTextureId{3};

TextureResourceId GetDefaultTextureFromType(
    rendering::TextureType texture_type);

struct TextureResourceDescr {
  u64 size{0};
  rendering::TextureFormat format{rendering::TextureFormat::Unknown};
  u32 resolution[3]{0, 0, 0};
  u8 channel_count{0};
};

struct TextureResource : Resource {
  using Id = TextureResourceId;
  using Handle = TextureResourceHandle;
  using TypeId = ResourceTypeId;
  using TypeName = ResourceTypeName;

  static constexpr TypeName kResourceTypeName{"texture"};
  static const TypeId kResourceTypeId;

  TextureResourceDescr descr{};
  Array<u8> data{};

  Id GetId() const noexcept;
};

usize GetTextureResourceSize(const TextureResource& resource);
}  // namespace resource
}  // namespace comet

#endif  // COMET_COMET_RESOURCE_TEXTURE_TEXTURE_RESOURCE_H_
