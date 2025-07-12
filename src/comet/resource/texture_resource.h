// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RESOURCE_TEXTURE_RESOURCE_H_
#define COMET_COMET_RESOURCE_TEXTURE_RESOURCE_H_

#include "comet/core/essentials.h"
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

struct TextureResource : Resource {
  static const ResourceTypeId kResourceTypeId;

  TextureResourceDescr descr{};
  Array<u8> data{};
};
}  // namespace resource
}  // namespace comet

#endif  // COMET_COMET_RESOURCE_TEXTURE_RESOURCE_H_
