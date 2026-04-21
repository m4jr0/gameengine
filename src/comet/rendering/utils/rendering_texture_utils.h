// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_UTILS_RENDERING_TEXTURE_UTILS_H_
#define COMET_COMET_RENDERING_UTILS_RENDERING_TEXTURE_UTILS_H_

#include "comet/core/essentials.h"
#include "comet/rendering/type/rendering_texture_type.h"

namespace comet {
namespace rendering {
bool IsSrgbTextureType(TextureType type);

u32 GetMipLevels(u32 width, u32 height);
u8 GetResolvedChannelCount(TextureFormat format, u8 fallback);
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_UTILS_RENDERING_TEXTURE_UTILS_H_