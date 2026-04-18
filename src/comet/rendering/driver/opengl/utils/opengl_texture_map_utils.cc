// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "opengl_texture_map_utils.h"
////////////////////////////////////////////////////////////////////////////////

namespace comet {
namespace rendering {
namespace gl {
TextureMap BuildTextureMap(SamplerHandle sampler_handle,
                           TextureHandle texture_handle,
                           resource::TextureResourceId texture_resource_id,
                           TextureType type) {
  TextureMap map{};
  map.sampler_handle = sampler_handle;
  map.texture_handle = texture_handle;
  map.texture_resource_id = texture_resource_id;
  map.type = type;
  return map;
}
}  // namespace gl
}  // namespace rendering
}  // namespace comet