// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_OPENGL_TYPE_OPENGL_TEXTURE_MAP_H_
#define COMET_COMET_RENDERING_DRIVER_OPENGL_TYPE_OPENGL_TEXTURE_MAP_H_

// External. ///////////////////////////////////////////////////////////////////
#include "glad/glad.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/essentials.h"
#include "comet/rendering/rendering_handle.h"
#include "comet/rendering/type/texture.h"
#include "comet/resource/texture/texture_resource.h"

namespace comet {
namespace rendering {
namespace gl {
struct TextureMap {
  SamplerHandle sampler_handle{};
  TextureHandle texture_handle{};
  resource::TextureResourceId texture_resource_id{};
  TextureType type{TextureType::Unknown};
};
}  // namespace gl
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_OPENGL_TYPE_OPENGL_TEXTURE_MAP_H_