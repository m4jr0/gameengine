// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_TEXTURE_TEXTURE_LOADER_H_
#define COMET_COMET_RENDERING_TEXTURE_TEXTURE_LOADER_H_

#include "comet_precompile.h"

namespace comet {
namespace rendering {
unsigned int Load2DTextureFromFile(const std::string& texture_path,
                                   bool is_gamma = false);
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_TEXTURE_TEXTURE_LOADER_H_
