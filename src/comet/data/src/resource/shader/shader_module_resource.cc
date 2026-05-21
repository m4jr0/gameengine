// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/data/resource/shader/shader_module_resource.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/type/string_id.h"

namespace comet {
namespace resource {
const ShaderModuleResource::TypeId ShaderModuleResource::kResourceTypeId{
    COMET_STRING_ID(ShaderModuleResource::kResourceTypeName.data())};

ShaderModuleResource::Id ShaderModuleResource::GetId() const noexcept {
  return Id{id};
}

usize GetShaderModuleResourceSize(const ShaderModuleResource& resource) {
  return sizeof(RawResourceId) + sizeof(ResourceTypeId) +
         resource.data.GetSize();
}
}  // namespace resource
}  // namespace comet
