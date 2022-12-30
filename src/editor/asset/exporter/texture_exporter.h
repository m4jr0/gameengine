// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_EDITOR_ASSET_EXPORTER_TEXTURE_EXPORTER_H_
#define COMET_EDITOR_ASSET_EXPORTER_TEXTURE_EXPORTER_H_

#include "comet_precompile.h"

#include "editor/asset/exporter/asset_exporter.h"

namespace comet {
namespace editor {
namespace asset {
constexpr char kCometEditorTextureMetadataKeyFormat[]{"format"};
constexpr char kCometEditorTextureMetadataKeyWidth[]{"width"};
constexpr char kCometEditorTextureMetadataKeyHeight[]{"height"};
constexpr char kCometEditorTextureMetadataKeySize[]{"size"};

constexpr char kCometEditorTextureFormatRgba8[]{"rgba8"};

class TextureExporter : public AssetExporter {
 public:
  TextureExporter() = default;
  TextureExporter(const TextureExporter&) = delete;
  TextureExporter(TextureExporter&&) = delete;
  TextureExporter& operator=(const TextureExporter&) = delete;
  TextureExporter& operator=(TextureExporter&&) = delete;
  ~TextureExporter() = default;

  bool IsCompatible(const std::string& extension) override;

 protected:
  std::vector<resource::ResourceFile> GetResourceFiles(
      AssetDescr& asset_descr) override;
};
}  // namespace asset
}  // namespace editor
}  // namespace comet

#endif  // COMET_EDITOR_ASSET_EXPORTER_TEXTURE_EXPORTER_H_
