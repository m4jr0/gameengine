// Copyright 2021 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef KOMA_CORE_RENDER_TEXTURE_TEXTURE_LOADER_HPP_
#define KOMA_CORE_RENDER_TEXTURE_TEXTURE_LOADER_HPP_

constexpr auto kLoggerKomaCoreRenderTextureTextureLoader = "koma_core_render";

#include <string>

namespace koma {
unsigned int Load2DTextureFromFile(const std::string &, bool = false);
}  // namespace koma

#endif  // KOMA_CORE_RENDER_TEXTURE_TEXTURE_LOADER_HPP_
