// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_EDITOR_ASSET_EXPORTER_TEXTURE_DATA_TEXTURE_EXPORT_KEYS_H_
#define COMET_EDITOR_ASSET_EXPORTER_TEXTURE_DATA_TEXTURE_EXPORT_KEYS_H_

#include "comet/core.h"

using namespace std::literals;

namespace comet {
namespace tool {
namespace assetc {
static constexpr auto kCometEditorTextureMetadataKeyFormat{"format"sv};
static constexpr auto kCometEditorTextureMetadataKeyWidth{"width"sv};
static constexpr auto kCometEditorTextureMetadataKeyHeight{"height"sv};
static constexpr auto kCometEditorTextureMetadataKeySize{"size"sv};

static constexpr auto kCometEditorTextureFormatRgba8{"rgba8"sv};
}  // namespace assetc
}  // namespace tool
}  // namespace comet

#endif  // COMET_EDITOR_ASSET_EXPORTER_TEXTURE_DATA_TEXTURE_EXPORT_KEYS_H_