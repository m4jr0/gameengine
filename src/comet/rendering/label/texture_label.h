// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_LABEL_TEXTURE_LABEL_H_
#define COMET_COMET_RENDERING_LABEL_TEXTURE_LABEL_H_

#include "comet/core/essentials.h"
#include "comet/rendering/type/texture.h"

namespace comet {
namespace rendering {
const schar* GetTextureTypeLabel(TextureType type);
const schar* GetTextureRepeatModeLabel(TextureRepeatMode mode);
const schar* GetTextureFilterModeLabel(TextureFilterMode mode);
const schar* GetTextureFormatLabel(TextureFormat format);
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_LABEL_TEXTURE_LABEL_H_