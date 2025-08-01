// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_OPENGL_DATA_OPENGL_TEXTURE_MAP_H_
#define COMET_COMET_RENDERING_DRIVER_OPENGL_DATA_OPENGL_TEXTURE_MAP_H_

#include "comet/core/essentials.h"
#include "comet/rendering/driver/opengl/data/opengl_texture.h"
#include "comet/resource/resource.h"

namespace comet {
namespace rendering {
namespace gl {
using RepeatMode = u32;
constexpr auto kInvalidRepeatMode{static_cast<RepeatMode>(-1)};

using FilterMode = u32;
constexpr auto kInvalidFilterMode{static_cast<FilterMode>(-1)};

constexpr auto kMaxTextureLabelSize{32};

enum TextureType { Invalid = -1, Diffuse = 0, Specular = 1, Normal = 2 };

struct TextureMap {
  TextureHandle texture_handle{kInvalidTextureHandle};
  TextureType type{TextureType::Invalid};
  resource::ResourceId texture_resource_id{resource::kInvalidResourceId};
};
}  // namespace gl
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_OPENGL_DATA_OPENGL_TEXTURE_MAP_H_
