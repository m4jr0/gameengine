// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "texture_resource.h"
////////////////////////////////////////////////////////////////////////////////

// External. ///////////////////////////////////////////////////////////////////
#include <type_traits>
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/type/string_id.h"

namespace comet {
namespace resource {
const TextureResource::TypeId TextureResource::kResourceTypeId{
    COMET_STRING_ID(TextureResource::kResourceTypeName.data())};

TextureResource::Id TextureResource::GetId() const noexcept { return Id{id}; }

TextureResourceId GetDefaultTextureFromType(
    rendering::TextureType texture_type) {
  switch (texture_type) {
    case rendering::TextureType::Diffuse:
      return kDefaultDiffuseTextureId;
    case rendering::TextureType::Specular:
      return kDefaultSpecularTextureId;
    case rendering::TextureType::Normal:
      return kDefaultNormalTextureId;
    default:
      COMET_ASSERT(false, "Unknown or unsupported texture type provided: ",
                   static_cast<std::underlying_type_t<rendering::TextureType>>(
                       texture_type),
                   "!");
      return TextureResourceId{kFallbackRawResourceId};
  }
}

usize GetTextureResourceSize(const TextureResource& resource) {
  return sizeof(RawResourceId) + sizeof(ResourceTypeId) +
         resource.data.GetSize();
}
}  // namespace resource
}  // namespace comet