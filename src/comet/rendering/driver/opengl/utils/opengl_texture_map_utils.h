// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_OPENGL_UTILS_OPENGL_TEXTURE_MAP_UTILS_H_
#define COMET_COMET_RENDERING_DRIVER_OPENGL_UTILS_OPENGL_TEXTURE_MAP_UTILS_H_

#include "comet/rendering/driver/opengl/data/opengl_texture_map.h"

namespace comet {
namespace rendering {
namespace gl {
TextureMap BuildTextureMap(SamplerHandle sampler_handle,
                           TextureHandle texture_handle,
                           resource::TextureResourceId texture_resource_id =
                               resource::TextureResourceId::Invalid(),
                           TextureType type = TextureType::Unknown);
}  // namespace gl
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_OPENGL_UTILS_OPENGL_TEXTURE_MAP_UTILS_H_