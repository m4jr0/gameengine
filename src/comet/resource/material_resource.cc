// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "material_resource.h"
////////////////////////////////////////////////////////////////////////////////

namespace comet {
namespace resource {
const ResourceTypeId MaterialResource::kResourceTypeId{
    COMET_STRING_ID("material")};

ResourceId GenerateMaterialId(const schar* material_name) {
  return COMET_STRING_ID(material_name);
}
}  // namespace resource
}  // namespace comet
