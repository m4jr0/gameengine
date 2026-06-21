// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_DATA_RENDER_UTILS_TEXTURE_UTILS_H_
#define COMET_DATA_RENDER_UTILS_TEXTURE_UTILS_H_

#include "comet/core/essentials.h"
#include "comet/data/render/texture.h"

namespace comet {
namespace render {
bool IsSrgbTextureType(TextureType type);

u32 GetMipLevels(u32 width, u32 height);
u8 GetResolvedChannelCount(TextureFormat format, u8 fallback);
}  // namespace render
}  // namespace comet

#endif  // COMET_DATA_RENDER_UTILS_TEXTURE_UTILS_H_