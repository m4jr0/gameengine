// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_EDITOR_ASSET_EXPORTER_TEXTURE_EXPORTER_H_
#define COMET_EDITOR_ASSET_EXPORTER_TEXTURE_EXPORTER_H_

#include "comet_precompile.h"

#include "comet/core/type/tstring.h"
#include "comet/rendering/rendering_common.h"
#include "editor/asset/exporter/asset_exporter.h"

using namespace std::literals;

namespace comet {
namespace editor {
namespace asset {
static constexpr auto kCometEditorTextureMetadataKeyFormat{"format"sv};
static constexpr auto kCometEditorTextureMetadataKeyWidth{"width"sv};
static constexpr auto kCometEditorTextureMetadataKeyHeight{"height"sv};
static constexpr auto kCometEditorTextureMetadataKeySize{"size"sv};

static constexpr auto kCometEditorTextureFormatRgba8{"rgba8"sv};

class TextureExporter : public AssetExporter {
 public:
  TextureExporter() = default;
  TextureExporter(const TextureExporter&) = delete;
  TextureExporter(TextureExporter&&) = delete;
  TextureExporter& operator=(const TextureExporter&) = delete;
  TextureExporter& operator=(TextureExporter&&) = delete;
  virtual ~TextureExporter() = default;

  bool IsCompatible(CTStringView extension) const override;

 protected:
  std::vector<resource::ResourceFile> GetResourceFiles(
      AssetDescr& asset_descr) const override;
};
}  // namespace asset
}  // namespace editor
}  // namespace comet

#endif  // COMET_EDITOR_ASSET_EXPORTER_TEXTURE_EXPORTER_H_
