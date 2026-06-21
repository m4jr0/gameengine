// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_data_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/data/resource/texture/texture_resource.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/id/string_id.h"
#include "comet/core/id/string_id_allocator.h"
#include "comet/core/type_trait.h"
#include "comet/data/render/label/texture_label.h"

namespace comet {
namespace resource {
const TextureResource::TypeId TextureResource::kResourceTypeId{
    COMET_STRING_ID(TextureResource::kResourceTypeName.data())};

TextureResource::Id TextureResource::GetId() const noexcept { return Id{id}; }

TextureResourceId GetDefaultTextureFromType(
    render::TextureType texture_type) {
  switch (texture_type) {
    case render::TextureType::Diffuse:
      return kDefaultDiffuseTextureId;
    case render::TextureType::Specular:
      return kDefaultSpecularTextureId;
    case render::TextureType::Normal:
      return kDefaultNormalTextureId;
    default:
      COMET_ASSERT(false, "texture_resource::GetDefaultTextureFromType",
                   "unsupported texture type", "texture_type",
                   render::GetTextureTypeLabel(texture_type),
                   "texture_type_value", ToUnderlying(texture_type));
      return TextureResourceId{kFallbackRawResourceId};
  }
}

usize GetTextureResourceSize(const TextureResource& resource) {
  return sizeof(RawResourceId) + sizeof(ResourceTypeId) +
         resource.data.GetSize();
}
}  // namespace resource
}  // namespace comet