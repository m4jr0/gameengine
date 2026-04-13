// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "opengl_texture.h"
////////////////////////////////////////////////////////////////////////////////

namespace comet {
namespace rendering {
namespace gl {
HashValue GenerateHash(const TextureKey& key) {
  HashValue hash{0};
  hash = HashCombine(hash, static_cast<HashValue>(key.resource_id));
  hash = HashCombine(hash, static_cast<u32>(key.type));
  return hash;
}
}  // namespace gl
}  // namespace rendering
}  // namespace comet