// Copyright 2021 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_CORE_RENDER_TEXTURE_TEXTURE_LOADER_HPP_
#define COMET_CORE_RENDER_TEXTURE_TEXTURE_LOADER_HPP_

constexpr auto kLoggerCometCoreRenderTextureTextureLoader = "comet_core_render";

#include "comet_precompile.hpp"

namespace comet {
unsigned int Load2DTextureFromFile(const std::string &, bool = false);
}  // namespace comet

#endif  // COMET_CORE_RENDER_TEXTURE_TEXTURE_LOADER_HPP_
