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

namespace comet {
namespace resource {
const ResourceTypeId TextureResource::kResourceTypeId{
    COMET_STRING_ID("texture")};

ResourceId GetDefaultTextureFromType(rendering::TextureType texture_type) {
  switch (texture_type) {
    case rendering::TextureType::Diffuse:
      return kDefaultDiffuseTextureResourceId;
    case rendering::TextureType::Specular:
      return kDefaultSpecularTextureResourceId;
    case rendering::TextureType::Normal:
      return kDefaultNormalTextureResourceId;
    default:
      COMET_ASSERT(false, "Unknown or unsupported texture type provided: ",
                   static_cast<std::underlying_type_t<rendering::TextureType>>(
                       texture_type),
                   "!");
      return kDefaultResourceId;
  }
}
}  // namespace resource
}  // namespace comet
